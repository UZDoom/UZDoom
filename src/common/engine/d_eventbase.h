/*
** d_eventbase.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
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

#pragma once
#include <stdint.h>
#include "d_gui.h"
#include "zstring.h"

// Input event types.
enum EGenericEvent
{
	EV_None,
	EV_KeyDown,		// data1: scan code, data2: Qwerty ASCII code
	EV_KeyUp,		// same
	EV_Mouse,		// x, y: mouse movement deltas
	EV_GUI_Event,	// subtype specifies actual event
	EV_DeviceChange,// a device has been connected or removed
};

// Event structure.
struct event_t
{
	uint8_t		type;
	uint8_t		subtype;
	int16_t 	data1;		// keys / mouse/joystick buttons
	int16_t		data2;
	int16_t		data3;
	float 		x;			// mouse/joystick x move
	float 		y;			// mouse/joystick y move
};

// Ignore minor mouse movements if it hasn't updated in a while.
struct mousestate_t
{
	TArray<EGUIEvent> HeldButtons = {};
	int LastUpdate = -1;
	float LastX = 0.0f;
	float LastY = 0.0f;
};



// Called by IO functions when input is detected.
void D_PostEvent (event_t* ev);
void D_RemoveNextCharEvent();
void D_ProcessEvents(void);
void PostMouseMove(int x, int y);

enum
{
	MAXEVENTS = 128
};

extern	event_t 		events[MAXEVENTS];
extern mousestate_t		LastMousePos;
extern int eventhead;
extern int eventtail;

struct FUiEvent
{
	// this essentially translates event_t UI events to ZScript.
	EGUIEvent Type;
	// for keys/chars/whatever
	FString KeyString;
	int KeyChar;
	// for mouse
	int MouseX;
	int MouseY;
	// global (?)
	bool IsShift;
	bool IsCtrl;
	bool IsAlt;

	FUiEvent(const event_t *ev);
};

struct FInputEvent
{
	// this translates regular event_t events to ZScript (not UI, UI events are sent via DUiEvent and only if requested!)
	EGenericEvent Type = EV_None;
	// for keys
	int KeyScan;
	FString KeyString;
	int KeyChar;
	// for mouse
	int MouseX;
	int MouseY;

	FInputEvent(const event_t *ev);
};

