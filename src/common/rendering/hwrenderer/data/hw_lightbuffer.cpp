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

//==========================================================================
//
//
//
//==========================================================================

int FLightBuffer::UploadLights(const FDynLightData &data)
{ // custom upload because it uploads partial light data when out of space
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
