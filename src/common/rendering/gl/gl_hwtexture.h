/*
** gl_hwtexture.h
**
** GL texture abstraction
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
class FBitmap;
class FTexture;

#include "tarray.h"
#include "hw_ihwtexture.h"


#ifdef LoadImage
#undef LoadImage
#endif

#define SHADED_TEXTURE -1
#define DIRECT_PALETTE -2

#include "tarray.h"
#include "gl_interface.h"
#include "hw_ihwtexture.h"

class FCanvasTexture;

namespace OpenGLRenderer
{

class FHardwareTexture : public IHardwareTexture
{
public:

	static unsigned int lastbound[MAX_TEXTURES];

	static int GetTexDimension(int value)
	{
		if (value > gl.max_texturesize) return gl.max_texturesize;
		return value;
	}

	static void InitGlobalState() { for (int i = 0; i < MAX_TEXTURES; i++) lastbound[i] = 0; }

private:

	bool forcenofilter;

	unsigned int glTexID = 0;
	unsigned int glDepthID = 0;	// only used by camera textures
	unsigned int glBufferID = 0;
	int glTextureBytes;
	bool mipmapped = false;

	int GetDepthBuffer(int w, int h);

public:
	FHardwareTexture(int numchannels = 4, bool disablefilter = false)
	{
		forcenofilter = disablefilter;
		glTextureBytes = numchannels;
	}

	~FHardwareTexture();

	static void Unbind(int texunit);
	static void UnbindAll();

	void BindToFrameBuffer(int w, int h);

	unsigned int Bind(int texunit, bool needmipmap);
	bool BindOrCreate(FTexture* tex, int texunit, int clampmode, int translation, int flags);

	void AllocateBuffer(int w, int h, int texelsize);
	uint8_t* MapBuffer();

	unsigned int CreateTexture(unsigned char* buffer, int w, int h, int texunit, bool mipmap, const char* name);
	unsigned int GetTextureHandle()
	{
		return glTexID;
	}

	int numChannels() { return glTextureBytes; }
};

}
