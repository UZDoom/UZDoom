/*
** r_renderthread.h
**
** Renderer multithreading framework
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
** Copyright 2016 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <memory>
#include <thread>

class RenderMemory;
struct FDynamicLight;

EXTERN_CVAR(Bool, r_models);

namespace swrenderer
{
	class VisibleSpriteList;
	class RenderPortal;
	class RenderOpaquePass;
	class RenderTranslucentPass;
	class RenderPlayerSprites;
	class RenderScene;
	class RenderViewport;
	class Clip3DFloors;
	class VisiblePlaneList;
	class DrawSegmentList;
	class RenderClipSegment;
	class RenderViewport;
	class LightVisibility;
	class SWPixelFormatDrawers;
	class SWTruecolorDrawers;
	class SWPalDrawers;
	class WallColumnDrawerArgs;

	class RenderThread
	{
	public:
		RenderThread(RenderScene *scene, bool mainThread = true);
		~RenderThread();

		RenderScene *Scene;
		int X1 = 0;
		int X2 = MAXWIDTH;
		bool MainThread = false;

		std::unique_ptr<RenderMemory> FrameMemory;
		std::unique_ptr<RenderOpaquePass> OpaquePass;
		std::unique_ptr<RenderTranslucentPass> TranslucentPass;
		std::unique_ptr<VisibleSpriteList> SpriteList;
		std::unique_ptr<RenderPortal> Portal;
		std::unique_ptr<Clip3DFloors> Clip3D;
		std::unique_ptr<RenderPlayerSprites> PlayerSprites;
		std::unique_ptr<VisiblePlaneList> PlaneList;
		std::unique_ptr<DrawSegmentList> DrawSegments;
		std::unique_ptr<RenderClipSegment> ClipSegments;
		std::unique_ptr<RenderViewport> Viewport;
		std::unique_ptr<LightVisibility> Light;

		TArray<FDynamicLight*> AddedLightsArray;

		std::thread thread;

		// VisibleSprite working buffers
		short clipbot[MAXWIDTH];
		short cliptop[MAXWIDTH];

		SWPixelFormatDrawers *Drawers(RenderViewport *viewport);

		// Setup poly object in a threadsafe manner
		void PreparePolyObject(subsector_t *sub);

		// Retrieve skycap color in a threadsafe way
		std::pair<PalEntry, PalEntry> GetSkyCapColor(FSoftwareTexture* tex);
		
	private:
		std::unique_ptr<SWTruecolorDrawers> tc_drawers;
		std::unique_ptr<SWPalDrawers> pal_drawers;
	};
}
