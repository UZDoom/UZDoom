/*
** r_draw.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
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
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "r_defs.h"
#include "c_cvars.h"
#include <memory>

struct FSWColormap;
struct FLightNode;

EXTERN_CVAR(Int, r_multithreaded);
EXTERN_CVAR(Bool, r_magfilter);
EXTERN_CVAR(Bool, r_minfilter);
EXTERN_CVAR(Bool, r_mipmap);
EXTERN_CVAR(Float, r_lod_bias);
EXTERN_CVAR(Int, r_drawfuzz);
EXTERN_CVAR(Bool, r_drawtrans);
EXTERN_CVAR(Float, transsouls);
EXTERN_CVAR(Bool, r_dynlights);
EXTERN_CVAR(Bool, r_fuzzscale);

namespace swrenderer
{
	class DrawerArgs;
	class SkyDrawerArgs;
	class WallDrawerArgs;
	class WallColumnDrawerArgs;
	class SpanDrawerArgs;
	class SpriteDrawerArgs;
	class VoxelBlock;

	extern uint8_t shadetables[/*NUMCOLORMAPS*16*256*/];
	extern FDynamicColormap ShadeFakeColormap[16];
	extern uint8_t identitymap[256];
	extern FDynamicColormap identitycolormap;

	// Constant arrays used for psprite clipping and initializing clipping.
	extern short zeroarray[MAXWIDTH];
	extern short screenheightarray[MAXWIDTH];

	// Spectre/Invisibility.
	#define FUZZTABLE 50
	#define FUZZ_RANDOM_X_SIZE 100
	extern int fuzzoffset[FUZZTABLE + 1];
	extern int fuzz_random_x_offset[FUZZ_RANDOM_X_SIZE];
	extern int fuzzpos;
	extern int fuzzviewheight;

	#define NUM_PARTICLE_TEXTURES 3
	#define PARTICLE_TEXTURE_SIZE 64
	extern uint32_t particle_texture[NUM_PARTICLE_TEXTURES][PARTICLE_TEXTURE_SIZE * PARTICLE_TEXTURE_SIZE];

	class SWPixelFormatDrawers
	{
	public:
		SWPixelFormatDrawers(RenderThread* thread) : thread(thread) { }
		virtual ~SWPixelFormatDrawers() = default;
		virtual void DrawWall(const WallDrawerArgs &args) = 0;
		virtual void DrawWallMasked(const WallDrawerArgs &args) = 0;
		virtual void DrawWallAdd(const WallDrawerArgs &args) = 0;
		virtual void DrawWallAddClamp(const WallDrawerArgs &args) = 0;
		virtual void DrawWallSubClamp(const WallDrawerArgs &args) = 0;
		virtual void DrawWallRevSubClamp(const WallDrawerArgs &args) = 0;
		virtual void DrawSingleSkyColumn(const SkyDrawerArgs &args) = 0;
		virtual void DrawDoubleSkyColumn(const SkyDrawerArgs &args) = 0;
		virtual void DrawColumn(const SpriteDrawerArgs &args) = 0;
		virtual void FillColumn(const SpriteDrawerArgs &args) = 0;
		virtual void FillAddColumn(const SpriteDrawerArgs &args) = 0;
		virtual void FillAddClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void FillSubClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void FillRevSubClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawFuzzColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawAddColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawTranslatedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawTranslatedAddColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawShadedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawAddClampShadedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawAddClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawAddClampTranslatedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawSubClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawSubClampTranslatedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawRevSubClampColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawRevSubClampTranslatedColumn(const SpriteDrawerArgs &args) = 0;
		virtual void DrawVoxelBlocks(const SpriteDrawerArgs &args, const VoxelBlock *blocks, int blockcount) = 0;
		virtual void DrawSpan(const SpanDrawerArgs &args) = 0;
		virtual void DrawSpanMasked(const SpanDrawerArgs &args) = 0;
		virtual void DrawSpanTranslucent(const SpanDrawerArgs &args) = 0;
		virtual void DrawSpanMaskedTranslucent(const SpanDrawerArgs &args) = 0;
		virtual void DrawSpanAddClamp(const SpanDrawerArgs &args) = 0;
		virtual void DrawSpanMaskedAddClamp(const SpanDrawerArgs &args) = 0;
		virtual void FillSpan(const SpanDrawerArgs &args) = 0;
		virtual void DrawTiltedSpan(const SpanDrawerArgs &args, const FVector3 &plane_sz, const FVector3 &plane_su, const FVector3 &plane_sv, bool plane_shade, int planeshade, float planelightfloat, fixed_t pviewx, fixed_t pviewy, FDynamicColormap *basecolormap) = 0;
		virtual void DrawColoredSpan(const SpanDrawerArgs &args) = 0;
		virtual void DrawFogBoundaryLine(const SpanDrawerArgs &args) = 0;
		virtual void DrawParticleColumn(int x, int yl, int ycount, uint32_t fg, uint32_t alpha, uint32_t fracposx) = 0;

		void SetLights(WallColumnDrawerArgs& drawerargs, int x, int y1, const WallDrawerArgs& wallargs);

		RenderThread* thread = nullptr;
	};

	void R_InitShadeMaps();
	void R_InitFuzzTable(int fuzzoff);
	void R_InitParticleTexture();

	void R_UpdateFuzzPosFrameStart();
	void R_UpdateFuzzPos(const SpriteDrawerArgs &args);
}
