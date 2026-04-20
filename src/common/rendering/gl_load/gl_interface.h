/*
** gl_interface.h
**
** OpenGL system interface
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2019 Christoph Oelckers
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

#ifndef R_RENDER
#define R_RENDER

#include "m_argv.h"
struct RenderContext
{
	unsigned int flags;
	unsigned int maxuniforms;
	unsigned int maxuniformblock;
	unsigned int uniformblockalignment;
	float glslversion;
	int max_texturesize;
	char * vendorstring;
	char * modelstring;
};

extern RenderContext gl;

EXTERN_FARG(glversion);

#endif

