/*
** c_buttons.cpp
**
** Functions for executing console commands and aliases
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2019 Christoph Oelckers
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

#include "c_buttons.h"

#include "c_dispatch.h"
#include "printf.h"
#include "cmdlib.h"
#include "c_console.h"
#include "m_joy.h"
#include "c_bind.h"

ButtonMap buttonMap;


//=============================================================================
//
//
//
//=============================================================================

void ButtonMap::SetButtons(const char** names, int count)
{
	Buttons.Resize(count);
	NumToName.Resize(count);
	NameToNum.Clear();
	for(int i = 0; i < count; i++)
	{
		Buttons[i] = {};
		NameToNum.Insert(names[i], i);
		NumToName[i] = names[i];
	}
}

//=============================================================================
//
//
//
//=============================================================================

int ButtonMap::ListActionCommands (const char *pattern)
{
	char matcher[32];
	int count = 0;

	for (auto& btn : NumToName)
	{
		if (pattern == NULL || CheckWildcards (pattern,
			(mysnprintf (matcher, countof(matcher), "+%s", btn.GetChars()), matcher)))
		{
			Printf ("+%s\n", btn.GetChars());
			count++;
		}
		if (pattern == NULL || CheckWildcards (pattern,
			(mysnprintf (matcher, countof(matcher), "-%s", btn.GetChars()), matcher)))
		{
			Printf ("-%s\n", btn.GetChars());
			count++;
		}
	}
	return count;
}


//=============================================================================
//
//
//
//=============================================================================

int ButtonMap::FindButtonIndex (const char *key, int funclen) const
{
    if (!key) return -1;

	FName name = funclen == -1? FName(key, true) : FName(key, funclen, true);
	if (name == NAME_None) return -1;

	auto res = NameToNum.CheckKey(name);
	if (!res) return -1;

	return *res;
}


//=============================================================================
//
//
//
//=============================================================================

void ButtonMap::ResetButtonTriggers ()
{
	for (auto &button : Buttons)
	{
		button.ResetTriggers ();
	}
}

//=============================================================================
//
//
//
//=============================================================================

void ButtonMap::ResetButtonStates ()
{
	for (auto &btn : Buttons)
	{
		if (!btn.bReleaseLock) 
		{
			btn.ReleaseKey (0);
		}
		btn.ResetTriggers ();
	}
}

//=============================================================================
//
//
//
//=============================================================================

void ButtonMap::GetAxes ()
{
	float joyaxes[NUM_AXIS_CODES];
	I_GetAxes(joyaxes);

	for (unsigned i = 0; i < Buttons.Size(); i++)
	{
		FButtonStatus &btn = Buttons[i];
		FString &btn_name = NumToName[i];

		btn.AddAxes(btn_name, joyaxes);
	}
}

//=============================================================================
//
//
//
//=============================================================================

bool FButtonStatus::PressKey (int keynum)
{
	int i, open;

	keynum &= KEY_DBLCLICKED-1;

	if (keynum == 0)
	{ // Issued from console instead of a key, so force on
		Keys[0] = 0xffff;
		for (i = MAX_KEYS-1; i > 0; --i)
		{
			Keys[i] = 0;
		}
	}
	else
	{
		for (i = MAX_KEYS-1, open = -1; i >= 0; --i)
		{
			if (Keys[i] == 0)
			{
				open = i;
			}
			else if (Keys[i] == keynum)
			{ // Key is already down; do nothing
				return false;
			}
		}
		if (open < 0)
		{ // No free key slots, so do nothing
			Printf ("More than %u keys pressed for a single action!\n", MAX_KEYS);
			return false;
		}
		Keys[open] = keynum;
	}
	uint8_t wasdown = bDown;
	bDown = bWentDown = true;
	// Returns true if this key caused the button to go down.
	return !wasdown;
}

//=============================================================================
//
//
//
//=============================================================================

bool FButtonStatus::ReleaseKey (int keynum)
{
	int i, numdown, match;
	uint8_t wasdown = bDown;

	keynum &= KEY_DBLCLICKED-1;

	if (keynum == 0)
	{ // Issued from console instead of a key, so force off
		for (i = MAX_KEYS-1; i >= 0; --i)
		{
			Keys[i] = 0;
		}
		bWentUp = true;
		bDown = false;
	}
	else
	{
		for (i = MAX_KEYS-1, numdown = 0, match = -1; i >= 0; --i)
		{
			if (Keys[i] != 0)
			{
				++numdown;
				if (Keys[i] == keynum)
				{
					match = i;
				}
			}
		}
		if (match < 0)
		{ // Key was not down; do nothing
			return false;
		}
		Keys[match] = 0;
		bWentUp = true;
		if (--numdown == 0)
		{
			bDown = false;
		}
	}
	// Returns true if releasing this key caused the button to go up.
	return wasdown && !bDown;
}

//=============================================================================
//
//
//
//=============================================================================

void FButtonStatus::AddAxes (FString &btn_name, float joyaxes[NUM_AXIS_CODES])
{
	int i;

	bIsAxis = false;
	Axis = 0.0f;

	char cmd_name[16];
	strcpy(&cmd_name[1], btn_name.GetChars());

	cmd_name[0] = '+';
	TArray<int> positive_keys = Bindings.GetKeysForCommand(cmd_name);

	cmd_name[0] = '-';
	TArray<int> negative_keys = Bindings.GetKeysForCommand(cmd_name);

	for (i = 0; i < NUM_AXIS_CODES; i++)
	{
		float axis_value = joyaxes[i];

		if (axis_value > 0.0)
		{
			int key_code = KeyAxisMapping[i];

			if (positive_keys.Contains(key_code))
			{
				Axis += axis_value;
				bIsAxis = true;
			}

			if (negative_keys.Contains(key_code))
			{
				Axis -= axis_value;
				bIsAxis = true;
			}
		}
	}

	Axis = clamp<float>(Axis, 0.0f, 1.0f);
}

//=============================================================================
//
//
//
//=============================================================================

void ButtonMap::AddButtonTabCommands()
{
	// Add all the action commands for tab completion
	for (auto& btn : NumToName)
	{
		char tname[16];
		strcpy (&tname[1], btn.GetChars());
		tname[0] = '+';
		C_AddTabCommand (tname);
		tname[0] = '-';
		C_AddTabCommand (tname);
	}
}
