/*
** memarena.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2016 Marisa Heit
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

#ifndef __MEMARENA_H
#define __MEMARENA_H

#include "zstring.h"

// A general purpose arena.
class FMemArena
{
public:
	FMemArena(size_t blocksize = 10*1024);
	~FMemArena();

	void *Alloc(size_t size);
	void* Calloc(size_t size);
	const char* Strdup(const char*);
	void FreeAll();
	void FreeAllBlocks();
	FString DumpInfo();
	void DumpData(FILE *f);

protected:
	struct Block;

	Block *AddBlock(size_t size);
	void FreeBlockChain(Block *&top);
	void *iAlloc(size_t size);

	Block *TopBlock;
	Block *FreeBlocks;
	size_t BlockSize;
};

// An arena specializing in storage of FStrings. It knows how to free them,
// but this means it also should never be used for allocating anything else.
// Identical strings all return the same pointer.
class FSharedStringArena : public FMemArena
{
public:
	FSharedStringArena();
	~FSharedStringArena();
	void FreeAll();

	class FString *Alloc(const FString &source);
	class FString *Alloc(const char *source);
	class FString *Alloc(const char *source, size_t strlen);

protected:
	struct Node
	{
		Node *Next;
		FString String;
		unsigned int Hash;
	};
	Node *Buckets[256];

	Node *FindString(const char *str, size_t strlen, unsigned int &hash);
private:
	void *Alloc(size_t size) { return NULL; }	// No access to FMemArena::Alloc for outsiders.
};


#endif
