/*
	ZVulkan include header for vulkan.
	Include this one instead of the system vulkan.h header.

	We can't use the system header because we don't want to include platform specific headers (windows.h and xlib.h in particular).
	On Linux we even have many competing platform technologies that may not always be available on the system building the project.
*/

#ifndef VULKAN_H_
#define VULKAN_H_

#define VK_NO_PROTOTYPES // Volk needs this

// Declare what we intend to support for each OS:

#if defined(_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
	#define VK_USE_PLATFORM_MACOS_MVK
	#define VK_USE_PLATFORM_METAL_EXT
#else
	#define VK_USE_PLATFORM_XLIB_KHR
	#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include "vulkan/vk_platform.h"
#include "vulkan/vulkan_core.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan/vulkan_android.h"
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
#include <zircon/types.h>
#include "vulkan/vulkan_fuchsia.h"
#endif

#ifdef VK_USE_PLATFORM_IOS_MVK
#include "vulkan/vulkan_ios.h"
#endif


#ifdef VK_USE_PLATFORM_MACOS_MVK
#include "vulkan/vulkan_macos.h"
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
#include "vulkan/vulkan_metal.h"
#endif

#ifdef VK_USE_PLATFORM_VI_NN
#include "vulkan/vulkan_vi.h"
#endif


#ifdef VK_USE_PLATFORM_WAYLAND_KHR

//#include <wayland-client.h>
struct wl_display;
struct wl_surface;

#include "vulkan/vulkan_wayland.h"
#endif


#ifdef VK_USE_PLATFORM_WIN32_KHR

// #include <windows.h>
typedef unsigned long DWORD;
typedef const wchar_t* LPCWSTR;
typedef void* HANDLE;
typedef struct HINSTANCE__* HINSTANCE;
typedef struct HWND__* HWND;
typedef struct HMONITOR__* HMONITOR;
typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;

#include "vulkan/vulkan_win32.h"
#endif


#ifdef VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#include "vulkan/vulkan_xcb.h"
#endif


#ifdef VK_USE_PLATFORM_XLIB_KHR

// #include <X11/Xlib.h>
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef unsigned long VisualID;
typedef XID Window;

#include "vulkan/vulkan_xlib.h"
#endif


#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
#include <directfb.h>
#include "vulkan/vulkan_directfb.h"
#endif


#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "vulkan/vulkan_xlib_xrandr.h"
#endif


#ifdef VK_USE_PLATFORM_GGP
#include <ggp_c/vulkan_types.h>
#include "vulkan/vulkan_ggp.h"
#endif


#ifdef VK_USE_PLATFORM_SCREEN_QNX
#include <screen/screen.h>
#include "vulkan/vulkan_screen.h"
#endif

#ifdef VK_ENABLE_BETA_EXTENSIONS
#include "vulkan/vulkan_beta.h"
#endif

#endif
