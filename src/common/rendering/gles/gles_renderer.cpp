/*
** gles_renderer.cpp
**
** Renderer interface
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
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include "gles_system.h"
#include "files.h"
#include "v_video.h"
#include "m_png.h"
#include "filesystem.h"
#include "i_time.h"
#include "cmdlib.h"
#include "version.h"
#include "gles_framebuffer.h"
#include "hw_cvars.h"
#include "gles_renderer.h"
#include "gles_hwtexture.h"
#include "gles_renderstate.h"
#include "gles_samplers.h"
#include "gles_renderbuffers.h"
#include "gles_shaderprogram.h"
#include "flatvertices.h"
#include "hw_lightbuffer.h"
#include "r_videoscale.h"
#include "model.h"
#include "gles_postprocessstate.h"
#include "gles_buffers.h"
#include "texturemanager.h"

EXTERN_CVAR(Int, screenblocks)

namespace OpenGLESRenderer
{

//===========================================================================
// 
// Renderer interface
//
//===========================================================================

//-----------------------------------------------------------------------------
//
// Initialize
//
//-----------------------------------------------------------------------------

FGLRenderer::FGLRenderer(OpenGLFrameBuffer *fb) 
{
	framebuffer = fb;
}

void FGLRenderer::Initialize(int width, int height)
{
	mScreenBuffers = new FGLRenderBuffers();
	mBuffers = mScreenBuffers;
	mPresentShader = new FPresentShader();

	mFBID = 0;
	mOldFBID = 0;

	mShaderManager = new FShaderManager;
	mSamplerManager = new FSamplerManager;
}

FGLRenderer::~FGLRenderer() 
{
	FlushModels();
	TexMan.FlushAll();
	if (mShaderManager != nullptr) delete mShaderManager;
	if (mFBID != 0) glDeleteFramebuffers(1, &mFBID);

	if (mBuffers) delete mBuffers;
	if (mPresentShader) delete mPresentShader;
}

//===========================================================================
// 
//
//
//===========================================================================

bool FGLRenderer::StartOffscreen()
{
	if (mFBID == 0)
		glGenFramebuffers(1, &mFBID);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFBID);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBID);
	return true;
}

//===========================================================================
// 
//
//
//===========================================================================

void FGLRenderer::EndOffscreen()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mOldFBID); 
}

//===========================================================================
//
//
//
//===========================================================================

void FGLRenderer::BindToFrameBuffer(FTexture *tex)
{
	auto BaseLayer = static_cast<FHardwareTexture*>(tex->GetHardwareTexture(0, 0));
	// must create the hardware texture first
	BaseLayer->BindOrCreate(tex, 0, 0, 0, 0);
	FHardwareTexture::Unbind(0);
	gl_RenderState.ClearLastMaterial();
	BaseLayer->BindToFrameBuffer(tex->GetWidth(), tex->GetHeight());
}

//===========================================================================
//
//
//
//===========================================================================

void FGLRenderer::BeginFrame()
{
	mScreenBuffers->Setup(screen->mScreenViewport.width, screen->mScreenViewport.height, screen->mSceneViewport.width, screen->mSceneViewport.height);
}

}
