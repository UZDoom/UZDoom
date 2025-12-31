/*
** hw_bonebuffer.cpp
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

#include "hw_bonebuffer.h"
#include "hw_dynlightdata.h"
#include "shaderuniforms.h"

static const int BONE_SIZE = (16*sizeof(float));

BoneBuffer::BoneBuffer(int pipelineNbr) : mPipelineNbr(pipelineNbr)
{
	int maxNumberOfBones = 80000;

	mBufferSize = maxNumberOfBones;
	mByteSize = mBufferSize * BONE_SIZE;

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
		mBlockSize = screen->maxuniformblock / BONE_SIZE;
		mBlockAlign = screen->uniformblockalignment < 64 ? 1 : screen->uniformblockalignment / BONE_SIZE;
		mMaxUploadSize = (mBlockSize - mBlockAlign);
	}

	for (int n = 0; n < mPipelineNbr; n++)
	{
		mBufferPipeline[n] = screen->CreateDataBuffer(BONEBUF_BINDINGPOINT, mBufferType, false);
		mBufferPipeline[n]->SetData(mByteSize, nullptr, BufferUsageType::Persistent);
	}

	Clear();
}

BoneBuffer::~BoneBuffer()
{
	delete mBuffer;
}

void BoneBuffer::Clear()
{
	mIndex = 0;

	mPipelinePos++;
	mPipelinePos %= mPipelineNbr;

	mBuffer = mBufferPipeline[mPipelinePos];
}

int BoneBuffer::UploadBones(const TArray<VSMatrix>& bones)
{
	Map();
	int totalsize = bones.Size();
	if (totalsize > (int)mMaxUploadSize)
	{
		totalsize = mMaxUploadSize;
	}

	uint8_t *mBufferPointer = (uint8_t*)mBuffer->Memory();
	assert(mBufferPointer != nullptr);
	if (mBufferPointer == nullptr) return -1;
	if (totalsize <= 0) return -1;	// there are no bones

	unsigned int thisindex = mIndex.fetch_add(totalsize);

	if (thisindex + totalsize <= mBufferSize)
	{
		memcpy(mBufferPointer + thisindex * BONE_SIZE, bones.Data(), totalsize * BONE_SIZE);
		Unmap();
		return thisindex;
	}
	else
	{
		Unmap();
		return -1;	// Buffer is full. Since it is being used live at the point of the upload we cannot do much here but to abort.
	}
}

int BoneBuffer::GetBinding(unsigned int index, size_t* pOffset, size_t* pSize)
{
	// this function will only get called if a uniform buffer is used. For a shader storage buffer we only need to bind the buffer once at the start.
	unsigned int offset = (index / mBlockAlign) * mBlockAlign;

	*pOffset = offset * BONE_SIZE;
	*pSize = mBlockSize * BONE_SIZE;
	return (index - offset);
}
