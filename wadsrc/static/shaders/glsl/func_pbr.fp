/*
** func_pbr.fp
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

void SetupMaterial(inout Material material)
{
	SetMaterialProps(material, vTexCoord.st);
	material.Metallic = texture(metallictexture, vTexCoord.st).r;
	material.Roughness = texture(roughnesstexture, vTexCoord.st).r;
	material.AO = texture(aotexture, vTexCoord.st).r;
}
