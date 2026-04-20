/*
** gl_sysfb.h
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

#include "base_sysfb.h"

class SystemGLFrameBuffer : public SystemBaseFrameBuffer
{
	typedef SystemBaseFrameBuffer Super;

public:
	SystemGLFrameBuffer() {}
	// Actually, hMonitor is a HMONITOR, but it's passed as a void * as there
    // look to be some cross-platform bits in the way.
	SystemGLFrameBuffer(void *hMonitor, bool fullscreen);

	void SetVSync (bool vsync);
	void SwapBuffers();

protected:
	int SwapInterval;
};
