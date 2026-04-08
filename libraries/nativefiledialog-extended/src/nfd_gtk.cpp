/*
  Native File Dialog Extended
  Repository: https://github.com/btzy/nativefiledialog-extended
  License: Zlib
  Authors: Bernard Teo, Michael Labbe

  Note: We do not check for malloc failure on Linux - Linux overcommits memory!
*/

#include <gtk/gtk.h>

#if defined(NFD_X11)
#if !defined(GDK_WINDOWING_X11)
#if defined(__GNUC__)
#pragma GCC warning \
    "NFD is built with X11 but GTK does not support X11, so window parenting will not work."
#endif
#undef NFD_X11
#endif
#endif
#if defined(NFD_WAYLAND)
#if !defined(GDK_WINDOWING_WAYLAND)
#if defined(__GNUC__)
#pragma GCC warning \
    "NFD is built with Wayland but GTK does not support Wayland, so window parenting will not work."
#endif
#undef NFD_WAYLAND
#endif
#endif

#if defined(NFD_X11)
#include <gdk/gdkx.h>
#endif
#if defined(NFD_WAYLAND)
#include <gdk/gdkwayland.h>
#endif

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nfd.h"

#include "nfd_linux_shared.hpp"

/*
Define NFD_CASE_SENSITIVE_FILTER if you want file filters to be case-sensitive.  The default
is case-insensitive.  While Linux uses a case-sensitive filesystem and is designed for
case-sensitive file extensions, perhaps in the vast majority of cases users actually expect the file
filters to be case-insensitive.
*/

