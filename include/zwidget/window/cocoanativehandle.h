#pragma once

#ifdef __OBJC__
@class NSWindow;
@class NSView;
@class CAMetalLayer;
#else
// Forward declarations for C++ - use opaque pointers to avoid conflicts with Metal headers
typedef struct NSWindow_t* NSWindow;
typedef struct NSView_t* NSView;

// CAMetalLayer may already be defined by vulkan_metal.h, so guard against redefinition
#ifndef VK_EXT_metal_surface
typedef struct CAMetalLayer_t* CAMetalLayer;
#endif
#endif


struct CocoaNativeHandle
{
	NSWindow* nsWindow = nullptr;
	NSView* nsView = nullptr;

	// Use void* to avoid typedef conflicts when CAMetalLayer is defined differently by Vulkan headers
	void* metalLayer = nullptr;
};