/*
** fuzz_jagged.fp
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

vec4 ProcessTexel()
{
	vec2 texCoord = vTexCoord.st;

	vec2 texSplat;
	const float pi = 3.14159265358979323846;
	texSplat.x = texCoord.x + mod(sin(pi * 2.0 * (texCoord.y + timer * 2.0)),0.1) * 0.1;
	texSplat.y = texCoord.y + mod(cos(pi * 2.0 * (texCoord.x + timer * 2.0)),0.1) * 0.1;

	vec4 basicColor = getTexel(texSplat);

	float texX = sin(texCoord.x * 100.0 + timer*5.0);
	float texY = cos(texCoord.x * 100.0 + timer*5.0);
	float vX = (texX/texY)*21.0;
	float vY = (texY/texX)*13.0;

	float test = mod(timer*2.0+(vX + vY), 0.5);

	basicColor.a = basicColor.a * test;

	return basicColor;
}
