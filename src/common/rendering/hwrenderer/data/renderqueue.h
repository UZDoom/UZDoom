/*
** renderqueue.h
**
**
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
*/

#pragma once

#include <stdint.h>
#include "tflags.h"

// A render queue is what contains all render commands.
// On Vulkan there can be several of them so this interface is needed to allow for the needed parallelism.
// On OpenGL the render state is global so all this will do is to translate the system independent calls into OpenGL API calls.

enum class ColormaskBits
{
  RED = 1,
  GREEN = 2,
  BLUE = 4,
  ALPHA = 8
};

typedef TFlags<ColormaskBits, uint8_t> Colormask;

class IRenderQueue
{
	Colormask mColorMask;


	Colormask GetColorMask() const
	{
		return mColorMask;
	}

	virtual void SetColorMask(Colormask mask) = 0;


};
