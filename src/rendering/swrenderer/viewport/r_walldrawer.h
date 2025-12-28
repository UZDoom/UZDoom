/*
** r_walldrawer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
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
*/

#pragma once

#include "r_drawerargs.h"
#include "swrenderer/line/r_wallsetup.h"

struct FSWColormap;
struct FLightNode;

namespace swrenderer
{
	class RenderThread;
	class RenderViewport;
	
	class WallDrawerArgs : public DrawerArgs
	{
	public:
		void SetStyle(bool masked, bool additive, fixed_t alpha, bool dynlights);
		void SetDest(RenderViewport *viewport);
		void DrawWall(RenderThread *thread);

		uint32_t* SrcBlend() const { return dc_srcblend; }
		uint32_t* DestBlend() const { return dc_destblend; }
		fixed_t SrcAlpha() const { return dc_srcalpha; }
		fixed_t DestAlpha() const { return dc_destalpha; }

		RenderViewport* Viewport() const { return dc_viewport; }

		int x1, x2;
		short* uwal;
		short* dwal;
		FWallCoords WallC;
		ProjectedWallTexcoords texcoords;
		TMap<FDynamicLight*, std::unique_ptr<FLightNode>>* lightlist = nullptr;

		float lightpos;
		float lightstep;

		int texwidth;
		int texheight;
		int fracbits;
		bool mipmapped;
		const void* texpixels;

		// Viewport data
		uint16_t PortalMirrorFlags;
		bool fixedlight;
		float CenterX;
		float CenterY;
		float FocalTangent;
		float InvZtoScale;
		FVector3 ViewpointPos;
		float Sin, Cos, TanCos, TanSin;

	private:
		typedef void(SWPixelFormatDrawers::*WallDrawerFunc)(const WallDrawerArgs &args);
		WallDrawerFunc wallfunc = nullptr;

		uint32_t* dc_srcblend = nullptr;
		uint32_t* dc_destblend = nullptr;
		fixed_t dc_srcalpha = 0;
		fixed_t dc_destalpha = 0;

		RenderViewport* dc_viewport = nullptr;
	};
}
