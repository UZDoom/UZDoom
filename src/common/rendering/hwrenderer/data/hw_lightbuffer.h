/*
** hw_lightbuffer.h
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

#ifndef __GL_LIGHTBUFFER_H
#define __GL_LIGHTBUFFER_H

#include "tarray.h"
#include "hw_dynlightdata.h"
#include "hwrenderer/data/buffers.h"
#include <atomic>
#include <mutex>

class FRenderState;

class FLightBuffer : public FBufferContainer
{
public:
	static constexpr int ELEMENTS_PER_LIGHT = 4;			// each light needs 4 vec4's.
	static constexpr int ELEMENT_SIZE = (4*sizeof(float));
	static constexpr int MAX_NUMBER_OF_LIGHTS = 80000;

	FLightBuffer(int pipelineNbr = 1)
	:FBufferContainer(BufferType::Data, BufferUsageType::Persistent, MAX_NUMBER_OF_LIGHTS * ELEMENTS_PER_LIGHT, ELEMENT_SIZE, LIGHTBUF_BINDINGPOINT, pipelineNbr, false)
	{
	}
	int UploadLights(const FDynLightData &data);
};


#endif
