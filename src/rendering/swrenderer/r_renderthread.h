/*
** r_renderthread.h
**
** Renderer multithreading framework
**
**---------------------------------------------------------------------------
**
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
** Copyright 2016 Magnus Norddahl
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
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
