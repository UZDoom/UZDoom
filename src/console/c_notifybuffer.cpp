/*
** c_notifybuffer.cpp
**
** Implements the buffer for the notification message
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

#include "c_console.h"
#include "vm.h"
#include "gamestate.h"
#include "c_cvars.h"
#include "sbar.h"
#include "v_video.h"
#include "i_time.h"
#include "c_notifybufferbase.h"

struct FNotifyBuffer : public FNotifyBufferBase
{
public:
	void AddString(int printlevel, FString source) override;
	void Clear() override;
	void Draw() override;

};

static FNotifyBuffer NotifyStrings;
static FString       lastNotifyString;

EXTERN_CVAR(Bool, show_messages)
EXTERN_CVAR(Bool, con_stackident)
extern bool generic_ui;
CVAR(Float, con_notifytime, 3.f, CVAR_ARCHIVE)
CVAR(Bool, con_centernotify, false, CVAR_ARCHIVE)
CVAR(Bool, con_pulsetext, false, CVAR_ARCHIVE)

CUSTOM_CVAR(Int, con_scaletext, 0, CVAR_ARCHIVE)		// Scale notify text at high resolutions?
{
	if (self < 0) self = 0;
}

constexpr int NOTIFYFADETIME = 6;
int countedIdentical = 0;

CUSTOM_CVAR(Int, con_notifylines, 4, CVAR_GLOBALCONFIG | CVAR_ARCHIVE)
{
	NotifyStrings.Shift(self);
}

void FNotifyBuffer::Clear()
{
	FNotifyBufferBase::Clear();

	countedIdentical = 0;
	lastNotifyString    = "";

	if (StatusBar == nullptr) return;
	IFVIRTUALPTR(StatusBar, DBaseStatusBar, FlushNotify)
	{
		VMValue params[] = { (DObject*)StatusBar };
		VMCall(func, params, countof(params), nullptr, 1);
	}

}

void FNotifyBuffer::AddString(int printlevel, FString source)
{
	if (!show_messages ||
		source.IsEmpty() ||
		gamestate == GS_FULLCONSOLE ||
		gamestate == GS_DEMOSCREEN ||
		con_notifylines == 0)
		return;

	if (con_stackident && Text.Size() > 0 && source.Compare(lastNotifyString) == 0)
	{
		FNotifyText &last = Text.Last();

		// Only stack if the previous message hasn't started fading out yet
		if (last.Ticker < last.TimeOut)
		{
			countedIdentical++;
			FString combined;

			// Clean up trailing newline
			FString cleanSource = source;
			if (cleanSource.Len() > 0)
			{
				cleanSource.Truncate(cleanSource.Len() - 1);
				combined.Format("%s (x%d)", cleanSource.GetChars(), countedIdentical);
				combined += '\n';
			}

			// Remove the previous entry to recalculate its size
			Text.Pop();

			source = combined;
		}
		else
		{
			// The old message was fading, start a fresh stack
			lastNotifyString = source;
			countedIdentical = 1;
		}
	}
	else
	{
		// Always Brand new message
		lastNotifyString = source;
		countedIdentical = 1;
	}


	// [MK] allow the status bar to take over notify printing
	if (StatusBar != nullptr)
	{
		IFVIRTUALPTR(StatusBar, DBaseStatusBar, ProcessNotify)
		{
			VMValue params[] = { (DObject*)StatusBar, printlevel, &source };
			int rv;
			VMReturn ret(&rv);
			VMCall(func, params, countof(params), &ret, 1);
			if (!!rv) return;
		}
	}

	int width = DisplayWidth / active_con_scaletext(twod, generic_ui);
	FFont *font = generic_ui ? NewSmallFont : AlternativeSmallFont;
	FNotifyBufferBase::AddString(printlevel & PRINT_TYPES, font, source, width, con_notifytime, con_notifylines);
}

void FNotifyBuffer::Draw()
{
	bool center = (con_centernotify != 0.f);
	int line, lineadv, color, j;
	bool canskip;
	
	FFont* font = generic_ui ? NewSmallFont : AlternativeSmallFont;

	line = Top + font->GetDisplacement();
	canskip = true;

	lineadv = font->GetHeight ();

	for (unsigned i = 0; i < Text.Size(); ++ i)
	{
		FNotifyText &notify = Text[i];

		if (notify.TimeOut == 0)
			continue;

		j = notify.TimeOut - notify.Ticker;
		if (j > 0)
		{
			double alpha = (j < NOTIFYFADETIME) ? 1. * j / NOTIFYFADETIME : 1;
			if (con_pulsetext)
			{
				alpha *= 0.7 + 0.3 * sin(I_msTime() / 100.);
			}

			if (notify.PrintLevel >= PRINTLEVELS)
				color = CR_UNTRANSLATED;
			else
				color = PrintColors[notify.PrintLevel];

			int scale = active_con_scaletext(twod, generic_ui);
			if (!center)
				DrawText(twod, font, color, 0, line, notify.Text.GetChars(),
					DTA_VirtualWidth, twod->GetWidth() / scale,
					DTA_VirtualHeight, twod->GetHeight() / scale,
					DTA_KeepRatio, true,
					DTA_Alpha, alpha, TAG_DONE);
			else
				DrawText(twod, font, color, (twod->GetWidth() -
					font->StringWidth (notify.Text) * scale) / 2 / scale,
					line, notify.Text.GetChars(),
					DTA_VirtualWidth, twod->GetWidth() / scale,
					DTA_VirtualHeight, twod->GetHeight() / scale,
					DTA_KeepRatio, true,
					DTA_Alpha, alpha, TAG_DONE);
			line += lineadv;
			canskip = false;
		}
		else
		{
			notify.TimeOut = 0;
		}
	}
	if (canskip)
	{
		Top = TopGoal;
	}
}

void SetConsoleNotifyBuffer()
{
	C_SetNotifyBuffer(&NotifyStrings);
}
