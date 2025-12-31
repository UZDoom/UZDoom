/*
** i_common.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2012-2018 Alexey Lysiuk
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#ifndef COCOA_I_COMMON_INCLUDED
#define COCOA_I_COMMON_INCLUDED

#import <AppKit/AppKit.h>


// Version of AppKit framework we are interested in
// The following values are needed to build with earlier SDKs

#define AppKit10_7 1138
#define AppKit10_8 1187
#define AppKit10_9 1265


@interface NSWindow(ExitAppOnClose)
- (void)exitAppOnClose;
@end


void I_ProcessEvent(NSEvent* event);

void I_ProcessJoysticks();

NSSize I_GetContentViewSize(const NSWindow* window);
void I_SetMainWindowVisible(bool visible);
void I_SetNativeMouse(bool wantNative);

#endif // COCOA_I_COMMON_INCLUDED
