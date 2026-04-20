/*
** ppshader.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
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

struct PPShader native
{
	native clearscope static void SetEnabled(string shaderName, bool enable);
	native clearscope static void SetUniform1f(string shaderName, string uniformName, float value);
	native clearscope static void SetUniform2f(string shaderName, string uniformName, vector2 value);
	native clearscope static void SetUniform3f(string shaderName, string uniformName, vector3 value);
	native clearscope static void SetUniform4f(string shaderName, string uniformName, vector4 value);
	native clearscope static void SetUniform1i(string shaderName, string uniformName, int value);
}