namespace {

template <typename T>
struct Free_Guard {
    T* data;
    Free_Guard(T* freeable) noexcept : data(freeable) {}
    ~Free_Guard() { NFDi_Free(data); }
};

template <typename T>
struct FreeCheck_Guard {
    T* data;
    FreeCheck_Guard(T* freeable = nullptr) noexcept : data(freeable) {}
    ~FreeCheck_Guard() {
        if (data) NFDi_Free(data);
    }
};

/* current error */
const char* g_errorstr = nullptr;

void NFDi_SetError(const char* msg) {
    g_errorstr = msg;
}

template <typename T = void>
T* NFDi_Malloc(size_t bytes) {
    void* ptr = malloc(bytes);
    if (!ptr) NFDi_SetError("NFDi_Malloc failed.");

    return static_cast<T*>(ptr);
}

template <typename T>
void NFDi_Free(T* ptr) {
    assert(ptr);
    free(static_cast<void*>(ptr));
}

template <typename T>
T* copy(const T* begin, const T* end, T* out) {
    for (; begin != end; ++begin) {
        *out++ = *begin;
    }
    return out;
}

#ifndef NFD_CASE_SENSITIVE_FILTER
nfdnchar_t* emit_case_insensitive_glob(const nfdnchar_t* begin,
                                       const nfdnchar_t* end,
                                       nfdnchar_t* out) {
    // this code will only make regular Latin characters case-insensitive; other
    // characters remain case sensitive
    for (; begin != end; ++begin) {
        if ((*begin >= 'A' && *begin <= 'Z') || (*begin >= 'a' && *begin <= 'z')) {
            *out++ = '[';
            *out++ = *begin;
            // invert the case of the original character
            *out++ = *begin ^ static_cast<nfdnchar_t>(0x20);
            *out++ = ']';
        } else {
            *out++ = *begin;
        }
    }
    return out;
}
#endif

// Does not own the filter and extension.
struct Pair_GtkFileFilter_FileExtension {
    GtkFileFilter* filter;
    const nfdnchar_t* extensionBegin;
    const nfdnchar_t* extensionEnd;
};

struct ButtonClickedArgs {
    Pair_GtkFileFilter_FileExtension* map;
    GtkFileChooser* chooser;
};

void AddFiltersToDialog(GtkFileChooser* chooser,
                        const nfdnfilteritem_t* filterList,
                        nfdfiltersize_t filterCount) {
    if (filterCount) {
        assert(filterList);

        // we have filters to add ... format and add them

        for (nfdfiltersize_t index = 0; index != filterCount; ++index) {
            GtkFileFilter* filter = gtk_file_filter_new();

            // count number of file extensions
            size_t sep = 1;
            for (const nfdnchar_t* p_spec = filterList[index].spec; *p_spec; ++p_spec) {
                if (*p_spec == ',') {
                    ++sep;
                }
            }

            // friendly name conversions: "png,jpg" -> "Image files
            // (png, jpg)"

            // calculate space needed (including the trailing '\0')
            size_t nameSize =
                sep + strlen(filterList[index].spec) + 3 + strlen(filterList[index].name);

            // malloc the required memory
            nfdnchar_t* nameBuf = NFDi_Malloc<nfdnchar_t>(sizeof(nfdnchar_t) * nameSize);

            nfdnchar_t* p_nameBuf = nameBuf;
            for (const nfdnchar_t* p_filterName = filterList[index].name; *p_filterName;
                 ++p_filterName) {
                *p_nameBuf++ = *p_filterName;
            }
            *p_nameBuf++ = ' ';
            *p_nameBuf++ = '(';
            const nfdnchar_t* p_extensionStart = filterList[index].spec;
            for (const nfdnchar_t* p_spec = filterList[index].spec; true; ++p_spec) {
                if (*p_spec == ',' || !*p_spec) {
                    if (*p_spec == ',') {
                        *p_nameBuf++ = ',';
                        *p_nameBuf++ = ' ';
                    }

#ifdef NFD_CASE_SENSITIVE_FILTER
                    // +1 for the trailing '\0'
                    nfdnchar_t* extnBuf = NFDi_Malloc<nfdnchar_t>(sizeof(nfdnchar_t) *
                                                                  (p_spec - p_extensionStart + 3));
                    nfdnchar_t* p_extnBufEnd = extnBuf;
                    *p_extnBufEnd++ = '*';
                    *p_extnBufEnd++ = '.';
                    p_extnBufEnd = copy(p_extensionStart, p_spec, p_extnBufEnd);
                    *p_extnBufEnd++ = '\0';
                    gtk_file_filter_add_pattern(filter, extnBuf);
                    NFDi_Free(extnBuf);
#else
                    // Each character in the Latin alphabet is converted into 4 characters.  E.g.
                    // 'a' is converted into "[Aa]".  Other characters are preserved.  Then we +1
                    // for the trailing '\0'.
                    nfdnchar_t* extnBuf = NFDi_Malloc<nfdnchar_t>(
                        sizeof(nfdnchar_t) * ((p_spec - p_extensionStart) * 4 + 3));
                    nfdnchar_t* p_extnBufEnd = extnBuf;
                    *p_extnBufEnd++ = '*';
                    *p_extnBufEnd++ = '.';
                    p_extnBufEnd =
                        emit_case_insensitive_glob(p_extensionStart, p_spec, p_extnBufEnd);
                    *p_extnBufEnd++ = '\0';
                    gtk_file_filter_add_pattern(filter, extnBuf);
                    NFDi_Free(extnBuf);
#endif

                    if (*p_spec) {
                        // update the extension start point
                        p_extensionStart = p_spec + 1;
                    } else {
                        // reached the '\0' character
                        break;
                    }
                } else {
                    *p_nameBuf++ = *p_spec;
                }
            }
            *p_nameBuf++ = ')';
            *p_nameBuf++ = '\0';
            assert((size_t)(p_nameBuf - nameBuf) == sizeof(nfdnchar_t) * nameSize);

            // add to the filter
            gtk_file_filter_set_name(filter, nameBuf);

            // free the memory
            NFDi_Free(nameBuf);

            // add filter to chooser
            gtk_file_chooser_add_filter(chooser, filter);
        }
    }

    /* always append a wildcard option to the end*/

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(chooser, filter);
}

// returns null-terminated map (trailing .filter is null)
Pair_GtkFileFilter_FileExtension* AddFiltersToDialogWithMap(GtkFileChooser* chooser,
                                                            const nfdnfilteritem_t* filterList,
                                                            nfdfiltersize_t filterCount) {
    Pair_GtkFileFilter_FileExtension* map = NFDi_Malloc<Pair_GtkFileFilter_FileExtension>(
        sizeof(Pair_GtkFileFilter_FileExtension) * (filterCount + 1));

    if (filterCount) {
        assert(filterList);

        // we have filters to add ... format and add them

        for (nfdfiltersize_t index = 0; index != filterCount; ++index) {
            GtkFileFilter* filter = gtk_file_filter_new();

            // store filter in map
            map[index].filter = filter;
            map[index].extensionBegin = filterList[index].spec;
            map[index].extensionEnd = nullptr;

            // count number of file extensions
            size_t sep = 1;
            for (const nfdnchar_t* p_spec = filterList[index].spec; *p_spec; ++p_spec) {
                if (*p_spec == ',') {
                    ++sep;
                }
            }

            // friendly name conversions: "png,jpg" -> "Image files
            // (png, jpg)"

            // calculate space needed (including the trailing '\0')
            size_t nameSize =
                sep + strlen(filterList[index].spec) + 3 + strlen(filterList[index].name);

            // malloc the required memory
            nfdnchar_t* nameBuf = NFDi_Malloc<nfdnchar_t>(sizeof(nfdnchar_t) * nameSize);

            nfdnchar_t* p_nameBuf = nameBuf;
            for (const nfdnchar_t* p_filterName = filterList[index].name; *p_filterName;
                 ++p_filterName) {
                *p_nameBuf++ = *p_filterName;
            }
            *p_nameBuf++ = ' ';
            *p_nameBuf++ = '(';
            const nfdnchar_t* p_extensionStart = filterList[index].spec;
            for (const nfdnchar_t* p_spec = filterList[index].spec; true; ++p_spec) {
                if (*p_spec == ',' || !*p_spec) {
                    if (*p_spec == ',') {
                        *p_nameBuf++ = ',';
                        *p_nameBuf++ = ' ';
                    }

#ifdef NFD_CASE_SENSITIVE_FILTER
                    // +1 for the trailing '\0'
                    nfdnchar_t* extnBuf = NFDi_Malloc<nfdnchar_t>(sizeof(nfdnchar_t) *
                                                                  (p_spec - p_extensionStart + 3));
                    nfdnchar_t* p_extnBufEnd = extnBuf;
                    *p_extnBufEnd++ = '*';
                    *p_extnBufEnd++ = '.';
                    p_extnBufEnd = copy(p_extensionStart, p_spec, p_extnBufEnd);
                    *p_extnBufEnd++ = '\0';
                    gtk_file_filter_add_pattern(filter, extnBuf);
                    NFDi_Free(extnBuf);
#else
                    // Each character in the Latin alphabet is converted into 4 characters.  E.g.
                    // 'a' is converted into "[Aa]".  Other characters are preserved.  Then we +1
                    // for the trailing '\0'.
                    nfdnchar_t* extnBuf = NFDi_Malloc<nfdnchar_t>(
                        sizeof(nfdnchar_t) * ((p_spec - p_extensionStart) * 4 + 3));
                    nfdnchar_t* p_extnBufEnd = extnBuf;
                    *p_extnBufEnd++ = '*';
                    *p_extnBufEnd++ = '.';
                    p_extnBufEnd =
                        emit_case_insensitive_glob(p_extensionStart, p_spec, p_extnBufEnd);
                    *p_extnBufEnd++ = '\0';
                    gtk_file_filter_add_pattern(filter, extnBuf);
                    NFDi_Free(extnBuf);
#endif

                    // store current pointer in map (if it's
                    // the first one)
                    if (map[index].extensionEnd == nullptr) {
                        map[index].extensionEnd = p_spec;
                    }

                    if (*p_spec) {
                        // update the extension start point
                        p_extensionStart = p_spec + 1;
                    } else {
                        // reached the '\0' character
                        break;
                    }
                } else {
                    *p_nameBuf++ = *p_spec;
                }
            }
            *p_nameBuf++ = ')';
            *p_nameBuf++ = '\0';
            assert((size_t)(p_nameBuf - nameBuf) == sizeof(nfdnchar_t) * nameSize);

            // add to the filter
            gtk_file_filter_set_name(filter, nameBuf);

            // free the memory
            NFDi_Free(nameBuf);

            // add filter to chooser
            gtk_file_chooser_add_filter(chooser, filter);
        }
    }
    // set trailing map index to null
    map[filterCount].filter = nullptr;

    /* always append a wildcard option to the end*/
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(chooser, filter);

    return map;
}

void SetDefaultPath(GtkFileChooser* chooser, const char* defaultPath) {
    if (!defaultPath || !*defaultPath) return;

    /* GTK+ manual recommends not specifically setting the default path.
    We do it anyway in order to be consistent across platforms.

    If consistency with the native OS is preferred, this is the line
    to comment out. -ml */
    gtk_file_chooser_set_current_folder(chooser, defaultPath);
}

void SetDefaultName(GtkFileChooser* chooser, const char* defaultName) {
    if (!defaultName || !*defaultName) return;

    gtk_file_chooser_set_current_name(chooser, defaultName);
}

void WaitForCleanup() {
    while (gtk_events_pending()) gtk_main_iteration();
}

struct Widget_Guard {
    GtkWidget* data;
    Widget_Guard(GtkWidget* widget) : data(widget) {}
    ~Widget_Guard() {
        WaitForCleanup();
        gtk_widget_destroy(data);
        WaitForCleanup();
    }
};

void FileActivatedSignalHandler(GtkButton* saveButton, void* userdata) {
    (void)saveButton;  // silence the unused arg warning

    ButtonClickedArgs* args = static_cast<ButtonClickedArgs*>(userdata);
    GtkFileChooser* chooser = args->chooser;
    char* currentFileName = gtk_file_chooser_get_current_name(chooser);
    if (*currentFileName) {  // string is not empty

        // find a '.' in the file name
        const char* p_period = currentFileName;
        for (; *p_period; ++p_period) {
            if (*p_period == '.') {
                break;
            }
        }

        if (!*p_period) {  // there is no '.', so append the default extension
            Pair_GtkFileFilter_FileExtension* filterMap =
                static_cast<Pair_GtkFileFilter_FileExtension*>(args->map);
            GtkFileFilter* currentFilter = gtk_file_chooser_get_filter(chooser);
            if (currentFilter) {
                for (; filterMap->filter; ++filterMap) {
                    if (filterMap->filter == currentFilter) break;
                }
            }
            if (filterMap->filter) {
                // memory for appended string (including '.' and
                // trailing '\0')
                char* appendedFileName = NFDi_Malloc<char>(
                    sizeof(char) * ((p_period - currentFileName) +
                                    (filterMap->extensionEnd - filterMap->extensionBegin) + 2));
                char* p_fileName = copy(currentFileName, p_period, appendedFileName);
                *p_fileName++ = '.';
                p_fileName = copy(filterMap->extensionBegin, filterMap->extensionEnd, p_fileName);
                *p_fileName++ = '\0';

                assert(p_fileName - appendedFileName ==
                       (p_period - currentFileName) +
                           (filterMap->extensionEnd - filterMap->extensionBegin) + 2);

                // set the appended file name
                gtk_file_chooser_set_current_name(chooser, appendedFileName);

                // free the memory
                NFDi_Free(appendedFileName);
            }
        }
    }

    // free the memory
    g_free(currentFileName);
}

// wrapper for gtk_dialog_run() that brings the dialog to the front
// see issues at:
// https://github.com/btzy/nativefiledialog-extended/issues/31
// https://github.com/mlabbe/nativefiledialog/pull/92
// https://github.com/guillaumechereau/noc/pull/11
gint RunDialogWithFocus(GtkDialog* dialog) {
#if defined(NFD_X11)
    gtk_widget_show_all(GTK_WIDGET(dialog));  // show the dialog so that it gets a display
    if (GDK_IS_X11_DISPLAY(gtk_widget_get_display(GTK_WIDGET(dialog)))) {
        GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(dialog));
        gdk_window_set_events(
            window,
            static_cast<GdkEventMask>(gdk_window_get_events(window) | GDK_PROPERTY_CHANGE_MASK));
        gtk_window_present_with_time(GTK_WINDOW(dialog), gdk_x11_get_server_time(window));
    }
#endif
    return gtk_dialog_run(dialog);
}

