/*
** gl_sysfb.h
**
**
**
**---------------------------------------------------------------------------
**
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

#ifndef __POSIX_SDL_GL_SYSFB_H__
#define __POSIX_SDL_GL_SYSFB_H__

#include <SDL2/SDL.h>

#include "v_video.h"

class SystemBaseFrameBuffer : public DFrameBuffer
{
	typedef DFrameBuffer Super;

public:
	// this must have the same parameters as the Windows version, even if they are not used!
	SystemBaseFrameBuffer (void *hMonitor, bool fullscreen);

	bool IsFullscreen() override;

	int GetClientWidth() override;
	int GetClientHeight() override;

	void ToggleFullscreen(bool yes) override;
	void SetWindowSize(int client_w, int client_h) override;

protected:
	SystemBaseFrameBuffer () {}
};

class SystemGLFrameBuffer : public SystemBaseFrameBuffer
{
	typedef SystemBaseFrameBuffer Super;

public:
	SystemGLFrameBuffer(void *hMonitor, bool fullscreen);
	~SystemGLFrameBuffer();

	int GetClientWidth() override;
	int GetClientHeight() override;

	virtual void SetVSync(bool vsync) override;
	void SwapBuffers();

protected:
	SDL_GLContext GLContext;

	SystemGLFrameBuffer() {}
};

#endif // __POSIX_SDL_GL_SYSFB_H__
