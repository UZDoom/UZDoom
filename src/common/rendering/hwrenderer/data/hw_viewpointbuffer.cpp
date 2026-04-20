/*
** hw_viewpointbuffer.cpp
**
** Buffer data maintenance for per viewpoint uniform data
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "hwrenderer/data/shaderuniforms.h"
#include "hw_viewpointuniforms.h"
#include "hw_renderstate.h"
#include "hw_viewpointbuffer.h"
#include "hw_cvars.h"

static const int INITIAL_BUFFER_SIZE = 100;	// 100 viewpoints per frame should nearly always be enough

HWViewpointBuffer::HWViewpointBuffer(int pipelineNbr):
	mPipelineNbr(pipelineNbr)
{
	mBufferSize = INITIAL_BUFFER_SIZE;
	mBlockAlign = ((sizeof(HWViewpointUniforms) / screen->uniformblockalignment) + 1) * screen->uniformblockalignment;
	mByteSize = mBufferSize * mBlockAlign;

	for (int n = 0; n < mPipelineNbr; n++)
	{
		mBufferPipeline[n] = screen->CreateDataBuffer(VIEWPOINT_BINDINGPOINT, false, true);
		mBufferPipeline[n]->SetData(mByteSize, nullptr, BufferUsageType::Persistent);
	}

	mUploadIndex = 0;
	Clear();

	mLastMappedIndex = UINT_MAX;
}

HWViewpointBuffer::~HWViewpointBuffer()
{
	delete mBuffer;
}


void HWViewpointBuffer::CheckSize()
{
	if (mUploadIndex >= mBufferSize)
	{
		mBufferSize *= 2;
		mByteSize *= 2;
		for (int n = 0; n < mPipelineNbr; n++)
		{
			mBufferPipeline[n]->Resize(mByteSize);
		}
	}
}

int HWViewpointBuffer::Bind(FRenderState &di, unsigned int index)
{
	if (index != mLastMappedIndex)
	{
		mLastMappedIndex = index;
		mBuffer->BindRange(&di, index * mBlockAlign, mBlockAlign);
		di.EnableClipDistance(0, mClipPlaneInfo[index]);
	}
	return index;
}

void HWViewpointBuffer::Set2D(FRenderState &di, int width, int height, int pll)
{
	HWViewpointUniforms matrices;

	matrices.mViewMatrix.loadIdentity();
	matrices.mNormalViewMatrix.loadIdentity();
	matrices.mViewHeight = 0;
	matrices.mGlobVis = 1.f;
	matrices.mPalLightLevels = pll;
	matrices.mClipLine.X = -10000000.0f;
	matrices.mShadowmapFilter = gl_shadowmap_filter;
	matrices.mLightBlendMode = 0;

	matrices.mProjectionMatrix.ortho(0, (float)width, (float)height, 0, -1.0f, 1.0f);
	matrices.CalcDependencies();

	CheckSize();
	mBuffer->Map();
	memcpy(((char*)mBuffer->Memory()) + mUploadIndex * mBlockAlign, &matrices, sizeof(matrices));
	mBuffer->Unmap();

	mClipPlaneInfo.Push(0);

	Bind(di, mUploadIndex++);
}

int HWViewpointBuffer::SetViewpoint(FRenderState &di, HWViewpointUniforms *vp)
{
	CheckSize();
	mBuffer->Map();
	memcpy(((char*)mBuffer->Memory()) + mUploadIndex * mBlockAlign, vp, sizeof(*vp));
	mBuffer->Unmap();

	mClipPlaneInfo.Push(vp->mClipHeightDirection != 0.f || vp->mClipLine.X > -10000000.0f);
	return Bind(di, mUploadIndex++);
}

void HWViewpointBuffer::Clear()
{
	bool needNewPipeline = mUploadIndex > 0; // Clear might be called multiple times before any actual rendering

	mUploadIndex = 0;
	mClipPlaneInfo.Clear();

	if (needNewPipeline)
	{
		mLastMappedIndex = UINT_MAX;

		mPipelinePos++;
		mPipelinePos %= mPipelineNbr;
	}

	mBuffer = mBufferPipeline[mPipelinePos];
}

