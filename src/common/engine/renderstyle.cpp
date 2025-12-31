/*
** renderstyle.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
**
*/

#include "basics.h"
#include "renderstyle.h"
#include "c_cvars.h"

CVAR (Bool, r_drawtrans, true, 0)
CVAR (Int, r_drawfuzz, 1, CVAR_ARCHIVE)

// Convert legacy render styles to flexible render styles.

FRenderStyle LegacyRenderStyles[STYLE_Count] =
{
	{ { STYLEOP_None, STYLEALPHA_Zero, STYLEALPHA_Zero, 0 } },											/* STYLE_None */  
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_Alpha1 } },								/* STYLE_Normal */
	{ { STYLEOP_Fuzz, STYLEALPHA_Src, STYLEALPHA_InvSrc, 0 } },											/* STYLE_Fuzzy */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_TransSoulsAlpha } },						/* STYLE_SoulTrans */
	{ { STYLEOP_FuzzOrAdd, STYLEALPHA_Src, STYLEALPHA_InvSrc, 0 } },									/* STYLE_OptFuzzy */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_Alpha1 | STYLEF_ColorIsFixed } },		/* STYLE_Stencil */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, 0 } },											/* STYLE_Translucent */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_One, 0 } },												/* STYLE_Add */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_RedIsAlpha | STYLEF_ColorIsFixed } },	/* STYLE_Shaded */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_ColorIsFixed } },						/* STYLE_TranslucentStencil */
	{ { STYLEOP_Shadow, 0, 0, 0 } },																	/* STYLE_Shadow */
	{ { STYLEOP_RevSub, STYLEALPHA_Src, STYLEALPHA_One, 0 } },											/* STYLE_Subtract*/
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_One, STYLEF_ColorIsFixed } },							/* STYLE_AddStencil */
	{ { STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_One, STYLEF_RedIsAlpha | STYLEF_ColorIsFixed } },		/* STYLE_AddShaded */
	{ { STYLEOP_Add, STYLEALPHA_DstCol, STYLEALPHA_Zero, 0 } },											/* STYLE_Multiply */
	{ { STYLEOP_Add, STYLEALPHA_InvDstCol, STYLEALPHA_Zero, 0 } },										/* STYLE_InverseMultiply */
	{ { STYLEOP_Add, STYLEALPHA_SrcCol, STYLEALPHA_InvSrcCol, 0 } },									/* STYLE_ColorBlend */
	{ { STYLEOP_Add, STYLEALPHA_One, STYLEALPHA_Zero, 0 } },											/* STYLE_Source */
	{ { STYLEOP_Add, STYLEALPHA_SrcCol, STYLEALPHA_One, 0 } },											/* STYLE_ColorAdd */
};

double GetAlpha(int type, double alpha)
{
	switch (type)
	{
	case STYLEALPHA_Zero:		return 0;
	case STYLEALPHA_One:		return 1.;
	case STYLEALPHA_Src:		return alpha;
	case STYLEALPHA_InvSrc:		return 1. - alpha;
	default:					return 0.5;	// undeterminable
	}
}

//==========================================================================
//
// FRenderStyle :: IsVisible
//
// Coupled with the given alpha, will this render style produce something
// visible on-screen?
//
//==========================================================================

bool FRenderStyle::IsVisible(double alpha) const throw()
{
	if (BlendOp == STYLEOP_None)
	{
		return false;
	}
	if (BlendOp == STYLEOP_Add || BlendOp == STYLEOP_RevSub)
	{
		if (Flags & STYLEF_Alpha1)
		{
			alpha = 1.;
		}
		else
		{
			alpha = clamp(alpha, 0., 1.);
		}
		return GetAlpha(SrcAlpha, alpha) != 0 || GetAlpha(DestAlpha, alpha) != 1;
	}
	// Treat anything else as visible.
	return true;
}


//==========================================================================
//
// FRenderStyle :: CheckFuzz
//
// Adjusts settings based on r_drawfuzz CVAR
//
//==========================================================================

void FRenderStyle::CheckFuzz()
{
	switch (BlendOp)
	{
	default:
		return;

	case STYLEOP_FuzzOrAdd:
		if (r_drawtrans && r_drawfuzz == 0)
		{
			BlendOp = STYLEOP_Add;
			return;
		}
		break;

	case STYLEOP_FuzzOrSub:
		if (r_drawtrans && r_drawfuzz == 0)
		{
			BlendOp = STYLEOP_Sub;
			return;
		}
		break;

	case STYLEOP_FuzzOrRevSub:
		if (r_drawtrans && r_drawfuzz == 0)
		{
			BlendOp = STYLEOP_RevSub;
			return;
		}
		break;
	}

	if (r_drawfuzz == 2)
	{
		BlendOp = STYLEOP_Shadow;
	}
	else
	{
		BlendOp = STYLEOP_Fuzz;
	}
}
