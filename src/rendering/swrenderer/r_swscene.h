/*
** r_swscene.h
**
** render the software scene through the hardware rendering backend
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2018 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

#include "r_defs.h"
#include "m_fixed.h"
#include "hwrenderer/scene/hw_clipper.h"
#include "r_utility.h"
#include "c_cvars.h"
#include <memory>

class FWrapperTexture;
class DCanvas;

class SWSceneDrawer
{
	FTexture *PaletteTexture;
	std::unique_ptr<FGameTexture> FBTexture[2];
	int FBTextureIndex = 0;
	bool FBIsTruecolor = false;
	std::unique_ptr<DCanvas> Canvas;

public:
	SWSceneDrawer();
	~SWSceneDrawer();

	sector_t *RenderView(player_t *player);
};

