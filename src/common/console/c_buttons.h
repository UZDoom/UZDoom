/*
** c_buttons.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2017 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <stdint.h>
#include "tarray.h"
#include "name.h"
#include "keydef.h"

// Actions
struct FButtonStatus
{
	enum { MAX_KEYS = 6 };	// Maximum number of keys that can press this button

	uint16_t Keys[MAX_KEYS];
	bool bDown;				// Button is down right now
	bool bWentDown;			// Button went down this tic
	bool bWentUp;			// Button went up this tic
	bool bReleaseLock;		// Lock ReleaseKey call in ResetButtonStates

	bool bIsAxis;	// Whenever or not this button is being controlled by any axis.
	float Axis;		// How far the button has been pressed. Updated by I_GetAxes.

	void (*PressHandler)();		// for optional game-side customization
	void (*ReleaseHandler)();

	bool PressKey (int keynum);		// Returns true if this key caused the button to be pressed.
	bool ReleaseKey (int keynum);	// Returns true if this key is no longer pressed.
	void AddAxes (FString &btn_name, float joyaxes[NUM_AXIS_CODES]); // Update joystick axis information.
	void ResetTriggers () { bWentDown = bWentUp = false; }
	void Reset () { bDown = bWentDown = bWentUp = bIsAxis = false; Axis = 0.0f; }
};

class ButtonMap
{

	TArray<FButtonStatus> Buttons;
	TArray<FString> NumToName;		// The internal name of the button
	TMap<FName, int> NameToNum;

public:
	void SetButtons(const char** names, int count);

	int NumButtons() const
	{
		return Buttons.Size();
	}

	int FindButtonIndex(const char* func, int funclen = -1) const;

	FButtonStatus* FindButton(const char* func, int funclen = -1)
	{
		int index = FindButtonIndex(func, funclen);
		return index > -1 ? &Buttons[index] : nullptr;
	}

	FButtonStatus* GetButton(int index)
	{
		return &Buttons[index];
	}

	void ResetButtonTriggers();	// Call ResetTriggers for all buttons
	void ResetButtonStates();		// Same as above, but also clear bDown
	void GetAxes();				// Call AddAxes for all buttons
	int ListActionCommands(const char* pattern);
	void AddButtonTabCommands();


	bool ButtonDown(int x) const
	{
		return Buttons[x].bDown;
	}

	bool ButtonDownDigital(int x) const
	{
		// Like ButtonDown, but only for digital buttons.
		// This is necessary, because of how the game runs different
		// code for both digital and analog, and they can't be rolled
		// together without introducing some subtle gameplay differences.
		return (Buttons[x].bIsAxis == false && Buttons[x].bDown == true);
	}

	float ButtonAnalog(int x) const
	{
		// Get the analog value of a button.
		// This is 0.0 (not 1.0) for digital presses, because again,
		// of how the game logic is split between the two.
		if (Buttons[x].bIsAxis == true)
		{
			return Buttons[x].Axis;
		}

		return 0.0f;
	}

	bool ButtonPressed(int x) const
	{
		return Buttons[x].bWentDown;
	}

	bool ButtonReleased(int x) const
	{
		return Buttons[x].bWentUp;
	}

	void ButtonSet(int x) const
	{
		Buttons[x].bDown = Buttons[x].bWentDown = true;
		Buttons[x].bWentUp = false;
	}

	void ClearButton(int x)
	{
		Buttons[x].Reset();
	}
};

extern ButtonMap buttonMap;
