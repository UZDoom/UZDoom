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
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

