/*
** buffers.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2019 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <stddef.h>
#include <assert.h>

class FRenderState;

#ifdef __ANDROID__
#define HW_MAX_PIPELINE_BUFFERS 4
#define HW_BLOCK_SSBO 1
#else
// On desktop this is only useful fpr letting the GPU run in parallel with the playsim and for that 2 buffers are enough.
#define HW_MAX_PIPELINE_BUFFERS 2
#endif

// The low level code needs to know which attributes exist.
// OpenGL needs to change the state of all of them per buffer binding.
// VAOs are mostly useless for this because they lump buffer and binding state together which the model code does not want.
enum
{
	VATTR_VERTEX,
	VATTR_TEXCOORD,
	VATTR_COLOR,
	VATTR_VERTEX2,
	VATTR_NORMAL,
	VATTR_NORMAL2,
	VATTR_LIGHTMAP,	
	VATTR_BONEWEIGHT,
	VATTR_BONESELECTOR,
	VATTR_MAX
};

enum EVertexAttributeFormat
{
	VFmt_Float4,
	VFmt_Float3,
	VFmt_Float2,
	VFmt_Float,
	VFmt_Byte4,
	VFmt_Packed_A2R10G10B10,
	VFmt_Byte4_UInt
};

struct FVertexBufferAttribute
{
	int binding;
	int location;
	int format;
	int offset;
};

enum class BufferUsageType
{
	Static,     // initial data is not null, staticdata is true
	Stream,     // initial data is not null, staticdata is false
	Persistent, // initial data is null, staticdata is false
	Mappable    // initial data is null, staticdata is true
};

class IBuffer
{
protected:
	size_t buffersize = 0;
	void *map = nullptr;
public:
	IBuffer() = default;
	IBuffer(const IBuffer &) = delete;
	IBuffer &operator=(const IBuffer &) = delete;
	virtual ~IBuffer() = default;

	virtual void SetData(size_t size, const void *data, BufferUsageType type) = 0;
	virtual void SetSubData(size_t offset, size_t size, const void *data) = 0;
	virtual void *Lock(unsigned int size) = 0;
	virtual void Unlock() = 0;
	virtual void Resize(size_t newsize) = 0;

	virtual void Upload(size_t start, size_t size) {} // For unmappable buffers

	virtual void Map() {}		// Only needed by old OpenGL but this needs to be in the interface.
	virtual void Unmap() {}
	void *Memory() { return map; }
	size_t Size() { return buffersize; }
	virtual void GPUDropSync() {}
	virtual void GPUWaitSync() {}
};

class IVertexBuffer : virtual public IBuffer
{
public:
	virtual void SetFormat(int numBindingPoints, int numAttributes, size_t stride, const FVertexBufferAttribute *attrs) = 0;
};

// This merely exists to have a dedicated type for index buffers to inherit from.
class IIndexBuffer : virtual public IBuffer
{
	// Element size is fixed to 4, thanks to OpenGL requiring this info to be coded into the glDrawElements call.
	// This mostly prohibits a more flexible buffer setup but GZDoom doesn't use any other format anyway.
	// Ob Vulkam, element size is a buffer property and of no concern to the drawing functions (as it should be.)
};

class IDataBuffer : virtual public IBuffer
{
	// Can be either uniform or shader storage buffer, depending on its needs.
public:
	virtual void BindRange(FRenderState *state, size_t start, size_t length) = 0;

};
