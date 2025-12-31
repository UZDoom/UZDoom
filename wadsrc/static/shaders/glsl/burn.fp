/*
** burn.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
** Copyright 2018-2025 GZDoom Maintainers and Contributors
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

layout(location=0) in vec4 vTexCoord;
layout(location=1) in vec4 vColor;
layout(location=0) out vec4 FragColor;

void main()
{
	vec4 frag = vColor;

	vec4 t1 = texture(tex, vTexCoord.xy);
	vec4 t2 = texture(texture2, vec2(vTexCoord.x, 1.0-vTexCoord.y));

	FragColor = frag * vec4(t1.r, t1.g, t1.b, t2.a);
}
