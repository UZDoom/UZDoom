/*
** depthblur.fp
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

#define KERNEL_RADIUS 3.0

void AddSample(vec2 blurSample, float r, float centerDepth, inout float totalAO, inout float totalW)
{
	const float blurSigma = KERNEL_RADIUS * 0.5;
	const float blurFalloff = 1.0 / (2.0 * blurSigma * blurSigma);

	float ao = blurSample.x;
	float z = blurSample.y;

	float deltaZ = (z - centerDepth) * BlurSharpness;
	float w = exp2(-r * r * blurFalloff - deltaZ * deltaZ);

	totalAO += w * ao;
	totalW += w;
}

void main()
{
	vec2 centerSample = textureOffset(AODepthTexture, TexCoord, ivec2( 0, 0)).xy;
	float centerDepth = centerSample.y;
	float totalAO = centerSample.x;
	float totalW = 1.0;

#if defined(BLUR_HORIZONTAL)
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(-3, 0)).xy, 3.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(-2, 0)).xy, 2.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(-1, 0)).xy, 1.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2( 1, 0)).xy, 1.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2( 2, 0)).xy, 2.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2( 3, 0)).xy, 3.0, centerDepth, totalAO, totalW);
#else
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0, -3)).xy, 3.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0, -2)).xy, 2.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0, -1)).xy, 1.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0,  1)).xy, 1.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0,  2)).xy, 2.0, centerDepth, totalAO, totalW);
	AddSample(textureOffset(AODepthTexture, TexCoord, ivec2(0,  3)).xy, 3.0, centerDepth, totalAO, totalW);
#endif

	float fragAO = totalAO / totalW;

#if defined(BLUR_HORIZONTAL)
	FragColor = vec4(fragAO, centerDepth, 0.0, 1.0);
#else
	FragColor = vec4(pow(clamp(fragAO, 0.0, 1.0), PowExponent), 0.0, 0.0, 1.0);
#endif
}
