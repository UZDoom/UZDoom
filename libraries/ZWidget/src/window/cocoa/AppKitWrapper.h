#pragma once

#ifdef __APPLE__

#define Point MacPoint
#define Rect MacRect
#define Size MacSize

#import <AppKit/AppKit.h>

#undef Point
#undef Rect
#undef Size

#endif
