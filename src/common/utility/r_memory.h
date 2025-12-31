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
