/*
** c_commandbuffer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2010-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
*/

#pragma once

#include <string>

#include "zstring.h"

struct FCommandBuffer
{
private:
	std::u32string Text;
	unsigned CursorPos = 0;
	unsigned StartPos = 0;	// First character to display
	unsigned CursorPosCells = 0;
	unsigned StartPosCells = 0;

	std::u32string YankBuffer;	// Deleted text buffer

public:
	bool AppendToYankBuffer = false;	// Append consecutive deletes to buffer
	int ConCols;

	FCommandBuffer() = default;

	FCommandBuffer(const FCommandBuffer &o);
	FString GetText() const;

	size_t TextLength() const
	{
		return Text.length();
	}

	void Draw(int x, int y, int scale, bool cursor);
	unsigned CalcCellSize(unsigned length);
	unsigned CharsForCells(unsigned cellin, bool *overflow);
	void MakeStartPosGood();
	bool CursorStart();
	bool CursorEnd();

private:
	void MoveCursorLeft()
	{
		CursorPos--;
	}

	void MoveCursorRight()
	{
		CursorPos++;
	}

public:
	void CursorLeft();
	void CursorRight();
	void CursorWordLeft();
	void CursorWordRight();
	void DeleteLeft();
	void DeleteRight();
	void DeleteWordLeft();
	void DeleteLineLeft();
	void DeleteLineRight();
	void AddChar(int character);
	void AddString(FString clip);
	void SetString(const FString &str);
	void AddYankBuffer();
};

extern FCommandBuffer CmdLine;
