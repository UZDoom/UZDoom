/*
** model_kvx.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2011-2016 Marisa Heit
** Copyright 2013-2016 Christoph Oelckers
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

#include "model.h"
#include "i_modelvertexbuffer.h"
#include "tarray.h"
#include "xs_Float.h"

struct FVoxel;
struct kvxslab_t;
class FModelRenderer;
class FGameTexture;

struct FVoxelVertexHash
{
	// Returns the hash value for a key.
	hash_t Hash(const FModelVertex &key) 
	{ 
		int ix = int(key.x);
		int iy = int(key.y);
		int iz = int(key.z);
		return (hash_t)(ix + (iy<<9) + (iz<<18));
	}

	// Compares two keys, returning zero if they are the same.
	int Compare(const FModelVertex &left, const FModelVertex &right) 
	{ 
		return left.x != right.x || left.y != right.y || left.z != right.z || left.u != right.u || left.v != right.v;
	}
};

struct FIndexInit
{
	void Init(unsigned int &value)
	{
		value = 0xffffffff;
	}
};

typedef TMap<FModelVertex, unsigned int, FVoxelVertexHash, FIndexInit> FVoxelMap;


class FVoxelModel : public FModel
{
protected:
	FVoxel *mVoxel;
	bool mOwningVoxel;	// if created through MODELDEF deleting this object must also delete the voxel object
	FTextureID mPalette;
	unsigned int mNumIndices;
	TArray<FModelVertex> mVertices;
	TArray<unsigned int> mIndices;

	void MakeSlabPolys(int x, int y, kvxslab_t *voxptr, FVoxelMap &check);
	void AddFace(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int x4, int y4, int z4, uint8_t color, FVoxelMap &check);
	unsigned int AddVertex(FModelVertex &vert, FVoxelMap &check);

public:
	FVoxelModel(FVoxel *voxel, bool owned);
	~FVoxelModel();
	bool Load(const char * fn, int lumpnum, const char * buffer, int length) override;
	void Initialize();
	virtual int FindFrame(const char* name, bool nodefault) override;
	virtual void RenderFrame(FModelRenderer *renderer, FGameTexture * skin, int frame, int frame2, double inter, FTranslationID translation, const FTextureID* surfaceskinids, int boneStartPosition) override;
	virtual void AddSkins(uint8_t *hitlist, const FTextureID* surfaceskinids) override;
	FTextureID GetPaletteTexture() const { return mPalette; }
	void BuildVertexBuffer(FModelRenderer *renderer) override;
	float getAspectFactor(float vscale) override;
};


