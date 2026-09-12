/*
** stats.cpp
**
** Performance-monitoring statistics
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2008-2016 Christoph Oelckers
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

#include "basics.h"
#include "c_dispatch.h"
#include "printf.h"
#include "stats.h"
#include "v_draw.h"
#include "v_font.h"

FStat *FStat::FirstStat;

FStat::FStat (const char *name)
{
	m_Name = name;
	m_Active = false;
	m_Next = FirstStat;
	FirstStat = this;
}

FStat::~FStat ()
{
	FStat **prev = &FirstStat;

	while (*prev && *prev != this)
		prev = &((*prev)->m_Next)->m_Next;

	if (*prev == this)
		*prev = m_Next;
}

FStat *FStat::FindStat (const char *name)
{
	FStat *stat = FirstStat, *partial = nullptr;
	bool badpartial = false;
	auto len = strlen(name);

	while (stat && stricmp (name, stat->m_Name))
	{
		if (!badpartial && 0 == strncmp(stat->m_Name, name, len))
		{
			if (partial) { badpartial = true; partial = nullptr; }
			else { partial = stat; }
		}
		stat = stat->m_Next;
	}

	return stat? stat: (badpartial ? nullptr: partial);
}

void FStat::ToggleStat (const char *name)
{
	FStat *stat = FindStat (name);
	if (stat)
		stat->ToggleStat ();
	else
		Printf ("Unknown stat: %s\n", name);
}

void FStat::EnableStat(const char* name, bool on)
{
	FStat* stat = FindStat(name);
	if (stat)
	{
		if(stat->m_Active != on) stat->ToggleStat();
	}
	else
	{
		Printf("Unknown stat: %s\n", name);
	}
}

void FStat::ToggleStat ()
{
	m_Active = !m_Active;

	if(m_Active)
	{
		OnEnable();
	}
	else
	{
		OnDisable();
	}
}

void FStat::PrintStat (F2DDrawer *drawer)
{
	int y = drawer->GetHeight();

	for (FStat *stat = FirstStat; stat != NULL; stat = stat->m_Next)
	{
		if (stat->m_Active)
		{
			y -= stat->DrawStat(drawer, y);
		}
	}
}

double FTextStat::DrawStat(F2DDrawer *drawer, double yoffset_bottom)
{
	int textScale = active_con_scale(drawer);
	int fontheight = NewConsoleFont->GetHeight() + 1;

	double y = yoffset_bottom / textScale;

	FString stattext(GetStats());

	if (stattext.Len() > 0)
	{
		y -= fontheight;	// there's at least one line of text
		for (unsigned i = 0; i < stattext.Len()-1; i++)
		{
			// Count number of linefeeds but ignore terminating ones.
			if (stattext[i] == '\n') y -= fontheight;
		}
		DrawText(drawer, NewConsoleFont, CR_GREEN, 5. / textScale, y, stattext.GetChars(),
			DTA_VirtualWidth, drawer->GetWidth() / textScale,
			DTA_VirtualHeight, drawer->GetHeight() / textScale,
			DTA_KeepRatio, true, TAG_DONE);
	}
	return yoffset_bottom - (y * textScale);
}

void FStat::DumpRegisteredStats ()
{
	FStat *stat = FirstStat;

	Printf ("Available stats:\n");
	while (stat)
	{
		Printf (" %c%s\n", stat->m_Active ? '*' : ' ', stat->m_Name);
		stat = stat->m_Next;
	}
}

CCMD (stat)
{
	if (argv.argc() != 2)
	{
		Printf ("Usage: stat <statistics>\n");
		FStat::DumpRegisteredStats ();
	}
	else
	{
		FStat::ToggleStat (argv[1]);
	}
}
