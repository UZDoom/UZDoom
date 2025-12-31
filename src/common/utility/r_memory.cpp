/*
** r_memory.cpp
**
** Render memory allocation
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
**  Copyright 2016-2020 Magnus Norddahl
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**---------------------------------------------------------------------------
**
*/

#include <stdlib.h>

#include "r_memory.h"
#include <stdlib.h>

void *RenderMemory::AllocBytes(int size)
{
	size = (size + 15) / 16 * 16; // 16-byte align

	if (UsedBlocks.empty() || UsedBlocks.back()->Position + size > BlockSize)
	{
		if (!FreeBlocks.empty())
		{
			auto block = std::move(FreeBlocks.back());
			block->Position = 0;
			FreeBlocks.pop_back();
			UsedBlocks.push_back(std::move(block));
		}
		else
		{
			UsedBlocks.push_back(std::unique_ptr<MemoryBlock>(new MemoryBlock()));
		}
	}

	auto &block = UsedBlocks.back();
	void *data = block->Data + block->Position;
	block->Position += size;

	return data;
}

void RenderMemory::Clear()
{
	while (!UsedBlocks.empty())
	{
		auto block = std::move(UsedBlocks.back());
		UsedBlocks.pop_back();
		FreeBlocks.push_back(std::move(block));
	}
}

static void* Aligned_Alloc(size_t alignment, size_t size)
{
	void* ptr;
#if defined (_MSC_VER) || defined (__MINGW32__)
	ptr = _aligned_malloc(size, alignment);
	if (!ptr)
		throw std::bad_alloc();
#else
	// posix_memalign required alignment to be a min of sizeof(void *)
	if (alignment < sizeof(void*))
		alignment = sizeof(void*);

	if (posix_memalign((void**)&ptr, alignment, size))
		throw std::bad_alloc();
#endif
	return ptr;
}

static void Aligned_Free(void* ptr)
{
	if (ptr)
	{
#if defined _MSC_VER
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}
}

RenderMemory::MemoryBlock::MemoryBlock() : Data(static_cast<uint8_t*>(Aligned_Alloc(16, BlockSize))), Position(0)
{
}

RenderMemory::MemoryBlock::~MemoryBlock()
{
	Aligned_Free(Data);
}
