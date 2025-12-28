/*
** func_wavex.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Christoph Oelckers
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

vec2 GetTexCoord()
{
	vec2 texCoord = vTexCoord.st;

	const float pi = 3.14159265358979323846;

	texCoord.x += sin(pi * 2.0 * (texCoord.y + timer * 0.125)) * 0.1;

	return texCoord;
}

vec4 ProcessTexel()
{
	return getTexel(GetTexCoord());
}

