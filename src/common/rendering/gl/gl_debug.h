/*
** gl_debug.h
**
** OpenGL debugging support functions
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

#ifndef __GL_DEBUG_H
#define __GL_DEBUG_H

#include <string.h>
#include "gl_interface.h"
#include "c_cvars.h"
#include "v_video.h"

namespace OpenGLRenderer
{

class FGLDebug
{
public:
	void Update();

	static void LabelObject(GLenum type, GLuint handle, const char *name);
	static void LabelObjectPtr(void *ptr, const char *name);

	static void PushGroup(const FString &name);
	static void PopGroup();

	static bool HasDebugApi() { return (gl.flags & RFL_DEBUG) != 0; }

private:
	void SetupBreakpointMode();
	void UpdateLoggingLevel();
	void OutputMessageLog();

	static bool IsFilteredByDebugLevel(GLenum severity);
	static void PrintMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message);

	static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

	static FString SourceToString(GLenum source);
	static FString TypeToString(GLenum type);
	static FString SeverityToString(GLenum severity);

	GLenum mCurrentLevel = 0;
	bool mBreakpointMode = false;
};

}
#endif
