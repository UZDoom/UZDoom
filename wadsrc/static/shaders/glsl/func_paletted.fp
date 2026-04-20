/*
** func_paletted.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

vec4 ProcessTexel()
{
	float index = getTexel(vTexCoord.st).r;
 	index = ((index * 255.0) + 0.5) / 256.0;
	vec4 tex = texture(texture2, vec2(index, 0.5));
	tex.a = 1.0;
	return tex;
}

