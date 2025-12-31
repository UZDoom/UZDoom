/*
** gl_sysfb.cpp
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "wglext.h"

#include "gl_sysfb.h"
#include "hardware.h"

#include "version.h"
#include "c_console.h"
#include "v_video.h"
#include "i_input.h"
#include "i_system.h"
#include "v_text.h"
#include "m_argv.h"
#include "engineerrors.h"
#include "win32glvideo.h"
#include "i_mainwindow.h"

extern "C" PROC zd_wglGetProcAddress(LPCSTR name);

PFNWGLSWAPINTERVALEXTPROC myWglSwapIntervalExtProc;

//==========================================================================
//
// Windows framebuffer
//
//==========================================================================


//==========================================================================
//
// 
//
//==========================================================================

SystemGLFrameBuffer::SystemGLFrameBuffer(void *hMonitor, bool fullscreen) : SystemBaseFrameBuffer(hMonitor, fullscreen)
{
	if (!static_cast<Win32GLVideo*>(Video)->InitHardware(mainwindow.GetHandle(), 0))
	{
		I_FatalError("Unable to initialize OpenGL");
		return;
	}

	HDC hDC = GetDC(mainwindow.GetHandle());
	const char *wglext = nullptr;

	myWglSwapIntervalExtProc = (PFNWGLSWAPINTERVALEXTPROC)zd_wglGetProcAddress("wglSwapIntervalEXT");
	auto myWglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)zd_wglGetProcAddress("wglGetExtensionsStringARB");
	if (myWglGetExtensionsStringARB)
	{
		wglext = myWglGetExtensionsStringARB(hDC);
	}
	else
	{
		auto myWglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)zd_wglGetProcAddress("wglGetExtensionsStringEXT");
		if (myWglGetExtensionsStringEXT)
		{
			wglext = myWglGetExtensionsStringEXT();
		}
	}
	SwapInterval = 1; 
	if (wglext != nullptr)
	{
		if (strstr(wglext, "WGL_EXT_swap_control_tear"))
		{
			SwapInterval = -1;
		}
	}
	ReleaseDC(mainwindow.GetHandle(), hDC);
}

//==========================================================================
//
// 
//
//==========================================================================
EXTERN_CVAR(Bool, vid_vsync);
CUSTOM_CVAR(Bool, gl_control_tear, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
{
	vid_vsync->Callback();
}

void SystemGLFrameBuffer::SetVSync (bool vsync)
{
	if (myWglSwapIntervalExtProc != NULL) myWglSwapIntervalExtProc(vsync ? (gl_control_tear? SwapInterval : 1) : 0);
}

void SystemGLFrameBuffer::SwapBuffers()
{
	::SwapBuffers(static_cast<Win32GLVideo *>(Video)->m_hDC);
}

