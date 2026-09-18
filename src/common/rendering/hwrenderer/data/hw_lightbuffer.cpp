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
		mBlockAlign = screen->uniformblockalignment < ELEMENT_SIZE ? 1 : screen->uniformblockalignment / ELEMENT_SIZE;
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

int FLightBuffer::UploadLights(const FDynLightData &data)
{
	size_t sz = data.Vec4Size();
	if(sz <= 1) return -1;	// there are no lights
	unsigned int thisindex = std::min(mIndex.fetch_add(sz), mBufferSize + 1);
	TArray<float> out;
	if(thisindex >= mBufferSize || !data.Combine(out, std::min(mMaxUploadSize, mBufferSize - thisindex)))
	{
		mIndex.store(thisindex); // don't let it overflow
		return -1; // Buffer is full. Since it is being used live at the point of the upload we cannot do much here but to abort.
	}

	float *mBufferPointer = (float*)mBuffer->Memory();
	assert(mBufferPointer != nullptr);
	if(mBufferPointer == nullptr) return -1;

	assert(out.Size() <= mMaxUploadSize);
	assert(thisindex + out.Size() <= mBufferSize);

	memcpy(mBufferPointer + (thisindex * 4), out.Data(), out.Size() * sizeof(float));

	return thisindex;
}

int FLightBuffer::GetBinding(unsigned int index, size_t* pOffset, size_t* pSize)
{
	// this function will only get called if a uniform buffer is used. For a shader storage buffer we only need to bind the buffer once at the start.
	unsigned int offset = (index / mBlockAlign) * mBlockAlign;

	*pOffset = offset * ELEMENT_SIZE;
	*pSize = mBlockSize * ELEMENT_SIZE;
	return (index - offset);
}
