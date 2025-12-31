/*
** hw_lightbuffer.cpp
**
** Buffer data maintenance for dynamic lights
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "hw_lightbuffer.h"
#include "hw_dynlightdata.h"
#include "shaderuniforms.h"

static const int ELEMENTS_PER_LIGHT = 4;			// each light needs 4 vec4's.
static const int ELEMENT_SIZE = (4*sizeof(float));


FLightBuffer::FLightBuffer(int pipelineNbr):
	mPipelineNbr(pipelineNbr)
{
	int maxNumberOfLights = 80000;

	mBufferSize = maxNumberOfLights * ELEMENTS_PER_LIGHT;
	mByteSize = mBufferSize * ELEMENT_SIZE;

	if (screen->useSSBO())
	{
		mBufferType = true;
		mBlockAlign = 0;
		mBlockSize = mBufferSize;
		mMaxUploadSize = mBlockSize;
	}
	else
	{
		mBufferType = false;
		mBlockSize = screen->maxuniformblock / ELEMENT_SIZE;
		mBlockAlign = screen->uniformblockalignment / ELEMENT_SIZE;
		mMaxUploadSize = (mBlockSize - mBlockAlign);

		//mByteSize += screen->maxuniformblock;	// to avoid mapping beyond the end of the buffer. REMOVED this...This can try to allocate 100's of MB..
	}

	for (int n = 0; n < mPipelineNbr; n++)
	{
		mBufferPipeline[n] = screen->CreateDataBuffer(LIGHTBUF_BINDINGPOINT, mBufferType, false);
		mBufferPipeline[n]->SetData(mByteSize, nullptr, BufferUsageType::Persistent);
	}

	Clear();
}

FLightBuffer::~FLightBuffer()
{
	delete mBuffer;
}

void FLightBuffer::Clear()
{
	mIndex = 0;

	mPipelinePos++;
	mPipelinePos %= mPipelineNbr;

	mBuffer = mBufferPipeline[mPipelinePos];
}

int FLightBuffer::UploadLights(FDynLightData &data)
{
	// All meaasurements here are in vec4's.
	int size0 = data.arrays[0].Size()/4;
	int size1 = data.arrays[1].Size()/4;
	int size2 = data.arrays[2].Size()/4;
	int totalsize = size0 + size1 + size2 + 1;

	if (totalsize > (int)mMaxUploadSize)
	{
		int diff = totalsize - (int)mMaxUploadSize;

		size2 -= diff;
		if (size2 < 0)
		{
			size1 += size2;
			size2 = 0;
		}
		if (size1 < 0)
		{
			size0 += size1;
			size1 = 0;
		}
		totalsize = size0 + size1 + size2 + 1;
	}

	float *mBufferPointer = (float*)mBuffer->Memory();
	assert(mBufferPointer != nullptr);
	if (mBufferPointer == nullptr) return -1;
	if (totalsize <= 1) return -1;	// there are no lights

	unsigned thisindex = mIndex.fetch_add(totalsize);
	float parmcnt[] = { 0, float(size0), float(size0 + size1), float(size0 + size1 + size2) };

	if (thisindex + totalsize <= mBufferSize)
	{
		float *copyptr = mBufferPointer + thisindex*4;

		memcpy(&copyptr[0], parmcnt, ELEMENT_SIZE);
		memcpy(&copyptr[4], &data.arrays[0][0], size0 * ELEMENT_SIZE);
		memcpy(&copyptr[4 + 4*size0], &data.arrays[1][0], size1 * ELEMENT_SIZE);
		memcpy(&copyptr[4 + 4*(size0 + size1)], &data.arrays[2][0], size2 * ELEMENT_SIZE);
		return thisindex;
	}
	else
	{
		return -1;	// Buffer is full. Since it is being used live at the point of the upload we cannot do much here but to abort.
	}
}

int FLightBuffer::GetBinding(unsigned int index, size_t* pOffset, size_t* pSize)
{
	// this function will only get called if a uniform buffer is used. For a shader storage buffer we only need to bind the buffer once at the start.
	unsigned int offset = (index / mBlockAlign) * mBlockAlign;

	*pOffset = offset * ELEMENT_SIZE;
	*pSize = mBlockSize * ELEMENT_SIZE;
	return (index - offset);
}



