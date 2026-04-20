/*
** fogboundary.fp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2016 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

varying vec4 pixelpos;

//===========================================================================
//
// Main shader routine
//
//===========================================================================

void main() 
{
	float fogdist;
	float fogfactor;
	
	//
	// calculate fog factor
	//
#if (DEF_FOG_ENABLED == 1) && (DEF_FOG_RADIAL == 0) && (DEF_FOG_COLOURED == 1) // This was uFogEnabled = -1,, TODO check this
	{
		fogdist = pixelpos.w;
	}
#else
	{
		fogdist = max(16.0, distance(pixelpos.xyz, uCameraPos.xyz));
	}
#endif
	fogfactor = exp2 (uFogDensity * fogdist);
	
	gl_FragColor = vec4(uFogColor.rgb, 1.0 - fogfactor);
}

