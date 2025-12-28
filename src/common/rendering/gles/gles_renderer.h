/*
** gles_renderer.h
**
** Renderer interface
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

#ifndef __GL_RENDERER_H
#define __GL_RENDERER_H

#include "v_video.h"
#include "vectors.h"
#include "matrix.h"
#include "gles_renderbuffers.h"
#include <functional>

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

struct particle_t;
class FCanvasTexture;
class FFlatVertexBuffer;
class FSkyVertexBuffer;
class HWPortal;
class FLightBuffer;
class BoneBuffer;
class DPSprite;
class FGLRenderBuffers;
class FGL2DDrawer;
class SWSceneDrawer;
class HWViewpointBuffer;
struct FRenderViewpoint;

namespace OpenGLESRenderer
{
	class FHardwareTexture;
	class FShaderManager;
	class FSamplerManager;
	class OpenGLFrameBuffer;
	class FPresentShaderBase;
	class FPresentShader;
	class FPresent3DCheckerShader;
	class FPresent3DColumnShader;
	class FPresent3DRowShader;
	class FShadowMapShader;

class FGLRenderer
{
public:

	OpenGLFrameBuffer *framebuffer;
	int mMirrorCount = 0;
	int mPlaneMirrorCount = 0;
	FShaderManager *mShaderManager = nullptr;
	FSamplerManager* mSamplerManager = nullptr;
	unsigned int mFBID = 0;
	unsigned int mStencilValue = 0;

	int mOldFBID = 0;

	FGLRenderBuffers *mBuffers = nullptr;
	FGLRenderBuffers *mScreenBuffers = nullptr;
	FPresentShader *mPresentShader = nullptr;

	//FRotator mAngles;

	FGLRenderer(OpenGLFrameBuffer *fb);
	~FGLRenderer() ;

	void Initialize(int width, int height);

	void ClearBorders();

	void PresentStereo();
	void RenderScreenQuad();
	void PostProcessScene(int fixedcm, float flash, const std::function<void()> &afterBloomDrawEndScene2D);

	void CopyToBackbuffer(const IntRect *bounds, bool applyGamma);
	void DrawPresentTexture(const IntRect &box, bool applyGamma);
	void Flush();
	void BeginFrame();

	bool StartOffscreen();
	void EndOffscreen();

	void BindToFrameBuffer(FTexture* tex);

private:

	bool QuadStereoCheckInitialRenderContextState();
	void PresentAnaglyph(bool r, bool g, bool b);
	void PresentSideBySide(int);
	void PresentTopBottom();
	void prepareInterleavedPresent(FPresentShaderBase& shader);
	void PresentColumnInterleaved();
	void PresentRowInterleaved();
	void PresentCheckerInterleaved();
	void PresentQuadStereo();

};

struct TexFilter_s
{
	int minfilter;
	int magfilter;
	bool mipmapping;
} ;


extern FGLRenderer *GLRenderer;

}
#endif
