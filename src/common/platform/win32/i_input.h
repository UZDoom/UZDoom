/*
** i_input.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __I_INPUT_H__
#define __I_INPUT_H__

#include "basics.h"

bool I_InitInput (void *hwnd);
void I_ShutdownInput ();

void I_GetWindowEvent();

void I_GetEvent();

enum
{
	INPUT_DIJoy,
	INPUT_XInput,
	INPUT_RawPS2,
	NUM_JOYDEVICES
};


#ifdef _WIN32
#include "m_joy.h"

// Don't make these definitions available to the main body of the source code.


struct tagRAWINPUT;

class FInputDevice
{
public:
	virtual ~FInputDevice() = 0;
	virtual bool GetDevice() = 0;
	virtual void ProcessInput();
	virtual bool ProcessRawInput(tagRAWINPUT *raw, int code);
	virtual bool WndProcHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result);
};

class FMouse : public FInputDevice
{
public:
	FMouse();

	virtual void Grab() = 0;
	virtual void Ungrab() = 0;

protected:
	void WheelMoved(int axis, int wheelmove);
	void PostButtonEvent(int button, bool down);
	void ClearButtonState();

	int WheelMove[2];
	int ButtonState;	// bit mask of current button states (1=down, 0=up)
};

class FKeyboard : public FInputDevice
{
public:
	FKeyboard();
	~FKeyboard();

	void AllKeysUp();

protected:
	uint8_t KeyStates[256/8];

	int CheckKey(int keynum) const
	{
		return KeyStates[keynum >> 3] & (1 << (keynum & 7));
	}
	void SetKey(int keynum, bool down)
	{
		if (down)
		{
			KeyStates[keynum >> 3] |= 1 << (keynum & 7);
		}
		else
		{
			KeyStates[keynum >> 3] &= ~(1 << (keynum & 7));
		}
	}
	bool CheckAndSetKey(int keynum, INTBOOL down);
	void PostKeyEvent(int keynum, INTBOOL down, bool foreground);
};

class FJoystickCollection : public FInputDevice
{
public:
	virtual void AddAxes(float axes[NUM_AXIS_CODES]) = 0;
	virtual void GetDevices(TArray<IJoystickConfig *> &sticks) = 0;
	virtual IJoystickConfig *Rescan() = 0;
};

extern FJoystickCollection *JoyDevices[NUM_JOYDEVICES];

void I_StartupMouse();
void I_CheckNativeMouse(bool prefer_native, bool eh);
void I_StartupKeyboard();
void I_StartupXInput();
void I_StartupDirectInputJoystick();
void I_StartupRawPS2();
bool I_IsPS2Adapter(DWORD vidpid);

// USB HID usage page numbers
#define HID_GENERIC_DESKTOP_PAGE			0x01
#define HID_SIMULATION_CONTROLS_PAGE		0x02
#define HID_VR_CONTROLS_PAGE				0x03
#define HID_SPORT_CONTROLS_PAGE				0x04
#define HID_GAME_CONTROLS_PAGE				0x05
#define HID_GENERIC_DEVICE_CONTROLS_PAGE	0x06
#define HID_KEYBOARD_PAGE					0x07
#define HID_LED_PAGE						0x08
#define HID_BUTTON_PAGE						0x09
#define HID_ORDINAL_PAGE					0x0a
#define HID_TELEPHONY_DEVICE_PAGE			0x0b
#define HID_CONSUMER_PAGE					0x0c
#define HID_DIGITIZERS_PAGE					0x0d
#define HID_UNICODE_PAGE					0x10
#define HID_ALPHANUMERIC_DISPLAY_PAGE		0x14
#define HID_MEDICAL_INSTRUMENT_PAGE			0x40

// HID Generic Desktop Page usages
#define HID_GDP_UNDEFINED					0x00
#define HID_GDP_POINTER						0x01
#define HID_GDP_MOUSE						0x02
#define HID_GDP_JOYSTICK					0x04
#define HID_GDP_GAMEPAD						0x05
#define HID_GDP_KEYBOARD					0x06
#define HID_GDP_KEYPAD						0x07
#define HID_GDP_MULTIAXIS_CONTROLLER		0x08
#endif


#endif
