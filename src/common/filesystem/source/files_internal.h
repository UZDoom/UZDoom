/*
** files_internal.h
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
*/

#pragma once

#include <memory>
#include "fs_files.h"

namespace FileSys {

class MemoryReader : public FileReaderInterface
{
protected:
	const char * bufptr = nullptr;
	ptrdiff_t FilePos = 0;

	MemoryReader()
	{}

public:
	MemoryReader(const char *buffer, ptrdiff_t length)
	{
		bufptr = buffer;
		Length = length;
		FilePos = 0;
	}

	ptrdiff_t Tell() const override;
	ptrdiff_t Seek(ptrdiff_t offset, int origin) override;
	ptrdiff_t Read(void *buffer, ptrdiff_t len) override;
	char *Gets(char *strbuf, ptrdiff_t len) override;
	virtual const char *GetBuffer() const override { return bufptr; }
};

class BufferingReader : public MemoryReader
{
	FileData buf;
	std::unique_ptr<FileReaderInterface> baseReader;
	ptrdiff_t bufferpos = 0;

	int FillBuffer(ptrdiff_t newpos);
public:
	BufferingReader(FileReaderInterface* base)
		: baseReader(base)
	{
		Length = base->Length;
		buf.allocate(Length);
		bufptr = (const char*)buf.data();
	}

	ptrdiff_t Seek(ptrdiff_t offset, int origin) override;
	ptrdiff_t Read(void* buffer, ptrdiff_t len) override;
	char* Gets(char* strbuf, ptrdiff_t len) override;
};

//==========================================================================
//
// MemoryArrayReader
//
// reads data from an array of memory
//
//==========================================================================

class MemoryArrayReader : public MemoryReader
{
	FileData buf;

public:
	MemoryArrayReader()
	{
		FilePos = 0;
		Length = 0;
	}

	MemoryArrayReader(size_t len)
	{
		buf.allocate(len);
		UpdateBuffer();
	}

	MemoryArrayReader(FileData& buffer)
	{
		buf = std::move(buffer);
		UpdateBuffer();
	}

	void UpdateBuffer()
	{
		bufptr = (const char*)buf.data();
		FilePos = 0;
		Length = buf.size();
	}
};

}
