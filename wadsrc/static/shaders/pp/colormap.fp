/*
** colormap.fp
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
	vec4 frag = texture(SceneTexture, TexCoord);
	frag.rgb = clamp(pow(frag.rgb, vec3(uFixedColormapStart.a)), 0.0, 1.0);
	if (uFixedColormapRange.a == 0)
	{
		float gray = (frag.r * 0.3 + frag.g * 0.56 + frag.b * 0.14);	
		vec4 cm = uFixedColormapStart + gray * uFixedColormapRange;
		frag.rgb = clamp(cm.rgb, 0.0, 1.0);
	}
	FragColor = frag;
}

