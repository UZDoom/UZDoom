/*
** blur.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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

layout(binding=0) uniform sampler2D SourceTexture;

void main()
{
#if defined(BLUR_HORIZONTAL)
	FragColor =
		textureOffset(SourceTexture, TexCoord, ivec2( 0, 0)) * SampleWeights0 +
		textureOffset(SourceTexture, TexCoord, ivec2( 1, 0)) * SampleWeights1 +
		textureOffset(SourceTexture, TexCoord, ivec2(-1, 0)) * SampleWeights2 +
		textureOffset(SourceTexture, TexCoord, ivec2( 2, 0)) * SampleWeights3 +
		textureOffset(SourceTexture, TexCoord, ivec2(-2, 0)) * SampleWeights4 +
		textureOffset(SourceTexture, TexCoord, ivec2( 3, 0)) * SampleWeights5 +
		textureOffset(SourceTexture, TexCoord, ivec2(-3, 0)) * SampleWeights6;
#else
	FragColor =
		textureOffset(SourceTexture, TexCoord, ivec2(0, 0)) * SampleWeights0 +
		textureOffset(SourceTexture, TexCoord, ivec2(0, 1)) * SampleWeights1 +
		textureOffset(SourceTexture, TexCoord, ivec2(0,-1)) * SampleWeights2 +
		textureOffset(SourceTexture, TexCoord, ivec2(0, 2)) * SampleWeights3 +
		textureOffset(SourceTexture, TexCoord, ivec2(0,-2)) * SampleWeights4 +
		textureOffset(SourceTexture, TexCoord, ivec2(0, 3)) * SampleWeights5 +
		textureOffset(SourceTexture, TexCoord, ivec2(0,-3)) * SampleWeights6;
#endif
}
