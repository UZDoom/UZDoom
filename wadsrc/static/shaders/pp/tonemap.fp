/*
** tonemap.fp
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

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

layout(binding=0) uniform sampler2D InputTexture;

vec3 Linear(vec3 c)
{
	c = max(c, vec3(0.0));
	return pow(c, vec3(2.2));
}

vec3 sRGB(vec3 c)
{
	c = max(c, vec3(0.0));
	return pow(c, vec3(1.0 / 2.2));
}

#if defined(LINEAR)

vec3 Tonemap(vec3 color)
{
	return sRGB(color);
}

#elif defined(REINHARD)

vec3 Tonemap(vec3 color)
{
	color = color / (1 + color);
	return sRGB(color);
}

#elif defined(HEJLDAWSON)

vec3 Tonemap(vec3 color)
{
	vec3 x = max(vec3(0), color - 0.004);
	return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06); // no sRGB needed
}

#elif defined(UNCHARTED2)

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Tonemap(vec3 color)
{
	float W = 11.2;
	vec3 curr = Uncharted2Tonemap(color);
	vec3 whiteScale = vec3(1) / Uncharted2Tonemap(vec3(W));
	return sRGB(curr * whiteScale);
}

#elif defined(ACES)

vec3 Tonemap(vec3 color)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	vec3 mapped = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
	return sRGB(mapped);
}

#elif defined(AGX)

vec3 AgXDefaultContrastApprox(vec3 x)
{
	vec3 x2 = x * x;
	vec3 x4 = x2 * x2;
	return + 15.5     * x4 * x2
	       - 40.14    * x4 * x
	       + 31.96    * x4
	       - 6.868    * x2 * x
	       + 0.4298   * x2
	       + 0.1191   * x
	       - 0.00232;
}

vec3 Tonemap(vec3 color)
{
	const mat3 agx_mat = mat3(
		0.842479062253094,  0.0423282422610123, 0.0423756549057051,
		0.0784335999999992, 0.878468636469772,  0.0784336,
		0.0792237451477643, 0.0791661274605434, 0.879142973793104
	);
	const mat3 agx_mat_inv = mat3(
		 1.19687900512017,   -0.0528968517574562, -0.0529716355144438,
		-0.0980208811401368,  1.15190312990417,   -0.0980434501171241,
		-0.0990297440797205, -0.0989611768448433,  1.15107367264116
	);
	const float min_ev = -12.47393;
	const float max_ev = 4.026069;

	vec3 val = agx_mat * max(color, vec3(1e-6));
	val = clamp(log2(val), min_ev, max_ev);
	val = (val - min_ev) / (max_ev - min_ev);
	val = AgXDefaultContrastApprox(val);
	val = agx_mat_inv * val;
	return clamp(val, 0.0, 1.0);
}

#elif defined(PALETTE)

layout(binding=1) uniform sampler2D PaletteLUT;

vec3 Tonemap(vec3 color)
{
	ivec3 c = ivec3(clamp(color.rgb, vec3(0.0), vec3(1.0)) * 63.0 + 0.5);
	int index = (c.r * 64 + c.g) * 64 + c.b;
	int tx = index % 512;
	int ty = index / 512;
	return texelFetch(PaletteLUT, ivec2(tx, ty), 0).rgb;
}

#else
#error Tonemap mode define is missing
#endif

void main()
{
	vec3 color = texture(InputTexture, TexCoord).rgb;
#ifndef PALETTE
	color = Linear(color); // needed because gzdoom's scene texture is not linear at the moment
#endif
	FragColor = vec4(Tonemap(color), 1.0);
}
