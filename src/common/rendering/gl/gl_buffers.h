/*
** gl_buffers.h
**
** Low level vertex buffer class
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2020 Christoph Oelckers
** Copyright 2018-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "buffers.h"
#include "gl_load.h"

#ifdef _MSC_VER
// silence bogus warning C4250: 'GLVertexBuffer': inherits 'GLBuffer::GLBuffer::SetData' via dominance
// According to internet infos, the warning is erroneously emitted in this case.
#pragma warning(disable:4250)
#endif

namespace OpenGLRenderer
{

class GLBuffer : virtual public IBuffer, virtual public ILockableBuffer
{
protected:
	const int mUseType;
	unsigned int mBufferId;
	int mAllocationSize = 0;
	bool mPersistent = false;
	bool nomap = true;
	GLsync mGLSync = 0;

	GLBuffer(int usetype);
	~GLBuffer();
	virtual void SetData(size_t size, const void *data, BufferUsageType usage) override;
	virtual void SetSubData(size_t offset, size_t size, const void *data) override;
	virtual void Map() override;
	virtual void Unmap() override;
	virtual void Resize(size_t newsize) override;
	virtual void *Lock(unsigned int size) override;
	virtual void Unlock() override;

	void GPUDropSync();
	void GPUWaitSync();
public:
	void Bind();
};


class GLVertexBuffer : public IVertexBuffer, public GLBuffer
{
	// If this could use the modern (since GL 4.3) binding system, things would be simpler... :(
	struct GLVertexBufferAttribute
	{
		int bindingpoint;
		int format;
		bool normalize;
		bool integerType;
		int size;
		int offset;
	};

	int mNumBindingPoints;
	GLVertexBufferAttribute mAttributeInfo[VATTR_MAX] = {};	// Thanks to OpenGL's state system this needs to contain info about every attribute that may ever be in use throughout the entire renderer.
	size_t mStride = 0;

public:
	GLVertexBuffer();
	void SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs) override;
	void Bind(int *offsets);
};

class GLIndexBuffer : public IIndexBuffer, public GLBuffer
{
public:
	GLIndexBuffer();
};

class GLDataBuffer : public IDataBuffer, public GLBuffer
{
	int mBindingPoint;
public:
	GLDataBuffer(int bindingpoint, bool is_ssbo);
	virtual void BindRange(FRenderState* state, size_t start, size_t length) override;
	void BindBase();
};

}
