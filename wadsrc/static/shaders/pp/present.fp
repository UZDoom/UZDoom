/*
** present.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// #include "shaders/pp/gamma.fp"

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

layout(binding=0) uniform sampler2D InputTexture;
layout(binding=1) uniform sampler2D DitherTexture;

// START `gamma.fp`

const vec3 rec709Weights = vec3(0.2126, 0.7152, 0.0722);
const vec3 averageWeights = vec3(1.0 / 3.0);
const vec3 oldWeights = vec3(0.3, 0.56, 0.14);

vec4 ApplyGamma(vec4 c)
{
	c.rgb = min(c.rgb, vec3(2.0)); // for HDR mode - prevents stacked translucent sprites (such as plasma) producing way too bright light

	vec3 val = pow(c.rgb, vec3(2.2));

	vec3 weights = (GrayFormula == 2) ? rec709Weights
	             : (GrayFormula == 1) ? oldWeights
	                                  : averageWeights;
	float lum = dot(val, weights);
	val = mix(vec3(lum), val, Saturation);

	val = val * Contrast - (Contrast - 1.0) * 0.5;

	val = (val + Brightness * 0.5) * (WhitePoint - BlackPoint) + BlackPoint;
	val = pow(max(val, vec3(0.0)), vec3(InvGamma));

	return vec4(val, c.a);
}

// END `gamma.fp`

vec4 Dither(vec4 c)
{
	if (ColorScale == 0.0)
		return c;

	vec2 texSize = vec2(textureSize(DitherTexture, 0));
	float threshold = texture(DitherTexture, gl_FragCoord.xy / texSize).r;

	return vec4(floor(c.rgb * ColorScale + threshold) / ColorScale, c.a);
}

vec3 sRGBtoscRGBLinear(vec3 c)
{
	return pow(c, vec3(2.2)) * 1.1;
}

vec4 ApplyHdrMode(vec4 c)
{
	if (HdrMode == 0)
		return c;

	return vec4(sRGBtoscRGBLinear(c.rgb), c.a);
}

void main()
{
	vec4 color;
	color = texture(InputTexture, UVOffset + TexCoord * UVScale);
	color = ApplyGamma(color);
	color = ApplyHdrMode(color);
	color = Dither(color);
	FragColor = color;
}

