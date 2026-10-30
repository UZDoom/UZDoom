/*
** hw_bonebuffer.h
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

#pragma once

#include "tarray.h"
#include "hwrenderer/data/buffers.h"
#include "common/utility/matrix.h"
#include <atomic>
#include <mutex>

class FRenderState;

class BoneBuffer : public FBufferContainer
{
public:
	static constexpr int MAX_NUMBER_OF_BONES = 80000;

	BoneBuffer(int pipelineNbr = 1)
	:FBufferContainer(BufferType::Data, BufferUsageType::Persistent, MAX_NUMBER_OF_BONES, sizeof(VSMatrix), BONEBUF_BINDINGPOINT, pipelineNbr, false)
	{
	}
	int UploadBones(const TArray<VSMatrix>& bones)
	{
		return UploadData<VSMatrix>(bones);
	}
};
