/*
** exposurecombine.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Magnus Norddahl
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

layout(binding=0) uniform sampler2D ExposureTexture;

void main()
{
	float light = texture(ExposureTexture, TexCoord).x;
	float exposureAdjustment = 1.0 / max(ExposureBase + light * ExposureScale, ExposureMin);
	FragColor = vec4(exposureAdjustment, 0.0, 0.0, ExposureSpeed);
}
