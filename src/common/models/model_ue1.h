/*
** model_ue1.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2022 Marisa Kirisame, UnSX Team
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <stdint.h>
#include "model.h"
#include "vectors.h"

class FUE1Model : public FModel
{
public:
	enum EPolyType
	{
		PT_Normal = 0,		// normal renderstyle
		PT_TwoSided = 1,	// like normal, but don't cull backfaces
		PT_Translucent = 2,	// additive blending
		PT_Masked = 3,		// draw with alpha testing
		PT_Modulated = 4,	// overlay-like blending (rgb values below 128 darken, 128 is unchanged, and above 128 lighten)
		// types mask
		PT_Type = 7,
		// flags
		PT_WeaponTriangle = 0x08,	// this poly is used for positioning a weapon attachment and should not be drawn
		PT_Unlit = 0x10,		// this poly is fullbright
		PT_Curvy = 0x20,		// this poly uses the facet normal
		PT_EnvironmentMap = 0x40,	// vertex UVs are remapped to their view-space X and Z normals, fake cubemap look
		PT_NoSmooth = 0x80		// this poly forcibly uses nearest filtering
	};

	bool Load(const char * fn, int lumpnum, const char * buffer, int length) override;
	int FindFrame(const char* name, bool nodefault) override;
	void RenderFrame(FModelRenderer *renderer, FGameTexture * skin, int frame, int frame2, double inter, FTranslationID translation, const FTextureID* surfaceskinids, int boneStartPosition) override;
	void BuildVertexBuffer(FModelRenderer *renderer) override;
	void AddSkins(uint8_t *hitlist, const FTextureID* surfaceskinids) override;
	void LoadGeometry();
	void UnloadGeometry();
	FUE1Model()
	{
		mDataLump = -1;
		mAnivLump = -1;
		dhead = NULL;
		dpolys = NULL;
		ahead = NULL;
		averts = NULL;
		numVerts = 0;
		numFrames = 0;
		numPolys = 0;
		numGroups = 0;
	}
	~FUE1Model();

private:
	int mDataLump, mAnivLump;

	// raw data structures
	struct d3dhead
	{
		uint16_t numpolys, numverts;
		uint16_t bogusrot, bogusframe;
		uint32_t bogusnorm[3];
		uint32_t fixscale;
		uint32_t unused[3];
		uint8_t padding[12];
	};
	struct d3dpoly
	{
		uint16_t vertices[3];
		uint8_t type, color;
		uint8_t uv[3][2];
		uint8_t texnum, flags;
	};
	struct a3dhead
	{
		uint16_t numframes, framesize;
	};
	const d3dhead * dhead;
	const d3dpoly * dpolys;
	const a3dhead * ahead;
	const uint32_t * averts;
	struct dxvert
	{
		int16_t x, y, z, pad;
	};
	const dxvert * dxverts;

	// converted data structures
	struct UE1Vertex
	{
		FVector3 Pos, Normal;
		TArray<int> P;	// polys that reference this vertex, used in normal computation to save time
		int nP;	// count of those polys
	};
	struct UE1Poly
	{
		int V[3];
		FVector2 C[3];
		TArray<FVector3> Normals;
	};
	struct UE1Group
	{
		TArray<int> P;
		int numPolys, texNum, type;
	};

	int numVerts;
	int numFrames;
	int numPolys;
	int numGroups;
	bool hasWeaponTriangle;

	TArray<UE1Vertex> verts;
	TArray<UE1Poly> polys;
	TArray<UE1Group> groups;
	//TArray<TRS> weapondata;	// pseudo-bone generated from weapon triangle (not yet implemented)
};
