/*
** gl_debug.h
**
** OpenGL debugging support functions
**
**---------------------------------------------------------------------------
**
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
** Copyright 2016-2020 Magnus Norddahl
**
** This software is provided 'as-is', without any express or implied
** warranty.  In no event will the authors be held liable for any damages
** arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose,
** including commercial applications, and to alter it and redistribute it
** freely, subject to the following restrictions:
**
** 1. The origin of this software must not be misrepresented; you must not
**    claim that you wrote the original software. If you use this software
**    in a product, an acknowledgment in the product documentation would be
**    appreciated but is not required.
** 2. Altered source versions must be plainly marked as such, and must not be
**    misrepresented as being the original software.
** 3. This notice may not be removed or altered from any source distribution.
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
