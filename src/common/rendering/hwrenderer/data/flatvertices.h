/*
** flatvertices.h
**
** Creates flat vertex data for hardware rendering.
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2020 Christoph Oelckers
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

#ifndef _HW__VERTEXBUFFER_H
#define _HW__VERTEXBUFFER_H

#include "tarray.h"
#include "hwrenderer/data/buffers.h"
#include <atomic>
#include <mutex>

class FRenderState;
struct secplane_t;

struct FFlatVertex
{
	float x, z, y;	// world position
	float u, v;		// texture coordinates
	float lu, lv;	// lightmap texture coordinates
	float lindex;	// lightmap texture index

	void Set(float xx, float zz, float yy, float uu, float vv)
	{
		x = xx;
		z = zz;
		y = yy;
		u = uu;
		v = vv;
		lindex = -1.0f;
	}

	void Set(float xx, float zz, float yy, float uu, float vv, float llu, float llv, float llindex)
	{
		x = xx;
		z = zz;
		y = yy;
		u = uu;
		v = vv;
		lu = llu;
		lv = llv;
		lindex = llindex;
	}

	void SetVertex(float _x, float _y, float _z = 0)
	{
		x = _x;
		z = _y;
		y = _z;
	}

	void SetTexCoord(float _u = 0, float _v = 0)
	{
		u = _u;
		v = _v;
	}

};

class FFlatVertexBuffer
{
public:
	TArray<FFlatVertex> vbo_shadowdata;
	TArray<uint32_t> ibo_data;

	int mPipelineNbr;
	int mPipelinePos = 0;

	IVertexBuffer* mVertexBuffer;
	IVertexBuffer *mVertexBufferPipeline[HW_MAX_PIPELINE_BUFFERS];
	IIndexBuffer *mIndexBuffer;



	unsigned int mIndex;
	std::atomic<unsigned int> mCurIndex;
	unsigned int mNumReserved;

	unsigned int mMapStart;

	static const unsigned int BUFFER_SIZE = 2000000;
	static const unsigned int BUFFER_SIZE_TO_USE = BUFFER_SIZE-500;

public:
	enum
	{
		QUAD_INDEX = 0,
		FULLSCREEN_INDEX = 4,
		PRESENT_INDEX = 8,
		STENCILTOP_INDEX = 12,
		STENCILBOTTOM_INDEX = 16,

		NUM_RESERVED = 20
	};

	FFlatVertexBuffer(int width, int height, int pipelineNbr = 1);
	~FFlatVertexBuffer();

	void OutputResized(int width, int height);
	std::pair<IVertexBuffer *, IIndexBuffer *> GetBufferObjects() const 
	{
		return std::make_pair(mVertexBuffer, mIndexBuffer);
	}

	void Copy(int start, int count);

	FFlatVertex *GetBuffer(int index) const
	{
		FFlatVertex *ff = (FFlatVertex*)mVertexBuffer->Memory();
		return &ff[index];
	}

	FFlatVertex *GetBuffer() const
	{
		return GetBuffer(mCurIndex);
	}

	std::pair<FFlatVertex *, unsigned int> AllocVertices(unsigned int count);

	void Reset()
	{
		mCurIndex = mIndex;
	}

	void NextPipelineBuffer()
	{
		mPipelinePos++;
		mPipelinePos %= mPipelineNbr;

		mVertexBuffer = mVertexBufferPipeline[mPipelinePos];
	}

	void Map()
	{
		mMapStart = mCurIndex;
		mVertexBuffer->Map();
	}

	void Unmap()
	{
		mVertexBuffer->Unmap();
		mVertexBuffer->Upload(mMapStart * sizeof(FFlatVertex), (mCurIndex - mMapStart) * sizeof(FFlatVertex));
	}

	void DropSync()
	{
		mVertexBuffer->GPUDropSync();
	}

	void WaitSync()
	{
		mVertexBuffer->GPUWaitSync();
	}

	int GetPipelinePos() 
	{ 
		return mPipelinePos; 
	}
};

#endif
