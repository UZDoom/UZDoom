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
** SPDX-License-Identifier: GPL-3.0-or-later
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
