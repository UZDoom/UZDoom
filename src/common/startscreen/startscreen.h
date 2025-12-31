/*
** startscreen.h
**
** Interface for the startup screen.
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
** The startup screen interface is based on a mix of Heretic and Hexen.
** Actual implementation is system-specific.
*/

#pragma once

#include <stdint.h>
#include <functional>
#include "bitmap.h"
#include "zstring.h"

class FGameTexture;

struct RgbQuad 
{
	uint8_t    rgbBlue;
	uint8_t    rgbGreen;
	uint8_t    rgbRed;
	uint8_t    rgbReserved;
};


extern const RgbQuad TextModePalette[16];

class FStartScreen
{
protected:
	int CurPos = 0;
	int MaxPos;
	int Scale = 1;
	int NetMaxPos = -1;
	int NetCurPos = 0;
	FBitmap StartupBitmap;
	FBitmap HeaderBitmap;
	FBitmap NetBitmap;
	FString NetMessageString;
	FGameTexture* StartupTexture = nullptr;
	FGameTexture* HeaderTexture = nullptr;
	FGameTexture* NetTexture = nullptr;
public:
	FStartScreen(int maxp) { MaxPos = maxp; }
	virtual ~FStartScreen();
	void Render(bool force = false);
	bool Progress(int);
	void NetProgress(int count);
	virtual void LoadingStatus(const char *message, int colors) {}
	virtual void AppendStatusLine(const char *status) {}
	virtual bool NetInit(const char* message, int numplayers);
	virtual void NetDone() {}
	virtual void NetTick() {}
	FBitmap& GetBitmap() { return StartupBitmap; }
	int GetScale() const { return Scale; }

	
protected:
	void ClearBlock(FBitmap& bitmap_info, RgbQuad fill, int x, int y, int bytewidth, int height);
	FBitmap AllocTextBitmap();
	void DrawTextScreen(FBitmap& bitmap_info, const uint8_t* text_screen);
	int DrawChar(FBitmap& screen, double x, double y, unsigned charnum, uint8_t attrib);
	int DrawChar(FBitmap& screen, double x, double y, unsigned charnum, RgbQuad fg, RgbQuad bg);
	int DrawString(FBitmap& screen, double x, double y, const char* text, RgbQuad fg, RgbQuad bg);
	void UpdateTextBlink(FBitmap& bitmap_info, const uint8_t* text_screen, bool on);
	void ST_Sound(const char* sndname);
	int SizeOfText(const char* text);
	void CreateHeader();
	void DrawNetStatus(int found, int total);
	void ValidateTexture();
	virtual bool DoProgress(int);
	virtual void DoNetProgress(int count);
};

FStartScreen* GetGameStartScreen(int max_progress);

[[noreturn]]
void ST_Endoom();
