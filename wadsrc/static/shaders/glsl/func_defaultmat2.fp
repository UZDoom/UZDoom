/*
** func_defaultmat2.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2020-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifdef OLD_PROCESSLIGHT
vec4 ProcessLight(vec4 color);

vec4 ProcessLight(Material material, vec4 color)
{
	return ProcessLight(color);
}
#endif

void SetupMaterial(inout Material material)
{
	vec2 texCoord = GetTexCoord();
	SetMaterialProps(material, texCoord);
}
