/*
  Native File Dialog Extended
  Repository: https://github.com/btzy/nativefiledialog-extended
  License: Zlib
  Authors: Bernard Teo

  This header contains a function to convert a GLFW window handle to a native window handle for
  passing to NFDe.
 */

#ifndef _NFD_GLFW3_H
#define _NFD_GLFW3_H

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <nfd.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#define NFD_INLINE inline
#else
#define NFD_INLINE static inline
#endif  // __cplusplus

/**
 * Sets the wayland display if the process is running under Wayland, otherwise does nothing.
 * @return Either true to indicate success, or false to indicate failure.  If false is returned,
 * you can call SDL_GetError() for more information.
 */
NFD_INLINE bool NFD_SetDisplayPropertiesFromGLFW(void) {
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
#if defined(GLFW_VERSION_MAJOR) && defined(GLFW_VERSION_MINOR) && \
    (GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4))
    // GLFW 3.4+ has glfwGetPlatform()
    const int platform = glfwGetPlatform();
    if (!platform) {
        return false;
    }
    if (platform != GLFW_PLATFORM_WAYLAND) {
        return true;
    }
    struct wl_display* wayland_display = glfwGetWaylandDisplay();
    if (!wayland_display) {
        return false;
    }
    return NFD_SetWaylandDisplay(wayland_display) == NFD_OKAY;
#else
    const GLFWerrorfun oldCallback = glfwSetErrorCallback(NULL);
    bool success = true;
    struct wl_display* wayland_display = glfwGetWaylandDisplay();
    if (wayland_display) {
        success = NFD_SetWaylandDisplay(wayland_display) == NFD_OKAY;
    }
    glfwSetErrorCallback(oldCallback);
    return success;
#endif
#else
    return true;
#endif
}

/**
 * Converts a GLFW window handle to a native window handle that can be passed to NFDe.
 * @param sdlWindow The GLFW window handle.
 * @param[out] nativeWindow The output native window handle, populated if and only if this function
 * returns true.
 * @return Either true to indicate success, or false to indicate failure.  It is intended that
 * users ignore the error and simply pass a value-initialized nfdwindowhandle_t to NFDe if this
 * function fails. */
NFD_INLINE bool NFD_GetNativeWindowFromGLFWWindow(GLFWwindow* glfwWindow,
                                                  nfdwindowhandle_t* nativeWindow) {
#if defined(GLFW_VERSION_MAJOR) && defined(GLFW_VERSION_MINOR) && \
    (GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4))
    // GLFW 3.4+ has glfwGetPlatform()
    const int platform = glfwGetPlatform();
    switch (platform) {
        case 0:
            return false;
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        case GLFW_PLATFORM_WIN32:
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
            nativeWindow->handle = (void*)glfwGetWin32Window(glfwWindow);
            return true;
#endif
#if defined(GLFW_EXPOSE_NATIVE_COCOA)
        case GLFW_PLATFORM_COCOA:
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_COCOA;
            nativeWindow->handle = (void*)glfwGetCocoaWindow(glfwWindow);
            return true;
#endif
#if defined(GLFW_EXPOSE_NATIVE_X11)
        case GLFW_PLATFORM_X11:
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_X11;
            nativeWindow->handle = (void*)glfwGetX11Window(glfwWindow);
            return true;
#endif
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        case GLFW_PLATFORM_WAYLAND:
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_WAYLAND;
            nativeWindow->handle = (void*)glfwGetWaylandWindow(glfwWindow);
            return true;
#endif
        default:
            (void)glfwWindow;
            (void)nativeWindow;
            return true;
    }
#else
    const GLFWerrorfun oldCallback = glfwSetErrorCallback(NULL);
    bool success = false;
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
    if (!success) {
        const HWND hwnd = glfwGetWin32Window(glfwWindow);
        if (hwnd) {
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
            nativeWindow->handle = (void*)hwnd;
            success = true;
        }
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_COCOA)
    if (!success) {
        const id cocoa_window = glfwGetCocoaWindow(glfwWindow);
        if (cocoa_window) {
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_COCOA;
            nativeWindow->handle = (void*)cocoa_window;
            success = true;
        }
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_X11)
    if (!success) {
        const Window x11_window = glfwGetX11Window(glfwWindow);
        if (x11_window != None) {
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_X11;
            nativeWindow->handle = (void*)x11_window;
            success = true;
        }
    }
#endif
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
    if (!success) {
        const struct wl_surface* wayland_window = glfwGetWaylandWindow(glfwWindow);
        if (wayland_window) {
            nativeWindow->type = NFD_WINDOW_HANDLE_TYPE_WAYLAND;
            nativeWindow->handle = (void*)wayland_window;
            success = true;
        }
    }
#endif
    glfwSetErrorCallback(oldCallback);
    return success;
#endif
}

#undef NFD_INLINE
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _NFD_GLFW3_H
