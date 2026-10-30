/*
** buffers.cpp
**
**
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

#include "buffers.h"
#include "v_video.h"

//==========================================================================
//
// FBufferContainer, TODO move to its own file
//
//==========================================================================

IBuffer * FBufferContainer::CreateBuffer()
{
	switch(mBufferType)
	{
	case BufferType::Data:
		return screen->CreateDataBuffer(mBindingPoint, mBufferSSBO, mNeedsResize);
	case BufferType::Index:
		return screen->CreateIndexBuffer();
	case BufferType::Vertex:
		return screen->CreateVertexBuffer();
	}
}

FBufferContainer::FBufferContainer(BufferType type, BufferUsageType usageType, unsigned numElements, unsigned elementSize, int bindingPoint, int pipelineNbr, bool needsResize)
	:	mBufferSize(numElements),
		mElementSize(elementSize),
		mByteSize(numElements * elementSize),
		mBindingPoint(bindingPoint),
		mPipelineNbr(pipelineNbr > 0 ? pipelineNbr : 1),
		mBufferType(type),
		mUsageType(usageType),
		mNeedsResize(needsResize)
{
	if(mBufferType == BufferType::Data)
	{
		assert(bindingPoint >= 0);
		mBufferSSBO = screen->useSSBO();
	}
	else
	{
		mBufferSSBO = false;
	}
	if(mBufferSSBO)
	{
		mBlockAlign = 0;
		mBlockSize = mBufferSize;
		mMaxUploadSize = mBlockSize;
	}
	else
	{
		mBlockSize = screen->maxuniformblock / mElementSize;
		mBlockAlign = (screen->uniformblockalignment <= mElementSize) ? 1 : (screen->uniformblockalignment / mElementSize);
		mMaxUploadSize = (mBlockSize - mBlockAlign);
	}

	for (int n = 0; n < mPipelineNbr; n++)
	{
		mBufferPipeline[n] = CreateBuffer();
		mBufferPipeline[n]->SetData(mByteSize, nullptr, mUsageType);
	}

	mIndex = 0;
	mPipelinePos = 0;
	mBuffer = mBufferPipeline[0];
}


int FBufferContainer::UploadData(void * data, size_t sz)
{
	if (!data || sz == 0 || sz > (int)mMaxUploadSize)
	{
		return -1;
	}

	uint8_t *mBufferPointer = (uint8_t*)mBuffer->Memory();
	assert(mBufferPointer != nullptr);
	if(mBufferPointer == nullptr) return -1;

	unsigned int thisindex = std::min(mIndex.fetch_add(sz), mBufferSize + 1);

	if (thisindex + sz <= mBufferSize)
	{
		memcpy(mBufferPointer + (thisindex * mElementSize), data, sz * mElementSize);
		return thisindex;
	}
	else
	{
		mIndex.store(thisindex); // don't let it overflow
		return -1;	// Buffer is full. Since it is being used live at the point of the upload we cannot do much here but to abort.
	}
}