#if defined(NFD_WAYLAND)
void DestroyXdgExported(void* context) {
    zxdg_exported_v1_destroy(static_cast<struct zxdg_exported_v1*>(context));
}

void zxdg_exported_v1_handle(void* context, struct zxdg_exported_v1*, const char* handle) {
    GdkWindow* childWindow = static_cast<GdkWindow*>(context);
    // GDK doesn't modify the string, even though it takes char*, so we can cast away the constness
    gdk_wayland_window_set_transient_for_exported(childWindow, const_cast<char*>(handle));
}

constexpr struct zxdg_exported_v1_listener wayland_xdg_exported_v1_listener {
    &zxdg_exported_v1_handle
};
#endif

// This is an RAII class that wraps the parenting of a GtkWidget (the file dialog).
// To parent a window on GTK, the child GdkWindow needs to be on the same screen as the parent.
// Before the GtkWidget is realized (i.e. the GdkWindow is created for it), we need to tell it the
// GdkScreen to use. Then, after realization, we can get the GtkWidget's GdkWindow and set its
// transient parent to the parent's GdkWindow (but this only works if the parent window uses the
// display server (i.e. X11 or Wayland)). So before realization, we give the GtkWidget a GdkScreen
// for the parent's display server, and after realization we set its transient parent.
struct NativeWindowParenter {
    NativeWindowParenter(GtkWidget* w, const nfdwindowhandle_t& parentHandle) noexcept {
        GdkScreen* gdk_screen;
        void (*realized_handler)(GtkWidget*, void*);
        GetScreenAndHandler(parentHandle.type, gdk_screen, realized_handler);

        if (gdk_screen && realized_handler) {
            widget = w;

            parentWindowHandle = parentHandle.handle;

            // make the dialog window use a GtkScreen with the same display server as the parent (so
            // that parenting works)
            gtk_window_set_screen(GTK_WINDOW(w), gdk_screen);

            handlerID = g_signal_connect(
                G_OBJECT(w), "realize", G_CALLBACK(realized_handler), static_cast<void*>(this));
        } else {
            widget = nullptr;
        }
    }

