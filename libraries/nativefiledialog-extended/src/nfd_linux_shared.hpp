/*
  Native File Dialog Extended
  Repository: https://github.com/btzy/nativefiledialog-extended
  License: Zlib
  Authors: Bernard Teo

  These are shared functions for Linux (GTK and Portal).
*/

#ifdef NFD_WAYLAND
#include <wayland-client.h>
#include "xdg-foreign-unstable-v1.h"
#endif

namespace {

#ifdef NFD_WAYLAND
struct wl_display* wayland_display;
struct wl_registry* wayland_registry;
uint32_t wayland_xdg_exporter_v1_name;
struct zxdg_exporter_v1* wayland_xdg_exporter_v1;
constexpr const char* XDG_EXPORTER_V1 = "zxdg_exporter_v1";
#endif

void EmptyFn(void*) {}

struct DestroyFunc {
    DestroyFunc() : fn(&EmptyFn), context(nullptr) {}
    ~DestroyFunc() { (*fn)(context); }
    void (*fn)(void*);
    void* context;
};

#ifdef NFD_WAYLAND
void registry_handle_global(void* context,
                            struct wl_registry* registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version) {
    (void)context;
    (void)version;
    if (strcmp(interface, XDG_EXPORTER_V1) == 0) {
        wayland_xdg_exporter_v1_name = name;
        wayland_xdg_exporter_v1 = static_cast<struct zxdg_exporter_v1*>(wl_registry_bind(
            registry, name, &zxdg_exporter_v1_interface, zxdg_exporter_v1_interface.version));
    }
}

void registry_handle_global_remove(void* context, struct wl_registry* registry, uint32_t name) {
    (void)context;
    (void)registry;
    if (wayland_xdg_exporter_v1 && name == wayland_xdg_exporter_v1_name) {
        zxdg_exporter_v1_destroy(wayland_xdg_exporter_v1);
        wayland_xdg_exporter_v1 = nullptr;
    }
}

constexpr struct wl_registry_listener wayland_registry_listener = {&registry_handle_global,
                                                                   &registry_handle_global_remove};

void NFD_Wayland_Init(void) {
    wayland_display = nullptr;
}

void NFD_Wayland_Quit(void) {
    if (wayland_display) {
        if (wayland_xdg_exporter_v1) zxdg_exporter_v1_destroy(wayland_xdg_exporter_v1);
        wl_registry_destroy(wayland_registry);
    }
}

#endif

}  // namespace

nfdresult_t NFD_SetWaylandDisplay(wl_display* display) {
#ifdef NFD_WAYLAND
    NFD_Wayland_Quit();
    wayland_display = display;
    if (wayland_display) {
        wayland_registry = wl_display_get_registry(wayland_display);
        wayland_xdg_exporter_v1 = nullptr;
        // seems like registry can't be null
        wl_registry_add_listener(wayland_registry, &wayland_registry_listener, nullptr);
        wl_display_roundtrip(wayland_display);
    }
#else
    (void)display;
#endif
    return NFD_OKAY;
}
