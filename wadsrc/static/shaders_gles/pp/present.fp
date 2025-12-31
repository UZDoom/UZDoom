/*
** present.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2021-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

varying vec2 TexCoord;

uniform sampler2D InputTexture;
uniform sampler2D DitherTexture;

vec4 ApplyGamma(vec4 c)
{
	vec3 valgray, val;

	valgray = vec3(dot(c.rgb, vec3(0.3,0.56,0.14)));
	valgray = mix(valgray, c.rgb, Saturation);
	valgray = pow(valgray, vec3(2.2));

	val = valgray * Contrast - (Contrast - 1.0) * 0.5;

	val = (val + Brightness * 0.5) * (WhitePoint - BlackPoint) + BlackPoint;
	val = pow(max(val, vec3(0.0)), vec3(InvGamma));

	return vec4(val, c.a);
}

void main()
{
	gl_FragColor =  ApplyGamma(texture2D(InputTexture, UVOffset + TexCoord * UVScale));
}