    ~NativeWindowParenter() {
        // unset the handler
        if (widget) {
            g_signal_handler_disconnect(G_OBJECT(widget), handlerID);
        }
        // No need to call destroy.fn because it is destroyed in the destructor of DestroyFunc.
    }

    static void GetScreenAndHandler(size_t parentWindowType,
                                    GdkScreen*& outScreen,
                                    void (*&outHandler)(GtkWidget*, void*)) {
        switch (parentWindowType) {
#if defined(NFD_X11)
            case NFD_WINDOW_HANDLE_TYPE_X11: {
                if (x11_gdk_screen) {
                    outScreen = x11_gdk_screen;
                    outHandler = &RealizedSignalHandler<&NativeWindowParenter::SetParentX11>;
                    return;
                }

                GdkDisplayManager* display_manager = gdk_display_manager_get();

                // If we can find an existing X11 display, use it.
                GSList* gdk_display_list = gdk_display_manager_list_displays(display_manager);
                while (gdk_display_list) {
                    GSList* node = gdk_display_list;
                    GdkDisplay* display = GDK_DISPLAY(node->data);
                    if (GDK_IS_X11_DISPLAY(display)) {
                        g_slist_free(node);
                        x11_gdk_display = display;
                        break;
                    } else {
                        gdk_display_list = node->next;
                        g_slist_free_1(node);
                    }
                }

                // Otherwise, we have to create our own X11 display.
                if (!x11_gdk_display) {
                    // This is not very nice, because we are always resetting the allowed backends
                    // setting to NULL (which means all backends are allowed), even though we can't
                    // be sure that the user didn't call gdk_set_allowed_backends() earlier to force
                    // a specific backend.  But well if the user doesn't have an X11 display already
                    // open and yet is telling us with have an X11 window as parent, they probably
                    // don't use GTK in their application at all so they probably won't notice this.
                    //
                    // There is no way, AFAIK, to get the allowed backends first so we can restore
                    // it later, and gdk_x11_display_open() is GTK4-only (the GTK3 version is a
                    // private implementation detail).
                    //
                    // Also, we don't close the display we specially opened, since GTK will need it
                    // to show the dialog.  Though it probably doesn't matter very much if we want
                    // to free up resources and clean it up.
                    gdk_set_allowed_backends("x11");
                    GdkDisplay* display =
                        gdk_display_manager_open_display(display_manager, nullptr);
                    gdk_set_allowed_backends(nullptr);
                    if (display) {
                        if (GDK_IS_X11_DISPLAY(display))
                            x11_gdk_display = display;
                        else
                            gdk_display_close(display);
                    }
                }

                if (x11_gdk_display) {
                    // Set the screen if we have a display.
                    x11_gdk_screen = gdk_display_get_default_screen(x11_gdk_display);
                    // In the unlikely situation that we can't get the default screen, set the
                    // display back to null.
                    if (!x11_gdk_screen) x11_gdk_display = nullptr;
                }

                outScreen = x11_gdk_screen;
                outHandler = x11_gdk_screen
                                 ? &RealizedSignalHandler<&NativeWindowParenter::SetParentX11>
                                 : nullptr;
                return;
            }
#endif
#if defined(NFD_WAYLAND)
            case NFD_WINDOW_HANDLE_TYPE_WAYLAND: {
                if (wayland_gdk_screen) {
                    outScreen = wayland_gdk_screen;
                    outHandler = &RealizedSignalHandler<&NativeWindowParenter::SetParentWayland>;
                    return;
                }

                // This will contain the Wayland display we want to use.
                GdkDisplay* wayland_gdk_display = nullptr;
                GdkDisplayManager* display_manager = gdk_display_manager_get();

                // If we can find an existing Wayland display, use it.
                GSList* gdk_display_list = gdk_display_manager_list_displays(display_manager);
                while (gdk_display_list) {
                    GSList* node = gdk_display_list;
                    GdkDisplay* display = GDK_DISPLAY(node->data);
                    if (GDK_IS_WAYLAND_DISPLAY(display)) {
                        g_slist_free(node);
                        wayland_gdk_display = display;
                        break;
                    } else {
                        gdk_display_list = node->next;
                        g_slist_free_1(node);
                    }
                }

                // Otherwise, we have to create our own Wayland display.
                if (!wayland_gdk_display) {
                    // This is not very nice, because we are always resetting the allowed backends
                    // setting to NULL (which means all backends are allowed), even though we can't
                    // be sure that the user didn't call gdk_set_allowed_backends() earlier to force
                    // a specific backend.  But well if the user doesn't have an Wayland display
                    // already open and yet is telling us with have an Wayland window as parent,
                    // they probably don't use GTK in their application at all so they probably
                    // won't notice this.
                    //
                    // There is no way, AFAIK, to get the allowed backends first so we can restore
                    // it later, and gdk_wayland_display_open() is GTK4-only (the GTK3 version is a
                    // private implementation detail).
                    //
                    // Also, we don't close the display we specially opened, since GTK will need it
                    // to show the dialog.  Though it probably doesn't matter very much if we want
                    // to free up resources and clean it up.
                    gdk_set_allowed_backends("wayland");
                    GdkDisplay* display = gdk_display_manager_open_display(display_manager, NULL);
                    gdk_set_allowed_backends(NULL);
                    if (display) {
                        if (GDK_IS_WAYLAND_DISPLAY(display))
                            wayland_gdk_display = display;
                        else
                            gdk_display_close(display);
                    }
                }
                if (wayland_gdk_display) {
                    // Set the screen if we have a display.
                    wayland_gdk_screen = gdk_display_get_default_screen(wayland_gdk_display);
                }
                outScreen = wayland_gdk_screen;
                outHandler = wayland_gdk_screen
                                 ? &RealizedSignalHandler<&NativeWindowParenter::SetParentWayland>
                                 : nullptr;
                return;
            }
#endif
            default:
                outScreen = nullptr;
                outHandler = nullptr;
                return;
        }
    }

