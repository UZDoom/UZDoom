/*
** present_checker3d.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christopher Bruns
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

layout(binding=0) uniform sampler2D LeftEyeTexture;
layout(binding=1) uniform sampler2D RightEyeTexture;

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

	val = val * (WhitePoint - BlackPoint) + BlackPoint;
	val = pow(max(val, vec3(0.0)), vec3(InvGamma));

	return vec4(val, c.a);
}

// END `gamma.fp`

void main()
{
	int thisVerticalPixel = int(gl_FragCoord.y); // Bottom row is typically the right eye, when WindowHeight is even
	int thisHorizontalPixel = int(gl_FragCoord.x); // column
	bool isLeftEye = (thisVerticalPixel // because we want to alternate eye view on each row
			+ thisHorizontalPixel // and each column
			+ WindowPositionParity // because the window might not be aligned to the screen
		) % 2 == 0;
	vec4 inputColor;
	if (isLeftEye) {
		inputColor = texture(LeftEyeTexture, UVOffset + TexCoord * UVScale);
	}
	else {
		// inputColor = vec4(0, 1, 0, 1);
		inputColor = texture(RightEyeTexture, UVOffset + TexCoord * UVScale);
	}
	FragColor = ApplyGamma(inputColor);
}
