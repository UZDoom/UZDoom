/*
** fs_stringpool.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2016 Marisa Heit
** Copyright 2005-2023 Christoph Oelckers
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

namespace FileSys {
// Storage for all the static strings the file system must hold.
class StringPool
{
	// do not allow externally defining this.
	friend class FileSystem;
	friend class FResourceFile;
private:
	StringPool(bool _shared, size_t blocksize = 10*1024) : TopBlock(nullptr), BlockSize(blocksize), shared(_shared) {}
public:
	~StringPool();
	const char* Strdup(const char*);
	void* Alloc(size_t size);

protected:
	struct Block;

	Block *AddBlock(size_t size);

	Block *TopBlock;
	size_t BlockSize;
public:
	bool shared;
};

}
