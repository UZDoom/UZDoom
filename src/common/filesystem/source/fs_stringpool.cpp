/*
** fs_stringpool.cpp
**
** allocate static strings from larger blocks
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fs_stringpool.h"

namespace FileSys {
	
struct StringPool::Block
{
	Block *NextBlock;
	void *Limit;			// End of this block
	void *Avail;			// Start of free space in this block
	void *alignme;			// align to 16 bytes.

	void *Alloc(size_t size);
};

//==========================================================================
//
// StringPool Destructor
//
//==========================================================================

StringPool::~StringPool()
{
	for (Block *next, *block = TopBlock; block != nullptr; block = next)
	{
		next = block->NextBlock;
		free(block);
	}
	TopBlock = nullptr;
}

//==========================================================================
//
// StringPool :: Alloc
//
//==========================================================================

void *StringPool::Block::Alloc(size_t size)
{
	if ((char *)Avail + size > Limit)
	{
		return nullptr;
	}
	void *res = Avail;
	Avail = ((char *)Avail + size);
	return res;
}

StringPool::Block *StringPool::AddBlock(size_t size)
{
	size += sizeof(Block);		// Account for header size

	// Allocate a new block
	if (size < BlockSize)
	{
		size = BlockSize;
	}
	auto mem = (Block *)malloc(size);
	if (mem == nullptr)
	{
		
	}
	mem->Limit = (uint8_t *)mem + size;
	mem->Avail = &mem[1];
	mem->NextBlock = TopBlock;
	TopBlock = mem;
	return mem;
}

void *StringPool::Alloc(size_t size)
{
	Block *block;

	size = (size + 7) & ~7;
	for (block = TopBlock; block != nullptr; block = block->NextBlock)
	{
		void *res = block->Alloc(size);
		if (res != nullptr)
		{
			return res;
		}
	}
	block = AddBlock(size);
	return block->Alloc(size);
}

const char* StringPool::Strdup(const char* str)
{
	char* p = (char*)Alloc(strlen(str) + 1);
	strcpy(p, str);
	return p;
}

}
