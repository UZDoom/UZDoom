/*
** i_system.mm
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2011-2016 Christoph Oelckers
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

#include <CoreFoundation/CoreFoundation.h>
#include "SDL.h"

void Mac_I_FatalError(const char* errortext)
{
	// Close window or exit fullscreen and release mouse capture
	SDL_Quit();

	const CFStringRef errorString = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, 
		errortext, kCFStringEncodingASCII, kCFAllocatorNull );
	if ( NULL != errorString )
	{
		CFOptionFlags dummy;

		CFUserNotificationDisplayAlert( 0, kCFUserNotificationStopAlertLevel, NULL, NULL, NULL, 
			CFSTR( "Fatal Error" ), errorString, CFSTR( "Exit" ), NULL, NULL, &dummy );
		CFRelease( errorString );
	}
}
