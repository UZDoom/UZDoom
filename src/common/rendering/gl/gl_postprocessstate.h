/*
** gl_postprocessstate.h
**
** Postprocessing framework
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2016-2020 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#ifndef __GL_POSTPROCESSSTATE_H
#define __GL_POSTPROCESSSTATE_H

#include <string.h>
#include "gl_interface.h"
#include "matrix.h"
#include "c_cvars.h"

namespace OpenGLRenderer
{

class FGLPostProcessState
{
public:
	FGLPostProcessState();
	~FGLPostProcessState();

	void SaveTextureBindings(unsigned int numUnits);

private:
	FGLPostProcessState(const FGLPostProcessState &) = delete;
	FGLPostProcessState &operator=(const FGLPostProcessState &) = delete;

	GLint activeTex;
	TArray<GLint> textureBinding;
	TArray<GLint> samplerBinding;
	GLboolean blendEnabled;
	GLboolean scissorEnabled;
	GLboolean depthEnabled;
	GLboolean multisampleEnabled;
	GLint currentProgram;
	GLint blendEquationRgb;
	GLint blendEquationAlpha;
	GLint blendSrcRgb;
	GLint blendSrcAlpha;
	GLint blendDestRgb;
	GLint blendDestAlpha;
};

}
#endif
