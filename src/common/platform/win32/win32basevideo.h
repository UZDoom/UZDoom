/*
** win32basevideo.h
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

#pragma once

#include "v_video.h"

//==========================================================================
//
// 
//
//==========================================================================

class Win32BaseVideo : public IVideo
{
public:
	Win32BaseVideo();

	void DumpAdapters();

	HDC m_hDC;

protected:
	HMODULE hmRender;

	char m_DisplayDeviceBuffer[CCHDEVICENAME];
	char *m_DisplayDeviceName;
	HMONITOR m_hMonitor;

	HWND m_Window;

	void GetDisplayDeviceName();
public:
	virtual void Shutdown() = 0;

};