    template <void (NativeWindowParenter::*Func)(GdkWindow*)>
    static void RealizedSignalHandler(GtkWidget* childWindow, void* userdata) {
        NativeWindowParenter& data = *static_cast<NativeWindowParenter*>(userdata);
        (data.*Func)(gtk_widget_get_window(childWindow));
    }

#if defined(NFD_X11)
    void SetParentX11(GdkWindow* childWindow) {
        const Window x11_handle = reinterpret_cast<Window>(parentWindowHandle);
        GdkWindow* gdk_window = gdk_x11_window_foreign_new_for_display(x11_gdk_display, x11_handle);
        gdk_window_set_transient_for(childWindow, gdk_window);
        destroy.fn = &g_object_unref;
        destroy.context = static_cast<void*>(gdk_window);
    }
#endif

#if defined(NFD_WAYLAND)
    void SetParentWayland(GdkWindow* childWindow) {
        if (wayland_display && wayland_xdg_exporter_v1) {
            struct zxdg_exported_v1* exported = zxdg_exporter_v1_export(
                wayland_xdg_exporter_v1, static_cast<struct wl_surface*>(parentWindowHandle));
            if (!exported) {
                // if we fail to export the wl_surface, act as if the window has no parent
                return;
            }
            zxdg_exported_v1_add_listener(
                exported, &wayland_xdg_exported_v1_listener, static_cast<void*>(childWindow));
            wl_display_roundtrip(wayland_display);
            zxdg_exported_v1_set_user_data(exported, nullptr);
            destroy.fn = &DestroyXdgExported;
            destroy.context = static_cast<void*>(exported);
        }
    }
#endif

    GtkWidget* widget;
    void* parentWindowHandle;
    DestroyFunc destroy;
    gulong handlerID;
#if defined(NFD_X11)
    static GdkDisplay* x11_gdk_display;
    static GdkScreen* x11_gdk_screen;
#endif
#if defined(NFD_WAYLAND)
    static GdkScreen* wayland_gdk_screen;
#endif
};
#if defined(NFD_X11)
GdkDisplay* NativeWindowParenter::x11_gdk_display = nullptr;
GdkScreen* NativeWindowParenter::x11_gdk_screen = nullptr;
#endif
#if defined(NFD_WAYLAND)
GdkScreen* NativeWindowParenter::wayland_gdk_screen = nullptr;
#endif

}  // namespace

