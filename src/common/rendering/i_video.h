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
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
