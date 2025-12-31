/*
** c_notifybufferbase.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2020 Christoph Oelckers
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
#include "zstring.h"
#include "tarray.h"

class FFont;

struct FNotifyText
{
	int TimeOut;
	int Ticker;
	int PrintLevel;
	FString Text;
};

class FNotifyBufferBase
{
public:
	virtual ~FNotifyBufferBase() = default;
	virtual void AddString(int printlevel, FString source) = 0;
	virtual void Shift(int maxlines);
	virtual void Clear();
	virtual void Tick();
	virtual void Draw() = 0;

protected:
	TArray<FNotifyText> Text;
	int Top = 0;
	int TopGoal = 0;
	int LineHeight = 0;
	enum { NEWLINE, APPENDLINE, REPLACELINE } AddType = NEWLINE;

	void AddString(int printlevel, FFont *printFont, const FString &source, int formatwidth, float keeptime, int maxlines);

};