const char* NFD_GetError(void) {
    return g_errorstr;
}

void NFD_ClearError(void) {
    NFDi_SetError(nullptr);
}

/* public */

nfdresult_t NFD_Init(void) {
    // Init GTK
    if (!gtk_init_check(NULL, NULL)) {
        NFDi_SetError("Failed to initialize GTK+ with gtk_init_check.");
        return NFD_ERROR;
    }
#if defined(NFD_WAYLAND)
    NFD_Wayland_Init();
#endif
    return NFD_OKAY;
}

void NFD_Quit(void) {
#if defined(NFD_WAYLAND)
    NFD_Wayland_Quit();
#endif
    // do nothing about GTK since it cannot be de-initialized
}

void NFD_FreePathN(nfdnchar_t* filePath) {
    assert(filePath);
    g_free(filePath);
}

void NFD_FreePathU8(nfdu8char_t* filePath) __attribute__((alias("NFD_FreePathN")));

nfdresult_t NFD_OpenDialogN(nfdnchar_t** outPath,
                            const nfdnfilteritem_t* filterList,
                            nfdfiltersize_t filterCount,
                            const nfdnchar_t* defaultPath) {
    nfdopendialognargs_t args{};
    args.filterList = filterList;
    args.filterCount = filterCount;
    args.defaultPath = defaultPath;
    return NFD_OpenDialogN_With_Impl(NFD_INTERFACE_VERSION, outPath, &args);
}

nfdresult_t NFD_OpenDialogN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdopendialognargs_t* args) {
    // We haven't needed to bump the interface version yet.
    (void)version;

    GtkWidget* widget = gtk_file_chooser_dialog_new("Open File",
                                                    nullptr,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "_Open",
                                                    GTK_RESPONSE_ACCEPT,
                                                    nullptr);

    // guard to destroy the widget when returning from this function
    Widget_Guard widgetGuard(widget);

    /* Build the filter list */
    AddFiltersToDialog(GTK_FILE_CHOOSER(widget), args->filterList, args->filterCount);

    /* Set the default path */
    SetDefaultPath(GTK_FILE_CHOOSER(widget), args->defaultPath);

    gint result;
    {
        /* Parent the window properly */
        NativeWindowParenter nativeWindowParenter(widget, args->parentWindow);

        /* invoke the dialog (blocks until dialog is closed) */
        result = RunDialogWithFocus(GTK_DIALOG(widget));
    }

    if (result == GTK_RESPONSE_ACCEPT) {
        // write out the file name
        *outPath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

        return NFD_OKAY;
    } else {
        return NFD_CANCEL;
    }
}

nfdresult_t NFD_OpenDialogU8(nfdu8char_t** outPath,
                             const nfdu8filteritem_t* filterList,
                             nfdfiltersize_t filterCount,
                             const nfdu8char_t* defaultPath)
    __attribute__((alias("NFD_OpenDialogN")));

nfdresult_t NFD_OpenDialogU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdopendialogu8args_t* args)
    __attribute__((alias("NFD_OpenDialogN_With_Impl")));

nfdresult_t NFD_OpenDialogMultipleN(const nfdpathset_t** outPaths,
                                    const nfdnfilteritem_t* filterList,
                                    nfdfiltersize_t filterCount,
                                    const nfdnchar_t* defaultPath) {
    nfdopendialognargs_t args{};
    args.filterList = filterList;
    args.filterCount = filterCount;
    args.defaultPath = defaultPath;
    return NFD_OpenDialogMultipleN_With_Impl(NFD_INTERFACE_VERSION, outPaths, &args);
}

nfdresult_t NFD_OpenDialogMultipleN_With_Impl(nfdversion_t version,
                                              const nfdpathset_t** outPaths,
                                              const nfdopendialognargs_t* args) {
    // We haven't needed to bump the interface version yet.
    (void)version;

    GtkWidget* widget = gtk_file_chooser_dialog_new("Open Files",
                                                    nullptr,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "_Open",
                                                    GTK_RESPONSE_ACCEPT,
                                                    nullptr);

    // guard to destroy the widget when returning from this function
    Widget_Guard widgetGuard(widget);

    // set select multiple
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(widget), TRUE);

    /* Build the filter list */
    AddFiltersToDialog(GTK_FILE_CHOOSER(widget), args->filterList, args->filterCount);

    /* Set the default path */
    SetDefaultPath(GTK_FILE_CHOOSER(widget), args->defaultPath);

    gint result;
    {
        /* Parent the window properly */
        NativeWindowParenter nativeWindowParenter(widget, args->parentWindow);

        /* invoke the dialog (blocks until dialog is closed) */
        result = RunDialogWithFocus(GTK_DIALOG(widget));
    }

    if (result == GTK_RESPONSE_ACCEPT) {
        // write out the file name
        GSList* fileList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(widget));

        *outPaths = static_cast<void*>(fileList);
        return NFD_OKAY;
    } else {
        return NFD_CANCEL;
    }
}

nfdresult_t NFD_OpenDialogMultipleU8(const nfdpathset_t** outPaths,
                                     const nfdu8filteritem_t* filterList,
                                     nfdfiltersize_t filterCount,
                                     const nfdu8char_t* defaultPath)
    __attribute__((alias("NFD_OpenDialogMultipleN")));

