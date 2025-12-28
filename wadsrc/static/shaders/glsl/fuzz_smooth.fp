/*
** fuzz_smooth.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013 Evil Space Tomato
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

vec4 ProcessTexel()
{
	vec2 texCoord = vTexCoord.st;
	vec4 basicColor = getTexel(texCoord);

	float texX = texCoord.x / 3.0 + 0.66;
	float texY = 0.34 - texCoord.y / 3.0;
	float vX = (texX/texY)*21.0;
	float vY = (texY/texX)*13.0;
	float test = mod(timer*2.0+(vX + vY), 0.5);

	basicColor.a = basicColor.a * test;
	basicColor.r = basicColor.g = basicColor.b = 0.0;

	return basicColor;
}
