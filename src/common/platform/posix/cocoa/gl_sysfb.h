/*
** gl_sysfb.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2012-2018 Alexey Lysiuk
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

#ifndef COCOA_GL_SYSFB_H_INCLUDED
#define COCOA_GL_SYSFB_H_INCLUDED

#include "v_video.h"

#ifdef __OBJC__
@class NSCursor;
@class CocoaWindow;
#else
typedef struct objc_object NSCursor;
typedef struct objc_object CocoaWindow;
#endif

class SystemBaseFrameBuffer : public DFrameBuffer
{
public:
	// This must have the same parameters as the Windows version, even if they are not used!
	SystemBaseFrameBuffer(void *hMonitor, bool fullscreen);
	~SystemBaseFrameBuffer();

	bool IsFullscreen() override;

	int GetClientWidth() override;
	int GetClientHeight() override;
	void ToggleFullscreen(bool yes) override;
	void SetWindowSize(int width, int height) override;

	virtual void SetMode(bool fullscreen, bool hiDPI);

	static void UseHiDPI(bool hiDPI);
	static void SetCursor(NSCursor* cursor);
	static void SetWindowVisible(bool visible);
	static void SetWindowTitle(const char* title);

	void SetWindow(CocoaWindow* window) { m_window = window; }

protected:
	SystemBaseFrameBuffer() {}

	void SetFullscreenMode();
	void SetWindowedMode();

	bool m_fullscreen;
	bool m_hiDPI;

	CocoaWindow* m_window;

	int GetTitleBarHeight() const;

};

class SystemGLFrameBuffer : public SystemBaseFrameBuffer
{
	typedef SystemBaseFrameBuffer Super;

public:
	SystemGLFrameBuffer(void *hMonitor, bool fullscreen);

	void SetVSync(bool vsync) override;

	void SetMode(bool fullscreen, bool hiDPI) override;

protected:
	void SwapBuffers();

	SystemGLFrameBuffer() {}
};

#endif // COCOA_GL_SYSFB_H_INCLUDED
