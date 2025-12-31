/*
** gl_samplers.cpp
**
** Texture sampler handling
**
**---------------------------------------------------------------------------
**
** Copyright 2015-2019 Christoph Oelckers
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
#include "c_cvars.h"

#include "gl_interface.h"
#include "hw_cvars.h"
#include "gl_debug.h"
#include "gl_renderer.h"
#include "gl_samplers.h"
#include "hw_material.h"
#include "i_interface.h"

namespace OpenGLRenderer
{

extern TexFilter_s TexFilter[];


FSamplerManager::FSamplerManager()
{
	glGenSamplers(NUMSAMPLERS, mSamplers);

	glSamplerParameteri(mSamplers[CLAMP_X], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_Y], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_XY], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_XY], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_NOFILTER_X], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_NOFILTER_Y], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_NOFILTER_XY], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_NOFILTER_XY], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	for (int i = CLAMP_NOFILTER; i <= CLAMP_NOFILTER_XY; i++)
	{
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameterf(mSamplers[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);

	}

	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameterf(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);
	glSamplerParameterf(mSamplers[CLAMP_CAMTEX], GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.f);

	SetTextureFilterMode();


	for (int i = 0; i < NUMSAMPLERS; i++)
	{
		FString name;
		name.Format("mSamplers[%d]", i);
		FGLDebug::LabelObject(GL_SAMPLER, mSamplers[i], name.GetChars());
	}
}

FSamplerManager::~FSamplerManager()
{
	UnbindAll();
	glDeleteSamplers(NUMSAMPLERS, mSamplers);
}

void FSamplerManager::UnbindAll()
{
	for (int i = 0; i < IHardwareTexture::MAX_TEXTURES; i++)
	{
		glBindSampler(i, 0);
	}
}

uint8_t FSamplerManager::Bind(int texunit, int num, int lastval)
{
	unsigned int samp = mSamplers[num];
	glBindSampler(texunit, samp);
	return 255;
}


void FSamplerManager::SetTextureFilterMode()
{
	GLint bounds[IHardwareTexture::MAX_TEXTURES];

	// Unbind all
	for(int i = IHardwareTexture::MAX_TEXTURES-1; i >= 0; i--)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glGetIntegerv(GL_SAMPLER_BINDING, &bounds[i]);
		glBindSampler(i, 0);
	}

	int filter = sysCallbacks.DisableTextureFilter && sysCallbacks.DisableTextureFilter() ? 0 : gl_texture_filter;
	float aniso  = filter <= 0 || (sysCallbacks.DisableAnisotropicFiltering && sysCallbacks.DisableAnisotropicFiltering()) ? 1.0f : gl_texture_filter_anisotropic;

	for (int i = 0; i < 4; i++)
	{
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
		glSamplerParameterf(mSamplers[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	}
	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_CAMTEX], GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_CAMTEX], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
	for(int i = 0; i < IHardwareTexture::MAX_TEXTURES; i++)
	{
		glBindSampler(i, bounds[i]);
	}
}


}
