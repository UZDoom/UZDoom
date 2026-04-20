/*
** r_memory.h
**
** Render memory allocation
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
**  Copyright 2016-2020 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

// Memory needed for the duration of a frame rendering
class RenderMemory
{
public:
	void Clear();

	template<typename T>
	T *AllocMemory(int size = 1)
	{
		return (T*)AllocBytes(sizeof(T) * size);
	}

	template<typename T, typename... Types>
	T *NewObject(Types &&... args)
	{
		void *ptr = AllocBytes(sizeof(T));
		return new (ptr)T(std::forward<Types>(args)...);
	}

private:
	void *AllocBytes(int size);

	enum { BlockSize = 1024 * 1024 };

	struct MemoryBlock
	{
		MemoryBlock();
		~MemoryBlock();

		MemoryBlock(const MemoryBlock &) = delete;
		MemoryBlock &operator=(const MemoryBlock &) = delete;

		uint8_t *Data;
		uint32_t Position;
	};
	std::vector<std::unique_ptr<MemoryBlock>> UsedBlocks;
	std::vector<std::unique_ptr<MemoryBlock>> FreeBlocks;
};
