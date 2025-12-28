/*
** model_md3.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "model.h"

#define MD3_MAGIC			0x33504449

class FMD3Model : public FModel
{
	struct MD3Tag
	{
		// Currently I have no use for this
	};

	struct MD3TexCoord
	{
		float s,t;
	};

	struct MD3Vertex
	{
		float x,y,z;
		float nx,ny,nz;
	};

	struct MD3Triangle
	{
		int VertIndex[3];
	};

	struct MD3Surface
	{
		unsigned numVertices;
		unsigned numTriangles;
		unsigned numSkins;

		TArray<FTextureID> Skins;
		TArray<MD3Triangle> Tris;
		TArray<MD3TexCoord> Texcoords;
		TArray<MD3Vertex> Vertices;

		unsigned int vindex = UINT_MAX;	// contains numframes arrays of vertices
		unsigned int iindex = UINT_MAX;

		void UnloadGeometry()
		{
			Tris.Reset();
			Vertices.Reset();
			Texcoords.Reset();
		}
	};

	struct MD3Frame
	{
		// The bounding box information is of no use in the Doom engine
		// That will still be done with the actor's size information.
		char Name[16];
		float origin[3];
	};

	int numTags;
	int mLumpNum;

	TArray<MD3Frame> Frames;
	TArray<MD3Surface> Surfaces;

public:
	FMD3Model() = default;

	virtual bool Load(const char * fn, int lumpnum, const char * buffer, int length) override;
	virtual int FindFrame(const char* name, bool nodefault) override;
	virtual void RenderFrame(FModelRenderer *renderer, FGameTexture * skin, int frame, int frame2, double inter, FTranslationID translation, const FTextureID* surfaceskinids, int boneStartPosition) override;
	void LoadGeometry();
	void BuildVertexBuffer(FModelRenderer *renderer);
	virtual void AddSkins(uint8_t *hitlist, const FTextureID* surfaceskinids) override;
};

