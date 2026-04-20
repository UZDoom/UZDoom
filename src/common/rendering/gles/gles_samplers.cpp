/*
** gles_samplers.cpp
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

#include "gles_system.h"
#include "c_cvars.h"

#include "hw_cvars.h"

#include "gles_renderer.h"
#include "gles_samplers.h"
#include "hw_material.h"
#include "i_interface.h"

namespace OpenGLESRenderer
{

extern TexFilter_s TexFilter[];


FSamplerManager::FSamplerManager()
{
	SetTextureFilterMode();
}

FSamplerManager::~FSamplerManager()
{

}

void FSamplerManager::UnbindAll()
{

}

uint8_t FSamplerManager::Bind(int texunit, int num, int lastval)
{

	int filter = sysCallbacks.DisableTextureFilter && sysCallbacks.DisableTextureFilter() ? 0 : gl_texture_filter;
	bool anisoAvailable = gles.anistropicFilterAvailable &&
	                      (!sysCallbacks.DisableTextureFilter || !sysCallbacks.DisableTextureFilter());
	float aniso =
		filter <= 0 || (sysCallbacks.DisableAnisotropicFiltering && sysCallbacks.DisableAnisotropicFiltering())
			? 1.0f
			: gl_texture_filter_anisotropic;

	glActiveTexture(GL_TEXTURE0 + texunit);
	switch (num)
	{
	case CLAMP_NONE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (lastval >= CLAMP_XY_NOMIP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
			if (anisoAvailable)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		}
		break;

	case CLAMP_X:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (lastval >= CLAMP_XY_NOMIP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
			if (anisoAvailable)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		}
		break;

	case CLAMP_Y:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (lastval >= CLAMP_XY_NOMIP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
			if (anisoAvailable)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		}
		break;

	case CLAMP_XY:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (lastval >= CLAMP_XY_NOMIP)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
			if (anisoAvailable)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		}
		break;

	case CLAMP_XY_NOMIP:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;

	case CLAMP_NOFILTER:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;

	case CLAMP_NOFILTER_X:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;

	case CLAMP_NOFILTER_Y:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;

	case CLAMP_NOFILTER_XY:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;

	case CLAMP_CAMTEX:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
		if (anisoAvailable)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
		break;
	}
	glActiveTexture(GL_TEXTURE0);
	return 255;
}


void FSamplerManager::SetTextureFilterMode()
{
	/*
	GLRenderer->FlushTextures();

	GLint bounds[IHardwareTexture::MAX_TEXTURES];

	// Unbind all
	for(int i = IHardwareTexture::MAX_TEXTURES-1; i >= 0; i--)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glGetIntegerv(GL_SAMPLER_BINDING, &bounds[i]);
		glBindSampler(i, 0);
	}

	int filter = sysCallbacks.DisableTextureFilter && sysCallbacks.DisableTextureFilter() ? 0 : gl_texture_filter;

	for (int i = 0; i < 4; i++)
	{
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MIN_FILTER, TexFilter[filter].minfilter);
		glSamplerParameteri(mSamplers[i], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
		glSamplerParameterf(mSamplers[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, filter > 0? gl_texture_filter_anisotropic : 1.0);
	}
	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_XY_NOMIP], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_CAMTEX], GL_TEXTURE_MIN_FILTER, TexFilter[filter].magfilter);
	glSamplerParameteri(mSamplers[CLAMP_CAMTEX], GL_TEXTURE_MAG_FILTER, TexFilter[filter].magfilter);
	for(int i = 0; i < IHardwareTexture::MAX_TEXTURES; i++)
	{
		glBindSampler(i, bounds[i]);
	}
	*/
}


}
