/*
** gles_buffers.cpp
**
** Low level vertex buffer class
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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

#include <algorithm>
#include "gles_system.h"
#include "gles_buffers.h"
#include "gles_renderstate.h"
#include "v_video.h"
#include "flatvertices.h"

namespace OpenGLESRenderer
{

//==========================================================================
//
// basic buffer implementation
//
//==========================================================================

static inline void InvalidateBufferState()
{
	gl_RenderState.ResetVertexBuffer();	// force rebinding of buffers on next Apply call.
}

GLBuffer::GLBuffer(int usetype)
	: mUseType(usetype)
{
	if ((usetype == GL_ARRAY_BUFFER) || (usetype == GL_ELEMENT_ARRAY_BUFFER))
	{
		glGenBuffers(1, &mBufferId);
		isData = false;
	}
	else
	{
		isData = true;
	}
}

GLBuffer::~GLBuffer()
{
	if (mBufferId != 0)
	{
		if (gles.useMappedBuffers)
		{
			glBindBuffer(mUseType, mBufferId);
			glUnmapBuffer(mUseType);
		}
		glBindBuffer(mUseType, 0);
		glDeleteBuffers(1, &mBufferId);
	}

	if (memory)
		delete[] memory;
}

void GLBuffer::Bind()
{
	if (!isData)
	{
		glBindBuffer(mUseType, mBufferId);
	}
}


void GLBuffer::SetData(size_t size, const void* data, BufferUsageType usage)
{
	bool staticdata = (usage == BufferUsageType::Static || usage == BufferUsageType::Mappable);
	if (isData || !gles.useMappedBuffers)
	{
		if (memory)
			delete[] memory;

		memory = (char*)(new uint64_t[size / 8 + 16]);

		if (data)
			memcpy(memory, data, size);
	}

	if (!isData)
	{
		Bind();
		glBufferData(mUseType, size, data, staticdata ? GL_STATIC_DRAW : GL_STREAM_DRAW);
	}

	if (!isData && gles.useMappedBuffers)
	{
		map = 0;
	}
	else
	{
		map = memory;
	}

	buffersize = size;
	InvalidateBufferState();
}

void GLBuffer::SetSubData(size_t offset, size_t size, const void *data)
{
	Bind();

	memcpy(memory + offset, data, size);

	if (!isData)
	{
		glBufferSubData(mUseType, offset, size, data);
	}
}

void GLBuffer::Upload(size_t start, size_t size)
{
	if (!gles.useMappedBuffers)
	{
		Bind();

		if(size)
			glBufferSubData(mUseType, start, size, memory + start);
	}
}

void GLBuffer::Map()
{
	if (!isData && gles.useMappedBuffers)
	{
		Bind();
		map = (FFlatVertex*)glMapBufferRange(mUseType, 0, buffersize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}
	else
	{
		map = memory;
	}
	InvalidateBufferState();
}

void GLBuffer::Unmap()
{
	if (!isData && gles.useMappedBuffers)
	{
		Bind();
		glUnmapBuffer(mUseType);
		InvalidateBufferState();
	}
}

void *GLBuffer::Lock(unsigned int size)
{
	// This initializes this buffer as a static object with no data.
	SetData(size, nullptr, BufferUsageType::Mappable);
	if (!isData && gles.useMappedBuffers)
	{
		return glMapBufferRange(mUseType, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	}
	else
	{
		return map;
	}
}

void GLBuffer::Unlock()
{
	if (!isData)
	{
		if (gles.useMappedBuffers)
		{
			Bind();
			glUnmapBuffer(mUseType);
			InvalidateBufferState();
		}
		else
		{
			Bind();
			glBufferData(mUseType, buffersize, map, GL_STATIC_DRAW);
			InvalidateBufferState();
		}
	}
}

void GLBuffer::Resize(size_t newsize)
{ 
	if (newsize > buffersize)
	{
		if (isData)
		{
			// Create new bigger memory
			char* memoryNew = (char*)(new uint64_t[newsize / 8 + 16]);

			// Copy old data
			memcpy(memoryNew, memory, buffersize);

			// Delete old memory
			delete[] memory;

			memory = memoryNew;

			buffersize = newsize;
		}
		else
		{
			// TODO
		}
	}
}

void GLBuffer::GPUDropSync()
{
	if (gles.glesMode > GLES_MODE_GLES && gles.useMappedBuffers && glFenceSync && glDeleteSync)
	{
		if (mGLSync != NULL)
		{
			glDeleteSync(mGLSync);
		}

		mGLSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
}

void GLBuffer::GPUWaitSync()
{
	if (gles.glesMode > GLES_MODE_GLES && gles.useMappedBuffers && glDeleteSync && glClientWaitSync)
	{
		GLenum status = glClientWaitSync(mGLSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000 * 1000 * 50); // Wait for a max of 50ms...

		if (status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED)
		{
			//Printf("Error on glClientWaitSync: %d\n", status);
		}

		glDeleteSync(mGLSync);

		mGLSync = NULL;
	}
}


//===========================================================================
//
// Vertex buffer implementation
//
//===========================================================================

void GLVertexBuffer::SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs)
{
	static int VFmtToGLFmt[] = { GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE, GL_INT_2_10_10_10_REV, GL_UNSIGNED_BYTE }; // TODO Fix last entry GL_INT_2_10_10_10_REV, normals for models will be broken
	static uint8_t VFmtToSize[] = {4, 3, 2, 1, 4, 4, 4};
	static bool VFmtToNormalize[] = { false, false, false, false, true, true, false };
	static bool VFmtToIntegerType[] = { false, false, false, false, false, false, true };

	mStride = stride;
	mNumBindingPoints = numBindingPoints;

	for(int i = 0; i < numAttributes; i++)
	{
		if (attrs[i].location >= 0 && attrs[i].location < VATTR_MAX)
		{
			auto & attrinf = mAttributeInfo[attrs[i].location];
			attrinf.format = VFmtToGLFmt[attrs[i].format];
			attrinf.size = VFmtToSize[attrs[i].format];
			attrinf.offset = attrs[i].offset;
			attrinf.bindingpoint = attrs[i].binding;
			attrinf.normalize = VFmtToNormalize[attrs[i].format];
			attrinf.integerType = VFmtToIntegerType[attrs[i].format];
		}
	}
}

void GLVertexBuffer::Bind(int *offsets)
{
	int i = 0;

	// This is what gets called from RenderState.Apply. It shouldn't be called anywhere else if the render state is in use
	GLBuffer::Bind();
	for(auto &attrinf : mAttributeInfo)
	{
		if (attrinf.size == 0)
		{
			glDisableVertexAttribArray(i);
		}
		else
		{
			glEnableVertexAttribArray(i);
			size_t ofs = offsets == nullptr ? attrinf.offset : attrinf.offset + mStride * offsets[attrinf.bindingpoint];
			if (!attrinf.integerType)
				glVertexAttribPointer(i, attrinf.size, attrinf.format, attrinf.normalize, (GLsizei)mStride, (void*)(intptr_t)ofs);
			else
			{
				if (gles.glesMode >= GLES_MODE_OGL3)
					glVertexAttribIPointer(i, attrinf.size, attrinf.format, (GLsizei)mStride, (void*)(intptr_t)ofs);
			}
		}
		i++;
	}
}

void GLDataBuffer::BindRange(FRenderState *state, size_t start, size_t length)
{
	if (mBindingPoint == 3)// VIEWPOINT_BINDINGPOINT
	{
		static_cast<FGLRenderState*>(state)->ApplyViewport(memory + start);
	} 
}

void GLDataBuffer::BindBase()
{

}


GLVertexBuffer::GLVertexBuffer() : GLBuffer(GL_ARRAY_BUFFER) {}
GLIndexBuffer::GLIndexBuffer() : GLBuffer(GL_ELEMENT_ARRAY_BUFFER) {}
GLDataBuffer::GLDataBuffer(int bindingpoint, bool is_ssbo) : GLBuffer(0), mBindingPoint(bindingpoint) {}

}
