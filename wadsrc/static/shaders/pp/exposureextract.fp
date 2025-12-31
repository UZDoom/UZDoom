/*
** exposureextract.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2014-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

layout(binding=0) uniform sampler2D SceneTexture;

void main()
{
	vec4 color = texture(SceneTexture, Offset + TexCoord * Scale);
	FragColor = vec4(max(max(color.r, color.g), color.b), 0.0, 0.0, 1.0);
}
