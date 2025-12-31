/*
** hw_modelvertexbuffer.h
**
** hardware renderer model handling code
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "hwrenderer/data/buffers.h"
#include "i_modelvertexbuffer.h"

class FModelRenderer;

class FModelVertexBuffer : public IModelVertexBuffer
{
	IVertexBuffer *mVertexBuffer;
	IIndexBuffer *mIndexBuffer;

public:

	FModelVertexBuffer(bool needindex, bool singleframe);
	~FModelVertexBuffer();

	FModelVertex *LockVertexBuffer(unsigned int size) override;
	void UnlockVertexBuffer() override;

	unsigned int *LockIndexBuffer(unsigned int size) override;
	void UnlockIndexBuffer() override;

	IVertexBuffer* vertexBuffer() const { return mVertexBuffer; }
	IIndexBuffer* indexBuffer() const { return mIndexBuffer; }
};
