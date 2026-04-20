/*
** filereadermusicinterface.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

