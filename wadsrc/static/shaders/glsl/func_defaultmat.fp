/*
** func_defaultmat.fp
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

#ifdef OLD_PROCESSLIGHT
vec4 ProcessLight(vec4 color);

vec4 ProcessLight(Material material, vec4 color)
{
	return ProcessLight(color);
}
#endif

void SetupMaterial(inout Material material)
{
	#ifdef NO_PROCESS_TEXEL
		material.Base = Process(vec4(1.0));
	#else
		material.Base = ProcessTexel();
	#endif
	material.Normal = ApplyNormalMap(vTexCoord.st);
	material.Bright = texture(brighttexture, vTexCoord.st);
}
