/*
** hardware.cpp
**
** Somewhat OS-independent interface to the screen, mouse, keyboard, and stick
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "hardware.h"
#include "c_dispatch.h"
#include "v_text.h"
#include "basics.h"
#include "m_argv.h"
#include "version.h"
#include "printf.h"
#include "win32glvideo.h"
#ifdef HAVE_VULKAN
#include "win32vulkanvideo.h"
#endif
#include "engineerrors.h"
#include "i_system.h"
#include "i_mainwindow.h"

IVideo *Video;

// do not include GL headers here, only declare the necessary functions.
IVideo *gl_CreateVideo();

void I_RestartRenderer();
int currentcanvas = -1;
bool changerenderer;

void I_ShutdownGraphics ()
{
	if (screen)
	{
		DFrameBuffer *s = screen;
		screen = NULL;
		delete s;
	}
	if (Video)
		delete Video, Video = NULL;
}

void I_InitGraphics ()
{
	// If the focus window is destroyed, it doesn't go back to the active window.
	// (e.g. because the net pane was up, and a button on it had focus)
	if (GetFocus() == NULL && GetActiveWindow() == mainwindow.GetHandle())
	{
		// Make sure it's in the foreground and focused. (It probably is
		// already foregrounded but may not be focused.)
		SetForegroundWindow(mainwindow.GetHandle());
		SetFocus(mainwindow.GetHandle());
		// Note that when I start a 2-player game on the same machine, the
		// window for the game that isn't focused, active, or foregrounded
		// still receives a WM_ACTIVATEAPP message telling it that it's the
		// active window. The window that is really the active window does
		// not receive a WM_ACTIVATEAPP message, so both games think they
		// are the active app. Huh?
	}

#ifdef HAVE_VULKAN
	if (V_GetBackend() == 1)
	{
		// first try Vulkan, if that fails OpenGL
		try
		{
			Video = new Win32VulkanVideo();
		}
		catch (CVulkanError &error)
		{
			Printf(TEXTCOLOR_RED "Initialization of Vulkan failed: %s\n", error.what());
			Video = new Win32GLVideo();
		}
	}
	else
#endif
	{
		Video = new Win32GLVideo();
	}

	// we somehow STILL don't have a display!!
	if (Video == NULL)
		I_FatalError ("Failed to initialize display");

}
