/*
** r_walldraw.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "swrenderer/viewport/r_walldrawer.h"
#include "r_line.h"

struct FLightNode;
struct seg_t;
struct FLightNode;
struct FDynamicColormap;

namespace swrenderer
{
	class RenderThread;
	struct DrawSegment;
	struct FWallCoords;
	class ProjectedWallLine;
	class ProjectedWallTexcoords;
	struct WallSampler;

	class RenderWallPart
	{
	public:
		RenderWallPart(RenderThread *thread);

		void Render(
			const sector_t *lightsector,
			seg_t *curline,
			int tier,
			const FWallCoords &WallC,
			FSoftwareTexture *pic,
			int x1,
			int x2,
			const short *walltop,
			const short *wallbottom,
			const ProjectedWallTexcoords &texcoords,
			bool mask,
			bool additive,
			fixed_t alpha);

	private:
		void ProcessStripedWall(const short *uwal, const short *dwal, const ProjectedWallTexcoords& texcoords);
		void ProcessNormalWall(const short *uwal, const short *dwal, const ProjectedWallTexcoords& texcoords);
		TMap<FDynamicLight*, std::unique_ptr<FLightNode>>* GetLightList();

		RenderThread* Thread = nullptr;

		int x1 = 0;
		int x2 = 0;
		FSoftwareTexture *pic = nullptr;
		const sector_t *lightsector = nullptr;
		seg_t *curline = nullptr;
		int tier;
		FWallCoords WallC;

		ProjectedWallLight mLight;

		TMap<FDynamicLight*, std::unique_ptr<FLightNode>> *light_list = nullptr;
		bool mask = false;
		bool additive = false;
		fixed_t alpha = 0;
	};
}
