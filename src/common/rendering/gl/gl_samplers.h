/*
** gl_samplers.h
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

#ifndef __GL_SAMPLERS_H
#define __GL_SAMPLERS_H

#include "gl_hwtexture.h"
#include "textures.h"

namespace OpenGLRenderer
{


class FSamplerManager
{
	unsigned int mSamplers[NUMSAMPLERS];

	void UnbindAll();

public:

	FSamplerManager();
	~FSamplerManager();

	uint8_t Bind(int texunit, int num, int lastval);
	void SetTextureFilterMode();


};

}
#endif

