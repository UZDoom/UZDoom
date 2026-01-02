/*
** base_sysfb.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2003-2005 Tim Stump
** Copyright 2005-2016 Christoph Oelckers
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

#include "m_argv.h"
#include "v_video.h"

EXTERN_FARG(0);

class SystemBaseFrameBuffer : public DFrameBuffer
{
	typedef DFrameBuffer Super;

	void SaveWindowedPos();
	void RestoreWindowedPos();

public:
	SystemBaseFrameBuffer();
	// Actually, hMonitor is a HMONITOR, but it's passed as a void * as there
    // look to be some cross-platform bits in the way.
	SystemBaseFrameBuffer(void *hMonitor, bool fullscreen);
	virtual ~SystemBaseFrameBuffer();

	int GetClientWidth() override;
	int GetClientHeight() override;

	bool IsFullscreen() override;
	void ToggleFullscreen(bool yes) override;
	void SetWindowSize(int client_w, int client_h);

protected:

	void GetCenteredPos(int in_w, int in_h, int &winx, int &winy, int &winw, int &winh, int &scrwidth, int &scrheight);
	void KeepWindowOnScreen(int &winx, int &winy, int winw, int winh, int scrwidth, int scrheight);

	void PositionWindow(bool fullscreen, bool initialcall = false);

	bool m_Fullscreen = false;
	char m_displayDeviceNameBuffer[32/*CCHDEVICENAME*/];	// do not use windows.h constants here!
	char *m_displayDeviceName;
	void *m_Monitor;
};
