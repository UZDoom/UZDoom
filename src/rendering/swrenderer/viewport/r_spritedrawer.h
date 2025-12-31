/*
** r_spritedrawer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
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

#include "r_drawerargs.h"

struct FSWColormap;
struct FLightNode;

namespace swrenderer
{
	class RenderThread;
	struct FWallCoords;
	class ProjectedWallLight;
	
	class VoxelBlock
	{
	public:
		int x, y;
		uint16_t width, height;
		fixed_t vPos;
		fixed_t vStep;
		const uint8_t *voxels;
		int voxelsCount;
	};

	class SpriteDrawerArgs : public DrawerArgs
	{
	public:
		SpriteDrawerArgs();

		bool SetStyle(RenderViewport *viewport, FRenderStyle style, fixed_t alpha, FTranslationID translation, uint32_t color, const ColormapLight &light);
		bool SetStyle(RenderViewport *viewport, FRenderStyle style, float alpha, FTranslationID translation, uint32_t color, const ColormapLight &light);
		void SetSolidColor(int color) { dc_color = color; dc_color_bgra = GPalette.BaseColors[color]; }
		void SetDynamicLight(uint32_t color) { dynlightcolor = color; }

		void DrawMasked(RenderThread* thread, double topZ, double scale, bool flipX, bool flipY, const FWallCoords& WallC, int clipx1, int clipx2, const ProjectedWallLight& light, FSoftwareTexture* texture, const short* mfloorclip, const short* mceilingclip, FRenderStyle style);
		void DrawMasked2D(RenderThread *thread, double x0, double x1, double y0, double y1, FSoftwareTexture* texture, FRenderStyle style);
		void DrawVoxelBlocks(RenderThread *thread, const VoxelBlock *blocks, int blockcount);

		uint8_t *Dest() const { return dc_dest; }
		int DestY() const { return dc_dest_y; }
		int Count() const { return dc_count; }

		int FuzzX() const { return dc_x; }
		int FuzzY1() const { return dc_yl; }
		int FuzzY2() const { return dc_yh; }

		uint32_t TextureUPos() const { return dc_texturefracx; }
		fixed_t TextureVPos() const { return dc_texturefrac; }
		fixed_t TextureVStep() const { return dc_iscale; }

		int SolidColor() const { return dc_color; }
		uint32_t SolidColorBgra() const { return dc_color_bgra; }
		uint32_t SrcColorIndex() const { return dc_srccolor; }
		uint32_t SrcColorBgra() const { return dc_srccolor_bgra; }

		const uint8_t *TexturePixels() const { return dc_source; }
		const uint8_t *TexturePixels2() const { return dc_source2; }
		uint32_t TextureHeight() const { return dc_textureheight; }

		uint32_t *SrcBlend() const { return dc_srcblend; }
		uint32_t *DestBlend() const { return dc_destblend; }
		fixed_t SrcAlpha() const { return dc_srcalpha; }
		fixed_t DestAlpha() const { return dc_destalpha; }
		
		uint32_t DynamicLight() const { return dynlightcolor; }

		bool DrawerNeedsPalInput() const { return drawer_needs_pal_input; }
		RenderViewport *Viewport() const { return dc_viewport; }

	private:
		void DrawMaskedColumn(RenderThread* thread, int x, float y1, int cliptop, int clipbottom, uint32_t texelX, uint32_t texelStepX, uint32_t texelStepY, float scaleV, bool flipY, FSoftwareTexture* tex, int texwidth, int texheight, bool bgra, FRenderStyle style);

		void SetDest(RenderViewport* viewport, int x, int y);
		void SetCount(int count) { dc_count = count; }

		bool SetBlendFunc(int op, fixed_t fglevel, fixed_t bglevel, int flags);
		static fixed_t GetAlpha(int type, fixed_t alpha);

		uint8_t *dc_dest = nullptr;
		int dc_dest_y = 0;
		int dc_count = 0;

		fixed_t dc_iscale;
		fixed_t dc_texturefrac;
		uint32_t dc_texturefracx;

		uint32_t dc_textureheight = 0;
		const uint8_t *dc_source = nullptr;
		const uint8_t *dc_source2 = nullptr;
		bool drawer_needs_pal_input = false;

		uint32_t *dc_srcblend = nullptr;
		uint32_t *dc_destblend = nullptr;
		fixed_t dc_srcalpha = OPAQUE;
		fixed_t dc_destalpha = 0;

		int dc_x = 0;
		int dc_yl = 0;
		int dc_yh = 0;

		int dc_color = 0;
		uint32_t dc_color_bgra = 0;
		uint32_t dc_srccolor = 0;
		uint32_t dc_srccolor_bgra = 0;
		
		uint32_t dynlightcolor = 0;

		typedef void(SWPixelFormatDrawers::*SpriteDrawerFunc)(const SpriteDrawerArgs &args);
		SpriteDrawerFunc colfunc;

		RenderViewport *dc_viewport = nullptr;

		friend class SWTruecolorDrawers;
		friend class SWPalDrawers;
	};
}
