/*
** i_video.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __I_VIDEO_H__
#define __I_VIDEO_H__

#include <cstdint>

class DFrameBuffer;


class IVideo
{
public:
	virtual ~IVideo() {}

	virtual DFrameBuffer *CreateFrameBuffer() = 0;

	bool SetResolution();

	virtual void DumpAdapters();
};

void I_InitGraphics();
void I_ShutdownGraphics();

extern IVideo *Video;

void I_PolyPresentInit();
uint8_t *I_PolyPresentLock(int w, int h, bool vsync, int &pitch);
void I_PolyPresentUnlock(int x, int y, int w, int h);
void I_PolyPresentDeinit();


#endif // __I_VIDEO_H__
