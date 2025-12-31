/*
** win32basevideo.cpp
**
** Code to let ZDoom draw to the screen
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2013-2016 Christoph Oelckers
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

#include <windows.h>

#include "hardware.h"

#include "version.h"
#include "c_console.h"
#include "v_video.h"
#include "i_input.h"
#include "i_system.h"
#include "v_text.h"
#include "m_argv.h"
#include "engineerrors.h"
#include "printf.h"
#include "win32basevideo.h"
#include "cmdlib.h"
#include "i_mainwindow.h"

CVAR(Int, vid_adapter, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

//==========================================================================
//
// 
//
//==========================================================================

Win32BaseVideo::Win32BaseVideo()
{
	mainwindow.ShowGameView();

	GetDisplayDeviceName();
}

//==========================================================================
//
// 
//
//==========================================================================

HMONITOR GetPrimaryMonitorHandle()
{
	const POINT ptZero = { 0, 0 };
	return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}

struct MonitorEnumState
{
	int curIdx;
	HMONITOR hFoundMonitor;
};

static BOOL CALLBACK GetDisplayDeviceNameMonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
	MonitorEnumState *state = reinterpret_cast<MonitorEnumState *>(dwData);

	MONITORINFOEX mi;
	mi.cbSize = sizeof mi;
	GetMonitorInfo(hMonitor, &mi);

	// This assumes the monitors are returned by EnumDisplayMonitors in the
	// order they're found in the Direct3D9 adapters list. Fingers crossed...
	if (state->curIdx == vid_adapter || state->hFoundMonitor == nullptr)
	{
		state->hFoundMonitor = hMonitor;

		// Don't stop enumeration; this makes EnumDisplayMonitors fail. I like
		// proper fails.
	}

	++state->curIdx;

	return TRUE;
}

//==========================================================================
//
// 
//
//==========================================================================

void Win32BaseVideo::GetDisplayDeviceName()
{
	// If anything goes wrong, anything at all, everything uses the primary
	// monitor.
	m_DisplayDeviceName = 0;
	m_hMonitor = 0;

	MonitorEnumState mes;

	mes.curIdx = 1;
	mes.hFoundMonitor = nullptr;

	if (vid_adapter == 0)
	{
		mes.hFoundMonitor = GetPrimaryMonitorHandle();
	}

	// Could also use EnumDisplayDevices, I guess. That might work.
	else EnumDisplayMonitors(0, 0, &GetDisplayDeviceNameMonitorEnumProc, LPARAM(&mes));

	if (mes.hFoundMonitor)
	{
		MONITORINFOEXA mi;

		mi.cbSize = sizeof mi;

		if (GetMonitorInfoA(mes.hFoundMonitor, &mi))
		{
			strcpy(m_DisplayDeviceBuffer, mi.szDevice);
			m_DisplayDeviceName = m_DisplayDeviceBuffer;

			m_hMonitor = mes.hFoundMonitor;
		}
	}

}

//==========================================================================
//
// 
//
//==========================================================================

struct DumpAdaptersState
{
	unsigned index;
	char *displayDeviceName;
};

static BOOL CALLBACK DumpAdaptersMonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
	DumpAdaptersState *state = reinterpret_cast<DumpAdaptersState *>(dwData);

	MONITORINFOEXA mi;
	mi.cbSize = sizeof mi;

	char moreinfo[64] = "";

	bool active = true;

	if (GetMonitorInfoA(hMonitor, &mi))
	{
		bool primary = !!(mi.dwFlags & MONITORINFOF_PRIMARY);

		mysnprintf(moreinfo, countof(moreinfo), " [%ldx%ld @ (%ld,%ld)]%s",
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			mi.rcMonitor.left, mi.rcMonitor.top,
			primary ? " (Primary)" : "");

		if (!state->displayDeviceName && !primary)
			active = false;//primary selected, but this ain't primary
		else if (state->displayDeviceName && strcmp(state->displayDeviceName, mi.szDevice) != 0)
			active = false;//this isn't the selected one
	}

	Printf("%s%u. %s\n",
		active ? TEXTCOLOR_BOLD : "",
		state->index,
		moreinfo);

	++state->index;

	return TRUE;
}

//==========================================================================
//
// 
//
//==========================================================================

void Win32BaseVideo::DumpAdapters()
{
	DumpAdaptersState das;

	das.index = 1;
	das.displayDeviceName = m_DisplayDeviceName;

	EnumDisplayMonitors(0, 0, DumpAdaptersMonitorEnumProc, LPARAM(&das));
}

