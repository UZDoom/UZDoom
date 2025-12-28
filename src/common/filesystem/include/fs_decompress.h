/*
** fs_decompress.h
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
#include "fs_files.h"

namespace FileSys {

// Zip compression methods, extended by some internal types to be passed to OpenDecompressor
enum ECompressionMethod
{
	METHOD_STORED = 0,
	METHOD_SHRINK = 1,
	METHOD_IMPLODE = 6,
	METHOD_DEFLATE = 8,
	METHOD_BZIP2 = 12,
	METHOD_LZMA = 14,
	METHOD_XZ = 95,
	METHOD_PPMD = 98,
	METHOD_LZSS = 1337,			// not used in Zips - this is for Console Doom compression
	METHOD_ZLIB = 1338,			// Zlib stream with header, used by compressed nodes.
	METHOD_RFFCRYPT = 1339,		// not actual compression but can be put in here to make handling easier.
	METHOD_IMPLODE_MIN = 1000, // having discrete types for these avoids keeping around the GPFlags word in Zips.
	METHOD_IMPLODE_0 = 1000,
	METHOD_IMPLODE_2 = 1002,
	METHOD_IMPLODE_4 = 1004,
	METHOD_IMPLODE_6 = 1006,
	METHOD_IMPLODE_MAX = 1006,
	METHOD_INVALID = 0x7fff,
	METHOD_TRANSFEROWNER = 0x8000,
};

enum EDecompressFlags
{
	DCF_TRANSFEROWNER = 1,
	DCF_SEEKABLE = 2,
	DCF_EXCEPTIONS = 4,
	DCF_CACHED = 8,
};

bool OpenDecompressor(FileReader& self, FileReader &parent, FileReader::Size length, int method, int flags = 0);	// creates a decompressor stream. 'seekable' uses a buffered version so that the Seek and Tell methods can be used.

// This holds a compresed Zip entry with all needed info to decompress it.
struct FCompressedBuffer
{
	size_t mSize;
	size_t mCompressedSize;
	int mMethod;
	unsigned mCRC32;
	char* mBuffer;
	const char* filename;

	bool Decompress(char* destbuffer);
	void Clean()
	{
		mSize = mCompressedSize = 0;
		if (mBuffer != nullptr)
		{
			delete[] mBuffer;
			mBuffer = nullptr;
		}
	}
};


}
