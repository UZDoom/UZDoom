/*
** r_3dfloors.h
**
** software 3D floors addon
**
**---------------------------------------------------------------------------
**
** Copyright 2010 kgsws
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
#include <unordered_map>

EXTERN_CVAR(Int, r_3dfloors);

namespace swrenderer
{
	class RenderThread;
	class FakeFloorClip;

	struct HeightLevel
	{
		double height;
		struct HeightLevel *prev;
		struct HeightLevel *next;
	};

	struct HeightStack
	{
		HeightLevel *height_top;
		HeightLevel *height_cur;
		int height_max;
	};

	struct ClipStack
	{
		short floorclip[MAXWIDTH];
		short ceilingclip[MAXWIDTH];
		FakeFloorClip *ffloor;
		ClipStack *next;
	};

	// BSP stage
	struct Fake3DOpaque
	{
		enum Type
		{
			Normal,       // Not a 3D floor
			FakeFloor,    // fake floor, mark seg as FAKE
			FakeCeiling,  // fake ceiling, mark seg as FAKE
			FakeBack      // RenderLine with fake backsector, mark seg as FAKE
		};
		Type type = Normal;

		bool clipBotFront = false; // use front sector clipping info (bottom)
		bool clipTopFront = false; // use front sector clipping info (top)

		Fake3DOpaque() { }
		Fake3DOpaque(Type type) : type(type) { }
	};

	// Drawing stage
	struct Fake3DTranslucent
	{
		bool clipBottom = false;
		bool clipTop = false;
		bool down2Up = false; // rendering from down to up (floors)
		double sclipBottom = 0;
		double sclipTop = 0;
	};

	class FakeFloorClip
	{
	public:
		FakeFloorClip(F3DFloor *fakeFloor) : fakeFloor(fakeFloor) { }

		F3DFloor *fakeFloor = nullptr;
		short *floorclip = nullptr;
		short *ceilingclip = nullptr;
		int	validcount = -1;
	};

	class Clip3DFloors
	{
	public:
		Clip3DFloors(RenderThread *thread);

		void Cleanup();

		void DeleteHeights();
		void AddHeight(secplane_t *add, sector_t *sec);
		void NewClip();
		void ResetClip();
		void EnterSkybox();
		void LeaveSkybox();
		void SetFakeFloor(F3DFloor *fakeFloor);
		void ClearFakeFloors() { FakeFloors.clear(); }

		RenderThread *Thread = nullptr;

		FakeFloorClip *fakeFloor = nullptr;
		bool fakeActive = false;
		HeightLevel *height_top = nullptr;
		HeightLevel *height_cur = nullptr;
		int CurrentSkybox = 0;

	private:
		int height_max = -1;
		TArray<HeightStack> toplist;
		ClipStack *clip_top = nullptr;
		ClipStack *clip_cur = nullptr;

		std::unordered_map<F3DFloor *, FakeFloorClip *> FakeFloors;
	};
}
