/*
** c_console.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
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

#ifndef __C_CONSOLE__
#define __C_CONSOLE__

#include <stdarg.h>
#include "basics.h"
#include "c_tabcomplete.h"
#include "textureid.h"

struct event_t;

enum cstate_t : uint8_t
{
	c_up=0, c_down=1, c_falling=2, c_rising=3
};

enum
{
	PRINTLEVELS = 5
};
extern int PrintColors[PRINTLEVELS + 2];

extern uint8_t ConsoleState;

// Initialize the console
void C_InitConsole (int width, int height, bool ingame);
void C_DeinitConsole ();
void C_InitConback(FTextureID fallback, bool tile, double lightlevel = 1.);

// Adjust the console for a new screen mode
void C_NewModeAdjust (void);

void C_Ticker (void);

void AddToConsole (int printlevel, const char *string);
int PrintString (int printlevel, const char *string);
int PrintStringHigh (const char *string);
int VPrintf (int printlevel, const char *format, va_list parms) GCCFORMAT(2);

void C_DrawConsole ();
void C_ToggleConsole (void);
void C_FullConsole (void);
void C_HideConsole (void);
void C_AdjustBottom (void);
void C_FlushDisplay (void);
class FNotifyBufferBase;
void C_SetNotifyBuffer(FNotifyBufferBase *nbb);


bool C_Responder (event_t *ev);

extern double NotifyFontScale;
void C_SetNotifyFontScale(double scale);

extern const char *console_bar;
extern int chatmodeon;

#endif
