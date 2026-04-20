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
