/*
** burn.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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

varying vec4 vTexCoord;
varying vec4 vColor;

void main()
{
	vec4 frag = vColor;

	vec4 t1 = texture2D(tex, vTexCoord.xy);
	vec4 t2 = texture2D(texture2, vec2(vTexCoord.x, 1.0-vTexCoord.y));
	
	gl_FragColor = frag * vec4(t1.r, t1.g, t1.b, t2.a);
}
