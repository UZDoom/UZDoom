/*
** ssaocombine.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Magnus Norddahl
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
*/

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

layout(binding=0) uniform sampler2D AODepthTexture;

#if defined(MULTISAMPLE)
layout(binding=1) uniform sampler2DMS SceneFogTexture;
#else
layout(binding=1) uniform sampler2D SceneFogTexture;
#endif

void main()
{
	vec2 uv = Offset + TexCoord * Scale;

#if defined(MULTISAMPLE)
	ivec2 texSize = textureSize(SceneFogTexture);
#else
	ivec2 texSize = textureSize(SceneFogTexture, 0);
#endif
	ivec2 ipos = ivec2(uv * vec2(texSize));

#if defined(MULTISAMPLE)
	vec3 fogColor = texelFetch(SceneFogTexture, ipos, 0).rgb;
#else
	vec3 fogColor = texelFetch(SceneFogTexture, ipos, 0).rgb;
#endif

	vec4 ssao = texture(AODepthTexture, TexCoord);
	float attenutation = ssao.x;

	if (DebugMode == 0)
		FragColor = vec4(fogColor, 1.0 - attenutation);
	else if (DebugMode < 3)
		FragColor = vec4(attenutation, attenutation, attenutation, 1.0);
	else if (DebugMode == 3)
		FragColor = vec4(ssao.yyy / 1000.0, 1.0);
	else
		FragColor = vec4(ssao.xyz, 1.0);
}