nfdresult_t NFD_OpenDialogMultipleU8_With_Impl(nfdversion_t version,
                                               const nfdpathset_t** outPaths,
                                               const nfdopendialogu8args_t* args)
    __attribute__((alias("NFD_OpenDialogMultipleN_With_Impl")));

nfdresult_t NFD_SaveDialogN(nfdnchar_t** outPath,
                            const nfdnfilteritem_t* filterList,
                            nfdfiltersize_t filterCount,
                            const nfdnchar_t* defaultPath,
                            const nfdnchar_t* defaultName) {
    nfdsavedialognargs_t args{};
    args.filterList = filterList;
    args.filterCount = filterCount;
    args.defaultPath = defaultPath;
    args.defaultName = defaultName;
    return NFD_SaveDialogN_With_Impl(NFD_INTERFACE_VERSION, outPath, &args);
}

nfdresult_t NFD_SaveDialogN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdsavedialognargs_t* args) {
    // We haven't needed to bump the interface version yet.
    (void)version;

    GtkWidget* widget = gtk_file_chooser_dialog_new("Save File",
                                                    nullptr,
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    nullptr);

    // guard to destroy the widget when returning from this function
    Widget_Guard widgetGuard(widget);

    GtkWidget* saveButton = gtk_dialog_add_button(GTK_DIALOG(widget), "_Save", GTK_RESPONSE_ACCEPT);

    // Prompt on overwrite
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(widget), TRUE);

    /* Build the filter list */
    ButtonClickedArgs buttonClickedArgs;
    buttonClickedArgs.chooser = GTK_FILE_CHOOSER(widget);
    buttonClickedArgs.map =
        AddFiltersToDialogWithMap(GTK_FILE_CHOOSER(widget), args->filterList, args->filterCount);

    /* Set the default path */
    SetDefaultPath(GTK_FILE_CHOOSER(widget), args->defaultPath);

    /* Set the default file name */
    SetDefaultName(GTK_FILE_CHOOSER(widget), args->defaultName);

    /* set the handler to add file extension */
    gulong handlerID = g_signal_connect(G_OBJECT(saveButton),
                                        "pressed",
                                        G_CALLBACK(FileActivatedSignalHandler),
                                        static_cast<void*>(&buttonClickedArgs));

    gint result;
    {
        /* Parent the window properly */
        NativeWindowParenter nativeWindowParenter(widget, args->parentWindow);

        /* invoke the dialog (blocks until dialog is closed) */
        result = RunDialogWithFocus(GTK_DIALOG(widget));
    }

    /* unset the handler */
    g_signal_handler_disconnect(G_OBJECT(saveButton), handlerID);

    /* free the filter map */
    NFDi_Free(buttonClickedArgs.map);

    if (result == GTK_RESPONSE_ACCEPT) {
        // write out the file name
        *outPath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

        return NFD_OKAY;
    } else {
        return NFD_CANCEL;
    }
}

nfdresult_t NFD_SaveDialogU8(nfdu8char_t** outPath,
                             const nfdu8filteritem_t* filterList,
                             nfdfiltersize_t filterCount,
                             const nfdu8char_t* defaultPath,
                             const nfdu8char_t* defaultName)
    __attribute__((alias("NFD_SaveDialogN")));

nfdresult_t NFD_SaveDialogU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdsavedialogu8args_t* args)
    __attribute__((alias("NFD_SaveDialogN_With_Impl")));

nfdresult_t NFD_PickFolderN(nfdnchar_t** outPath, const nfdnchar_t* defaultPath) {
    nfdpickfoldernargs_t args{};
    args.defaultPath = defaultPath;
    return NFD_PickFolderN_With_Impl(NFD_INTERFACE_VERSION, outPath, &args);
}

nfdresult_t NFD_PickFolderN_With_Impl(nfdversion_t version,
                                      nfdnchar_t** outPath,
                                      const nfdpickfoldernargs_t* args) {
    // We haven't needed to bump the interface version yet.
    (void)version;

    GtkWidget* widget = gtk_file_chooser_dialog_new("Select Folder",
                                                    nullptr,
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "_Select",
                                                    GTK_RESPONSE_ACCEPT,
                                                    nullptr);

    // guard to destroy the widget when returning from this function
    Widget_Guard widgetGuard(widget);

    /* Set the default path */
    SetDefaultPath(GTK_FILE_CHOOSER(widget), args->defaultPath);

    gint result;
    {
        /* Parent the window properly */
        NativeWindowParenter nativeWindowParenter(widget, args->parentWindow);

        /* invoke the dialog (blocks until dialog is closed) */
        result = RunDialogWithFocus(GTK_DIALOG(widget));
    }

    if (result == GTK_RESPONSE_ACCEPT) {
        // write out the file name
        *outPath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));

        return NFD_OKAY;
    } else {
        return NFD_CANCEL;
    }
}

nfdresult_t NFD_PickFolderU8(nfdu8char_t** outPath, const nfdu8char_t* defaultPath)
    __attribute__((alias("NFD_PickFolderN")));

nfdresult_t NFD_PickFolderU8_With_Impl(nfdversion_t version,
                                       nfdu8char_t** outPath,
                                       const nfdpickfolderu8args_t* args)
    __attribute__((alias("NFD_PickFolderN_With_Impl")));

