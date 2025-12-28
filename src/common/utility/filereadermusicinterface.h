/*
** filereadermusicinterface.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

#include <zmusic.h>

#include "files.h"

inline ZMusicCustomReader *GetMusicReader(FileReader& fr)
{
	using FileSys::FileReaderInterface;
	auto zcr = new ZMusicCustomReader;

	zcr->handle = fr.GetInterface();
	zcr->gets = [](ZMusicCustomReader* zr, char* buff, int n) { return reinterpret_cast<FileReaderInterface*>(zr->handle)->Gets(buff, n); };
	zcr->read = [](ZMusicCustomReader* zr, void* buff, int32_t size) -> long { return (long)reinterpret_cast<FileReaderInterface*>(zr->handle)->Read(buff, size); };
	zcr->seek = [](ZMusicCustomReader* zr, long offset, int whence) -> long { return (long)reinterpret_cast<FileReaderInterface*>(zr->handle)->Seek(offset, whence); };
	zcr->tell = [](ZMusicCustomReader* zr) -> long { return (long)reinterpret_cast<FileReaderInterface*>(zr->handle)->Tell(); };
	zcr->close = [](ZMusicCustomReader* zr)
	{
		delete reinterpret_cast<FileReaderInterface*>(zr->handle);
		delete zr;
	};
	return zcr;
}

