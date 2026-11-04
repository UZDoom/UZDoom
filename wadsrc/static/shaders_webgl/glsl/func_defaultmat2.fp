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

void SetupMaterial(inout Material material)
{
	vec2 texCoord = GetTexCoord();
	SetMaterialProps(material, texCoord);
}