nfdresult_t NFD_PickFolderMultipleN(const nfdpathset_t** outPaths, const nfdnchar_t* defaultPath) {
    nfdpickfoldernargs_t args{};
    args.defaultPath = defaultPath;
    return NFD_PickFolderMultipleN_With_Impl(NFD_INTERFACE_VERSION, outPaths, &args);
}

nfdresult_t NFD_PickFolderMultipleN_With_Impl(nfdversion_t version,
                                              const nfdpathset_t** outPaths,
                                              const nfdpickfoldernargs_t* args) {
    // We haven't needed to bump the interface version yet.
    (void)version;

    GtkWidget* widget = gtk_file_chooser_dialog_new("Select Folders",
                                                    nullptr,
                                                    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "_Select",
                                                    GTK_RESPONSE_ACCEPT,
                                                    nullptr);

    // guard to destroy the widget when returning from this function
    Widget_Guard widgetGuard(widget);

    /* Set the default path */
    SetDefaultPath(GTK_FILE_CHOOSER(widget), args->defaultPath);

    gint result;
    {
        /* Parent the window properly */
        NativeWindowParenter nativeWindowParenter(widget, args->parentWindow);

        /* invoke the dialog (blocks until dialog is closed) */
        result = RunDialogWithFocus(GTK_DIALOG(widget));
    }

    if (result == GTK_RESPONSE_ACCEPT) {
        // write out the file name
        GSList* fileList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(widget));

        *outPaths = static_cast<void*>(fileList);
        return NFD_OKAY;
    } else {
        return NFD_CANCEL;
    }
}

nfdresult_t NFD_PickFolderMultipleU8(const nfdpathset_t** outPaths, const nfdu8char_t* defaultPath)
    __attribute__((alias("NFD_PickFolderMultipleN")));

nfdresult_t NFD_PickFolderMultipleU8_With_Impl(nfdversion_t version,
                                               const nfdpathset_t** outPaths,
                                               const nfdpickfolderu8args_t* args)
    __attribute__((alias("NFD_PickFolderMultipleN_With_Impl")));

nfdresult_t NFD_PathSet_GetCount(const nfdpathset_t* pathSet, nfdpathsetsize_t* count) {
    assert(pathSet);
    // const_cast because methods on GSList aren't const, but it should act
    // like const to the caller
    GSList* fileList = const_cast<GSList*>(static_cast<const GSList*>(pathSet));

    *count = g_slist_length(fileList);
    return NFD_OKAY;
}

nfdresult_t NFD_PathSet_GetPathN(const nfdpathset_t* pathSet,
                                 nfdpathsetsize_t index,
                                 nfdnchar_t** outPath) {
    assert(pathSet);
    // const_cast because methods on GSList aren't const, but it should act
    // like const to the caller
    GSList* fileList = const_cast<GSList*>(static_cast<const GSList*>(pathSet));

    // Note: this takes linear time... but should be good enough
    *outPath = static_cast<nfdnchar_t*>(g_slist_nth_data(fileList, index));

    return NFD_OKAY;
}

nfdresult_t NFD_PathSet_GetPathU8(const nfdpathset_t* pathSet,
                                  nfdpathsetsize_t index,
                                  nfdu8char_t** outPath)
    __attribute__((alias("NFD_PathSet_GetPathN")));

void NFD_PathSet_FreePathN(const nfdnchar_t* filePath) {
    assert(filePath);
    (void)filePath;  // prevent warning in release build
    // no-op, because NFD_PathSet_Free does the freeing for us
}

void NFD_PathSet_FreePathU8(const nfdu8char_t* filePath)
    __attribute__((alias("NFD_PathSet_FreePathN")));

void NFD_PathSet_Free(const nfdpathset_t* pathSet) {
    assert(pathSet);
    // const_cast because methods on GSList aren't const, but it should act
    // like const to the caller
    GSList* fileList = const_cast<GSList*>(static_cast<const GSList*>(pathSet));

    // free all the nodes
    for (GSList* node = fileList; node; node = node->next) {
        assert(node->data);
        g_free(node->data);
    }

    // free the path set memory
    g_slist_free(fileList);
}

nfdresult_t NFD_PathSet_GetEnum(const nfdpathset_t* pathSet, nfdpathsetenum_t* outEnumerator) {
    // The pathset (GSList) is already a linked list, so the enumeration is itself
    outEnumerator->ptr = const_cast<void*>(pathSet);

    return NFD_OKAY;
}

void NFD_PathSet_FreeEnum(nfdpathsetenum_t*) {
    // Do nothing, because the enumeration is the pathset itself
}

nfdresult_t NFD_PathSet_EnumNextN(nfdpathsetenum_t* enumerator, nfdnchar_t** outPath) {
    const GSList* fileList = static_cast<const GSList*>(enumerator->ptr);

    if (fileList) {
        *outPath = static_cast<nfdnchar_t*>(fileList->data);
        enumerator->ptr = static_cast<void*>(fileList->next);
    } else {
        *outPath = nullptr;
    }

    return NFD_OKAY;
}

nfdresult_t NFD_PathSet_EnumNextU8(nfdpathsetenum_t* enumerator, nfdu8char_t** outPath)
    __attribute__((alias("NFD_PathSet_EnumNextN")));
