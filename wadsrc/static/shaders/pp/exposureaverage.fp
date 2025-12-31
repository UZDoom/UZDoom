/*
** exposureaverage.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
layout(binding=0) uniform sampler2D ExposureTexture;

void main()
{
#if __VERSION__ < 400
	ivec2 size = textureSize(ExposureTexture, 0);
	ivec2 tl = max(ivec2(TexCoord * vec2(size) - 0.5), ivec2(0));
	ivec2 br = min(tl + ivec2(1), size - ivec2(1));
	vec4 values = vec4(
		texelFetch(ExposureTexture, tl, 0).x,
		texelFetch(ExposureTexture, ivec2(tl.x, br.y), 0).x,
		texelFetch(ExposureTexture, ivec2(br.x, tl.y), 0).x,
		texelFetch(ExposureTexture, br, 0).x);
#else
	vec4 values = textureGather(ExposureTexture, TexCoord);
#endif

	FragColor = vec4((values.x + values.y + values.z + values.w) * 0.25, 0.0, 0.0, 1.0);
}
