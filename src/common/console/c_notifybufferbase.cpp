/*
** c_notifybufferbase.cpp
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

#include "v_text.h"
#include "i_time.h"
#include "v_draw.h"
#include "c_notifybufferbase.h"


void FNotifyBufferBase::Shift(int maxlines)
{
	if (maxlines >= 0 && Text.Size() > (unsigned)maxlines)
	{
		Text.Delete(0, Text.Size() - maxlines);
	}
}

void FNotifyBufferBase::Clear() 
{ 
	Text.Clear(); 
}


void FNotifyBufferBase::AddString(int printlevel, FFont *printFont, const FString &source, int formatwidth, float keeptime, int maxlines)
{
	if (printFont == nullptr) return;	// Without an initialized font we cannot handle the message (this is for those which come here before the font system is ready.)
	LineHeight = printFont->GetHeight();
	TArray<FBrokenLines> lines;

	if (AddType == APPENDLINE && Text.Size() > 0 && Text[Text.Size() - 1].PrintLevel == printlevel)
	{
		FString str = Text[Text.Size() - 1].Text + source;
		lines = V_BreakLines (printFont, formatwidth, str);
	}
	else
	{
		lines = V_BreakLines (printFont, formatwidth, source);
		if (AddType == APPENDLINE)
		{
			AddType = NEWLINE;
		}
	}

	if (lines.Size() == 0)
		return;

	for (auto &line : lines)
	{
		FNotifyText newline;

		newline.Text = line.Text;
		newline.TimeOut = int(keeptime * GameTicRate);
		newline.Ticker = 0;
		newline.PrintLevel = printlevel;
		if (AddType == NEWLINE || Text.Size() == 0)
		{
			if (maxlines > 0)
			{
				Shift(maxlines - 1);
			}
			Text.Push(newline);
		}
		else
		{
			Text[Text.Size() - 1] = newline;
		}
		AddType = NEWLINE;
	}

	switch (source[source.Len()-1])
	{
	case '\r':	AddType = REPLACELINE;	break;
	case '\n':	AddType = NEWLINE;		break;
	default:	AddType = APPENDLINE;	break;
	}

	TopGoal = 0;
}

void FNotifyBufferBase::Tick()
{
	if (TopGoal > Top)
	{
		Top++;
	}
	else if (TopGoal < Top)
	{
		Top--;
	}

	// Remove lines from the beginning that have expired.
	unsigned i;
	for (i = 0; i < Text.Size(); ++i)
	{
		Text[i].Ticker++;
	}

	for (i = 0; i < Text.Size(); ++i)
	{
		if (Text[i].TimeOut != 0 && Text[i].TimeOut > Text[i].Ticker)
			break;
	}
	if (i > 0)
	{
		Text.Delete(0, i);
		Top += LineHeight;
	}
}

