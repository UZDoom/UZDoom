/*
** gl_framebuffer.h
**
** Implementation of non-hardware specific parts of the OpenGL frame buffer
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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

#ifndef __GL_FRAMEBUFFER
#define __GL_FRAMEBUFFER

#include "gl_sysfb.h"
#include "m_png.h"

#include <memory>

namespace OpenGLRenderer
{

class FHardwareTexture;
class FGLDebug;

class OpenGLFrameBuffer : public SystemGLFrameBuffer
{
	typedef SystemGLFrameBuffer Super;

	void RenderTextureView(FCanvasTexture* tex, std::function<void(IntRect &)> renderFunc) override;

public:

	OpenGLFrameBuffer(void *hMonitor, bool fullscreen) ;
	~OpenGLFrameBuffer();
	int Backend() override { return 2; }
	bool CompileNextShader() override;
	void InitializeState() override;
	void Update() override;

	void AmbientOccludeScene(float m5) override;
	void FirstEye() override;
	void NextEye(int eyecount) override;
	void SetSceneRenderTarget(bool useSSAO) override;
	void UpdateShadowMap() override;
	void WaitForCommands(bool finish) override;
	void SetSaveBuffers(bool yes) override;
	void CopyScreenToBuffer(int width, int height, uint8_t* buffer) override;
	bool FlipSavePic() const override { return true; }

	FRenderState* RenderState() override;
	void UpdatePalette() override;
	const char* DeviceName() const override;
	void SetTextureFilterMode() override;
	IHardwareTexture *CreateHardwareTexture(int numchannels) override;
	void PrecacheMaterial(FMaterial *mat, int translation) override;
	void BeginFrame() override;
	void SetViewportRects(IntRect *bounds) override;
	void BlurScene(float amount) override;
	IVertexBuffer *CreateVertexBuffer() override;
	IIndexBuffer *CreateIndexBuffer() override;
	IDataBuffer *CreateDataBuffer(int bindingpoint, bool ssbo, bool needsresize) override;

	void InitLightmap(int LMTextureSize, int LMTextureCount, TArray<uint16_t>& LMTextureData) override;

	// Retrieves a buffer containing image data for a screenshot.
	// Hint: Pitch can be negative for upside-down images, in which case buffer
	// points to the last row in the buffer, which will be the first row output.
	virtual TArray<uint8_t> GetScreenshotBuffer(int &pitch, ESSType &color_type, float &gamma) override;

	void Swap();
	bool IsHWGammaActive() const { return HWGammaActive; }

	void SetVSync(bool vsync) override;

	void Draw2D() override;
	void PostProcessScene(bool swscene, int fixedcm, float flash, const std::function<void()> &afterBloomDrawEndScene2D) override;

	bool HWGammaActive = false;			// Are we using hardware or software gamma?
	std::unique_ptr<FGLDebug> mDebug;	// Debug API

    FTexture *WipeStartScreen() override;
    FTexture *WipeEndScreen() override;

	int camtexcount = 0;
};

}

#endif //__GL_FRAMEBUFFER
