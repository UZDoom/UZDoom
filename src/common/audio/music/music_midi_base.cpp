/*
** music_midi_base.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2020 Christoph Oelckers
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

#include "c_dispatch.h"

#include "v_text.h"
#include "menu.h"
#include <zmusic.h>
#include "s_music.h"
#include "c_cvars.h"
#include "printf.h"

#define DEF_MIDIDEV -5

EXTERN_CVAR(Int, snd_mididevice)

void I_BuildMIDIMenuList(FOptionValues* opt, DMenuDescriptor* menu)
{
	int amount;
	auto list = ZMusic_GetMidiDevices(&amount);

	for (int i = 0; i < amount; i++)
	{
		if (opt)
		{
			FOptionValues::Pair* pair = &opt->mValues[opt->mValues.Reserve(1)];
			pair->Text = list[i].Name;
			pair->Value = (float)list[i].ID;
		}
		if (menu)
		{
			auto it = CreateOptionMenuItemCommand(list[i].Name, FStringf("snd_mididevice %d", list[i].ID), true);
			static_cast<DOptionMenuDescriptor*>(menu)->mItems.Push(it);
		}
	}
}

static void PrintMidiDevice (int id, const char *name, uint16_t tech, uint32_t support)
{
	if (id == snd_mididevice)
	{
		Printf (TEXTCOLOR_BOLD);
	}
	Printf ("% 2d. %s : ", id, name);
	switch (tech)
	{
	case MIDIDEV_MIDIPORT:		Printf ("MIDIPORT");		break;
	case MIDIDEV_SYNTH:			Printf ("SYNTH");			break;
	case MIDIDEV_SQSYNTH:		Printf ("SQSYNTH");			break;
	case MIDIDEV_FMSYNTH:		Printf ("FMSYNTH");			break;
	case MIDIDEV_MAPPER:		Printf ("MAPPER");			break;
	case MIDIDEV_WAVETABLE:		Printf ("WAVETABLE");		break;
	case MIDIDEV_SWSYNTH:		Printf ("SWSYNTH");			break;
	}
	Printf (TEXTCOLOR_NORMAL "\n");
}

CCMD (snd_listmididevices)
{
	int amount;
	auto list = ZMusic_GetMidiDevices(&amount);

	for (int i = 0; i < amount; i++)
	{
		PrintMidiDevice(list[i].ID, list[i].Name, list[i].Technology, 0);
	}
}


CUSTOM_CVAR (Int, snd_mididevice, DEF_MIDIDEV, CVAR_ARCHIVE|CVAR_GLOBALCONFIG|CVAR_NOINITCALL)
{
	int amount;
	auto list = ZMusic_GetMidiDevices(&amount);

	bool found = false;
	// The list is not necessarily contiguous so we need to check each entry.
	for (int i = 0; i < amount; i++)
	{
		if (self == list[i].ID)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		// Don't do repeated message spam if there is no valid device.
		if (self != 0 && self != -1)
		{
			Printf("ID out of range. Using default device.\n");
		}
		if (self != DEF_MIDIDEV) self = DEF_MIDIDEV;
		return;
	}
	bool change = ChangeMusicSetting(zmusic_snd_mididevice, nullptr, self);
	if (change) S_MIDIDeviceChanged(self);
}
