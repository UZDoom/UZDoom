/*
** fuzz_swirly.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013 Evil Space Tomato
** Copyright 2013-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

vec4 ProcessTexel()
{
	vec2 texCoord = vTexCoord.st;
	vec4 basicColor = getTexel(texCoord);

	float texX = sin(texCoord.x * 100.0 + timer*5.0);
	float texY = cos(texCoord.x * 100.0 + timer*5.0);
	float vX = (texX/texY)*21.0;
	float vY = (texY/texX)*13.0;
	float test = mod(timer*2.0+(vX + vY), 0.5);

	basicColor.a = basicColor.a * test;
	basicColor.r = basicColor.g = basicColor.b = 0.0;
	
	return basicColor;
}
