/*
** r_renderer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Christoph Oelckers
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

#ifndef __R_RENDERER_H
#define __R_RENDERER_H

#include <stdio.h>

struct FRenderer;
extern FRenderer *SWRenderer;

class FSerializer;
class AActor;
class player_t;
struct sector_t;
class FCanvasTexture;
class DCanvas;
struct FLevelLocals;
class PClassActor;

struct FRenderer
{
	virtual ~FRenderer() {}

	// precache one texture
	virtual void Precache(uint8_t *texhitlist, TMap<PClassActor*, bool> &actorhitlist) = 0;

	// render 3D view
	virtual void RenderView(player_t *player, DCanvas *target, void *videobuffer, int bufferpitch) = 0;

	// renders view to a savegame picture
	virtual void WriteSavePic(player_t *player, FileWriter *file, int width, int height) = 0;

	// draws player sprites with hardware acceleration (only useful for software rendering)
	virtual void DrawRemainingPlayerSprites() = 0;

	// set up the colormap for a newly loaded level.
	virtual void SetColormap(FLevelLocals *) = 0;

	virtual void SetClearColor(int color) = 0;

	virtual void Init() = 0;

};


#endif
