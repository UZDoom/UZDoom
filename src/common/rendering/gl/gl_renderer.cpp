/*
** gl_renderer.cpp
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

#include "gl_system.h"
#include "files.h"
#include "v_video.h"
#include "m_png.h"
#include "filesystem.h"
#include "i_time.h"
#include "cmdlib.h"
#include "version.h"
#include "gl_interface.h"
#include "gl_framebuffer.h"
#include "hw_cvars.h"
#include "gl_debug.h"
#include "gl_renderer.h"
#include "gl_renderstate.h"
#include "gl_renderbuffers.h"
#include "gl_shaderprogram.h"
#include "flatvertices.h"
#include "gl_samplers.h"
#include "hw_lightbuffer.h"
#include "r_videoscale.h"
#include "model.h"
#include "gl_postprocessstate.h"
#include "gl_buffers.h"
#include "texturemanager.h"

EXTERN_CVAR(Int, screenblocks)

namespace OpenGLRenderer
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
	mSaveBuffers = new FGLRenderBuffers();
	mBuffers = mScreenBuffers;
	mPresentShader = new FPresentShader();
	mPresent3dCheckerShader = new FPresent3DCheckerShader();
	mPresent3dColumnShader = new FPresent3DColumnShader();
	mPresent3dRowShader = new FPresent3DRowShader();
	mShadowMapShader = new FShadowMapShader();

	// needed for the core profile, because someone decided it was a good idea to remove the default VAO.
	glGenVertexArrays(1, &mVAOID);
	glBindVertexArray(mVAOID);
	FGLDebug::LabelObject(GL_VERTEX_ARRAY, mVAOID, "FGLRenderer.mVAOID");

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
	if (mSamplerManager != nullptr) delete mSamplerManager;
	if (mFBID != 0) glDeleteFramebuffers(1, &mFBID);
	if (mVAOID != 0)
	{
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &mVAOID);
	}
	if (mBuffers) delete mBuffers;
	if (mSaveBuffers) delete mSaveBuffers;
	if (mPresentShader) delete mPresentShader;
	if (mPresent3dCheckerShader) delete mPresent3dCheckerShader;
	if (mPresent3dColumnShader) delete mPresent3dColumnShader;
	if (mPresent3dRowShader) delete mPresent3dRowShader;
	if (mShadowMapShader) delete mShadowMapShader;
}

//===========================================================================
// 
//
//
//===========================================================================

bool FGLRenderer::StartOffscreen()
{
	bool firstBind = (mFBID == 0);
	if (mFBID == 0)
		glGenFramebuffers(1, &mFBID);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFBID);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBID);
	if (firstBind)
		FGLDebug::LabelObject(GL_FRAMEBUFFER, mFBID, "OffscreenFB");
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
	mSaveBuffers->Setup(SAVEPICWIDTH, SAVEPICHEIGHT, SAVEPICWIDTH, SAVEPICHEIGHT);
}

}
