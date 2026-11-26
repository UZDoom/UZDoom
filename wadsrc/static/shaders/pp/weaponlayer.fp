/*
** weaponlayer.fp
**
** Passthrough shader for compositing the weapon layer onto the scene
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
*/

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

layout(binding=0) uniform sampler2D SceneTexture;
layout(binding=1) uniform sampler2D WeaponTexture;

void main()
{
	vec4 scene = texture(SceneTexture, TexCoord);
	vec4 weapon = texture(WeaponTexture, TexCoord);
	FragColor = vec4(mix(scene.rgb, weapon.rgb, weapon.a), scene.a);
}
