/*
** c_consolebuffer.h
**
** manages the text for the console
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
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

#include <limits.h>
#include <stdio.h>
#include "zstring.h"
#include "tarray.h"
#include "v_text.h"

enum EAddType
{
	NEWLINE,
	APPENDLINE,
	REPLACELINE
};

class FConsoleBuffer
{
	TArray<FString> mConsoleText;
	TArray<TArray<FBrokenLines>> m_BrokenConsoleText;	// This holds the structures returned by V_BreakLines and is used for memory management.
	TArray<unsigned int> mBrokenStart;		
	TArray<FBrokenLines> mBrokenLines;		// This holds the single lines, indexed by mBrokenStart and is used for printing.
	FILE * mLogFile;
	EAddType mAddType;
	int mTextLines;
	bool mBufferWasCleared;

	FFont *mLastFont;
	int mLastDisplayWidth;
	bool mLastLineNeedsUpdate;


public:
	FConsoleBuffer();
	void AddText(int printlevel, const char *string);
	void FormatText(FFont *formatfont, int displaywidth);
	void ResizeBuffer(unsigned newsize);
	void Clear()
	{
		mBufferWasCleared = true;
		mConsoleText.Clear();
	}
	int GetFormattedLineCount() { return mTextLines; }
	FBrokenLines *GetLines() { return &mBrokenLines[0]; }
};

