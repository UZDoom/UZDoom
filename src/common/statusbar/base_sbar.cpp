/*
** base_sbar.cpp
**
** Base status bar implementation
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2017-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <cassert>

#include "base_sbar.h"
#include "c_cvars.h"
#include "colorspace.h"
#include "cmdlib.h"
#include "i_interface.h"
#include "printf.h"
#include "r_videoscale.h"
#include "texturemanager.h"
#include "utf8.h"
#include "v_draw.h"
#include "v_font.h"
#include "v_text.h"
#include "vm.h"

FGameTexture* CrosshairImage;
static int CrosshairNum;

IMPLEMENT_CLASS(DStatusBarCore, false, false)
IMPLEMENT_CLASS(DHUDFont, false, false);

CVAR(Color, crosshaircolor,     0xff0000, CVAR_ARCHIVE);
CVAR(Color, crosshaircolorFull, 0x00ff00, CVAR_ARCHIVE);
CVAR(Color, crosshaircolorMax,  0x7f7fff, CVAR_ARCHIVE);
CVAR(Bool, crosshairshowshealth, false, CVAR_HIDDEN);
CVAR(Bool, crosshairhascolor, false, CVAR_HIDDEN);
CUSTOM_CVARD(Int, crosshaircolors, 2, CVAR_ARCHIVE, "0: basic, 1: show health, 2: show health bonus, 3: inverted")
{
	switch (self)
	{
		case 0: crosshairshowshealth = false; crosshairhascolor = true;  break;
		case 1:
		case 2: crosshairshowshealth = true;  crosshairhascolor = true;  break;
		case 3: crosshairshowshealth = false; crosshairhascolor = false; break;
		default: self = 2;
	}
};
CVARD(Float, crosshairscale, 1.0, CVAR_ARCHIVE, "changes the size of the crosshair");
CVARD(Bool, crosshairgrow, false, CVAR_ARCHIVE, "grow crosshair upon pickup");

CUSTOM_CVARD(Float, hud_scalefactor, 1.f, CVAR_ARCHIVE, "changes the hud scale")
{
	if (self < 0.36f) self = 0.36f;
	else if (self > 1) self = 1;
	else if (sysCallbacks.HudScaleChanged) sysCallbacks.HudScaleChanged();
}

CUSTOM_CVARD(Bool, hud_aspectscale, true, CVAR_ARCHIVE, "enables aspect ratio correction for the status bar")
{
	if (sysCallbacks.HudScaleChanged) sysCallbacks.HudScaleChanged();
}

void ST_LoadCrosshair(int num, bool alwaysload)
{
	char name[16];
	char size;

	if (!alwaysload && CrosshairNum == num && CrosshairImage != NULL)
	{ // No change.
		return;
	}

	if (num == 0)
	{
		CrosshairNum = 0;
		CrosshairImage = NULL;
		return;
	}
	if (num < 0)
	{
		num = -num;
	}
	size = (twod->GetWidth() < 640) ? 'S' : 'B';

	mysnprintf(name, countof(name), "XHAIR%c%d", size, num);
	FTextureID texid = TexMan.CheckForTexture(name, ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
	if (!texid.isValid())
	{
		mysnprintf(name, countof(name), "XHAIR%c1", size);
		texid = TexMan.CheckForTexture(name, ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
		if (!texid.isValid())
		{
			texid = TexMan.CheckForTexture("XHAIRS1", ETextureType::MiscPatch, FTextureManager::TEXMAN_TryAny | FTextureManager::TEXMAN_ShortNameOnly);
		}
	}
	CrosshairNum = num;
	CrosshairImage = TexMan.GetGameTexture(texid);
}

void ST_UnloadCrosshair()
{
	CrosshairImage = NULL;
	CrosshairNum = 0;
}

//---------------------------------------------------------------------------
//
// DrawCrosshair
//
//---------------------------------------------------------------------------

void ST_DrawCrosshair(int phealth, double xpos, double ypos, double scale, DAngle angle)
{
	FRenderStyle style {{ STYLEOP_Add, STYLEALPHA_Src, STYLEALPHA_InvSrc, STYLEF_RedIsAlpha | STYLEF_ColorIsFixed }};
	uint32_t color;
	double size;
	int w, h;

	// Don't draw the crosshair if there is none
	if (CrosshairImage == NULL)
	{
		return;
	}

	int mult = max(twod->Height/720, 1); // 1080p: 1, 1440p: 2, 4K: 3

	size = ((crosshairscale > 0.0f)? crosshairscale: 1.0) * mult;

	if (crosshairgrow) size *= scale;

	w = round(CrosshairImage->GetDisplayWidth() * size);
	h = round(CrosshairImage->GetDisplayHeight() * size);

	if (crosshaircolors == 0)
	{
		color = crosshaircolor;
	}
	else if (crosshaircolors == 3)
	{
		style = {{ STYLEOP_Add, STYLEALPHA_InvDstCol, STYLEALPHA_InvSrcCol, STYLEF_RedIsAlpha }};
		color = 0xffffff;
	}
	else
	{
		static int lastHealth = -1;
		static int lastColor = -1;
		int health = clamp(phealth, 0, (crosshaircolors == 1)? 100: 200);

		if (health != lastHealth)
		{
			int hi = crosshaircolorFull, lo = crosshaircolor;
			float mix = 1.0;

			if (health > 100)
			{
				lo = hi;
				hi = crosshaircolorMax;
				mix = (health-100)/100.0;
			}
			else if (health <= 85)
			{
				mix = health/85.0;
			}

			auto a = Color::rgb((lo>>16&0xff)/255., (lo>>8&0xff)/255., (lo&0xff)/255.);
			auto b = Color::rgb((hi>>16&0xff)/255., (hi>>8&0xff)/255., (hi&0xff)/255.);
			auto c = Color::mix(a, b, mix);

			lastHealth = health;
			lastColor = (int)(c.rgb.r*255)<<16 | (int)(c.rgb.g*255)<<8 | (int)(c.rgb.b*255);
		}

		color = lastColor;
	}

	DrawTexture(twod, CrosshairImage,
		xpos, ypos,
		DTA_DestWidth, w,
		DTA_DestHeight, h,
		DTA_Rotate, angle.Degrees(),
		DTA_AlphaChannel, true,
		DTA_RenderStyle, style,
		DTA_FillColor, color & 0xFFFFFF,
		TAG_DONE);
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

enum ENumFlags
{
	FNF_WHENNOTZERO = 0x1,
	FNF_FILLZEROS = 0x2,
};

void FormatNumber(int number, int minsize, int maxsize, int flags, const FString& prefix, FString* result)
{
	static int maxvals[] = { 1, 9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999 };

	if (number == 0 && (flags & FNF_WHENNOTZERO))
	{
		*result = "";
		return;
	}
	if (maxsize > 0 && maxsize < 10)
	{
		number = clamp(number, -maxvals[maxsize - 1], maxvals[maxsize]);
	}
	FString& fmt = *result;
	if (minsize <= 1) fmt.Format("%s%d", prefix.GetChars(), number);
	else if (flags & FNF_FILLZEROS) fmt.Format("%s%0*d", prefix.GetChars(), minsize, number);
	else fmt.Format("%s%*d", prefix.GetChars(), minsize, number);
}

void DStatusBarCore::ValidateResolution(int& hres, int& vres) const
{
	if (hres == 0)
	{
		static const int HORIZONTAL_RESOLUTION_DEFAULT = 320;
		hres = HORIZONTAL_RESOLUTION_DEFAULT;
	}

	if (vres == 0)
	{
		static const int VERTICAL_RESOLUTION_DEFAULT = 200;
		vres = VERTICAL_RESOLUTION_DEFAULT;
	}
}


//============================================================================
//
//
//
//============================================================================

void DStatusBarCore::SetSize(int reltop, int hres, int vres, int hhres, int hvres)
{
	ValidateResolution(hres, vres);

	BaseRelTop = reltop;
	BaseSBarHorizontalResolution = hres;
	BaseSBarVerticalResolution = vres;
	BaseHUDHorizontalResolution = hhres < 0 ? hres : hhres;
	BaseHUDVerticalResolution = hvres < 0 ? vres : hvres;
	SetDrawSize(reltop, hres, vres);
}

//============================================================================
//
// calculates a clean scale for the status bar
//
//============================================================================

static void ST_CalcCleanFacs(int designwidth, int designheight, int realwidth, int realheight, int* cleanx, int* cleany)
{
	float ratio;
	int cwidth;
	int cheight;
	int cx1, cy1, cx2, cy2;

	ratio = ActiveRatio(realwidth, realheight);
	if (AspectTallerThanWide(ratio))
	{
		cwidth = realwidth;
		cheight = realheight * AspectMultiplier(ratio) / 48;
	}
	else
	{
		cwidth = realwidth * AspectMultiplier(ratio) / 48;
		cheight = realheight;
	}
	// Use whichever pair of cwidth/cheight or width/height that produces less difference
	// between CleanXfac and CleanYfac.
	cx1 = max(cwidth / designwidth, 1);
	cy1 = max(cheight / designheight, 1);
	cx2 = max(realwidth / designwidth, 1);
	cy2 = max(realheight / designheight, 1);
	if (abs(cx1 - cy1) <= abs(cx2 - cy2) || max(cx1, cx2) >= 4)
	{ // e.g. 640x360 looks better with this.
		*cleanx = cx1;
		*cleany = cy1;
	}
	else
	{ // e.g. 720x480 looks better with this.
		*cleanx = cx2;
		*cleany = cy2;
	}

	if (*cleanx < *cleany)
		*cleany = *cleanx;
	else
		*cleanx = *cleany;
}

//============================================================================
//
//
//
//============================================================================

void DStatusBarCore::SetDrawSize(int reltop, int hres, int vres)
{
	ValidateResolution(hres, vres);

	RelTop = reltop;
	HorizontalResolution = hres;
	VerticalResolution = vres;

	int x, y;
	ST_CalcCleanFacs(hres, vres, twod->GetWidth(), twod->GetHeight(), &x, &y);
	defaultScale = { (double)x, (double)y };

	SetScale();	// recalculate positioning info.
}

//---------------------------------------------------------------------------
//
// PROC SetScaled
//
//---------------------------------------------------------------------------

void DStatusBarCore::SetScale()
{
	ValidateResolution(HorizontalResolution, VerticalResolution);

	int w = twod->GetWidth();
	int h = twod->GetHeight();
	double refw, refh;

	int horz = HorizontalResolution;
	int vert = VerticalResolution;
	double refaspect = horz / double(vert);
	double screenaspect = w / double(h);
	double aspectscale = 1.0;

	const double ViewportAspect = 1. / ViewportPixelAspect();

	if ((horz == 320 && vert == 200) || (horz == 640 && vert == 400))
	{
		refaspect = (4. / 3.);
		if (!hud_aspectscale) aspectscale = 1 / 1.2;
	}

	if (screenaspect < refaspect)
	{
		refw = w;
		refh = w / refaspect;
	}
	else
	{
		refh = h;
		refw = h * refaspect;
	}
	refw *= hud_scalefactor;
	refh *= hud_scalefactor * aspectscale * ViewportAspect;

	int sby = vert - int(RelTop * hud_scalefactor * aspectscale * ViewportAspect);
	// Use full pixels for destination size.

	ST_X = xs_CRoundToInt((w - refw) / 2);
	ST_Y = xs_CRoundToInt(h - refh);
	SBarTop = Scale(sby, h, vert);
	SBarScale.X = refw / horz;
	SBarScale.Y = refh / vert;
}

//---------------------------------------------------------------------------
//
// PROC GetHUDScale
//
//---------------------------------------------------------------------------

DVector2 DStatusBarCore::GetHUDScale() const
{
	return SBarScale;
}

//---------------------------------------------------------------------------
//
//  
//
//---------------------------------------------------------------------------

void DStatusBarCore::BeginStatusBar(int resW, int resH, int relTop, bool forceScaled)
{
	SetDrawSize(relTop < 0 ? BaseRelTop : relTop, resW < 0 ? BaseSBarHorizontalResolution : resW, resH < 0 ? BaseSBarVerticalResolution : resH);
	ForcedScale = forceScaled;
	fullscreenOffsets = false;
}

//---------------------------------------------------------------------------
//
//  
//
//---------------------------------------------------------------------------

void DStatusBarCore::BeginHUD(int resW, int resH, double Alpha, bool forcescaled)
{
	SetDrawSize(RelTop, resW < 0 ? BaseHUDHorizontalResolution : resW, resH < 0 ? BaseHUDVerticalResolution : resH);
	this->Alpha = Alpha;
	ForcedScale = forcescaled;
	CompleteBorder = false;
	fullscreenOffsets = true;
}

//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::StatusbarToRealCoords(double& x, double& y, double& w, double& h) const
{
	if (SBarScale.X == -1 || ForcedScale)
	{
		int hres = HorizontalResolution;
		int vres = VerticalResolution;
		ValidateResolution(hres, vres);

		VirtualToRealCoords(twod, x, y, w, h, hres, vres, true, true);
	}
	else
	{
		x = ST_X + x * SBarScale.X;
		y = ST_Y + y * SBarScale.Y;
		w *= SBarScale.X;
		h *= SBarScale.Y;
	}
}

//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::DrawGraphic(FTextureID texture, double x, double y, int flags, double Alpha, double boxwidth, double boxheight, double scaleX, double scaleY, ERenderStyle style, PalEntry color, int translation, double clipwidth)
{
	if (!texture.isValid())
		return;

	FGameTexture* tex = TexMan.GetGameTexture(texture, !(flags & DI_DONTANIMATE));
	DrawGraphic(tex, x, y, flags, Alpha, boxwidth, boxheight, scaleX, scaleY, style, color, translation, clipwidth);
}

void DStatusBarCore::DrawGraphic(FGameTexture* tex, double x, double y, int flags, double Alpha, double boxwidth, double boxheight, double scaleX, double scaleY, ERenderStyle style, PalEntry color, int translation, double clipwidth)
{
	double texwidth = tex->GetDisplayWidth() * scaleX;
	double texheight = tex->GetDisplayHeight() * scaleY;
	double texleftoffs = tex->GetDisplayLeftOffset() * scaleY;
	double textopoffs = tex->GetDisplayTopOffset() * scaleY;
	double boxleftoffs = 0, boxtopoffs = 0;

	if (boxwidth > 0 || boxheight > 0)
	{
		if (!(flags & DI_FORCEFILL))
		{
			double scale1 = 1., scale2 = 1.;

			if (boxwidth > 0 && (boxwidth < texwidth || (flags & DI_FORCESCALE)))
			{
				scale1 = boxwidth / texwidth;
			}
			if (boxheight != -1 && (boxheight < texheight || (flags & DI_FORCESCALE)))
			{
				scale2 = boxheight / texheight;
			}

			if (flags & DI_FORCESCALE)
			{
				if (boxwidth <= 0 || (boxheight > 0 && scale2 < scale1))
					scale1 = scale2;
			}
			else scale1 = min(scale1, scale2);

			boxwidth = texwidth * scale1;
			boxheight = texheight * scale1;
			boxleftoffs = texleftoffs * scale1;
			boxtopoffs = textopoffs * scale1;
		}
	}
	else
	{
		boxwidth = texwidth;
		boxheight = texheight;
		boxleftoffs = texleftoffs;
		boxtopoffs = textopoffs;
	}

	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	Alpha *= this->Alpha;
	if (Alpha <= 0) return;
	x += drawOffset.X;
	y += drawOffset.Y;

	if (flags & DI_ITEM_RELCENTER)
	{
		if (flags & DI_MIRROR) boxleftoffs = -boxleftoffs;
		if (flags & DI_MIRRORY)	boxtopoffs = -boxtopoffs;
		x -= boxwidth / 2 + boxleftoffs;
		y -= boxheight / 2 + boxtopoffs;
	}
	else
	{
		switch (flags & DI_ITEM_HMASK)
		{
		case DI_ITEM_HCENTER:	x -= boxwidth / 2; break;
		case DI_ITEM_RIGHT:		x -= boxwidth; break;
		case DI_ITEM_HOFFSET:	x -= boxleftoffs; break;
		}

		switch (flags & DI_ITEM_VMASK)
		{
		case DI_ITEM_VCENTER: y -= boxheight / 2; break;
		case DI_ITEM_BOTTOM:  y -= boxheight; break;
		case DI_ITEM_VOFFSET: y -= boxtopoffs; break;
		}
	}

	if (!fullscreenOffsets)
	{
		StatusbarToRealCoords(x, y, boxwidth, boxheight);
	}
	else
	{
		double orgx, orgy;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		DVector2 Scale = GetHUDScale();

		x *= Scale.X;
		y *= Scale.Y;
		boxwidth *= Scale.X;
		boxheight *= Scale.Y;
		x += orgx;
		y += orgy;
	}
	DrawTexture(twod, tex, x, y,
		DTA_TopOffset, 0,
		DTA_LeftOffset, 0,
		DTA_DestWidthF, boxwidth,
		DTA_DestHeightF, boxheight,
		DTA_ClipLeft, 0,
		DTA_ClipTop, 0,
		DTA_ClipBottom, twod->GetHeight(),
		DTA_ClipRight, clipwidth < 0? twod->GetWidth() : int(x + boxwidth * clipwidth),
		DTA_Color, color,
		DTA_TranslationIndex, translation? translation : (flags & DI_TRANSLATABLE) ? GetTranslation().index() : 0,
		DTA_ColorOverlay, (flags & DI_DIM) ? MAKEARGB(170, 0, 0, 0) : 0,
		DTA_Alpha, Alpha,
		DTA_AlphaChannel, !!(flags & DI_ALPHAMAPPED),
		DTA_FillColor, (flags & DI_ALPHAMAPPED) ? 0 : -1,
		DTA_FlipX, !!(flags & DI_MIRROR),
		DTA_FlipY, !!(flags& DI_MIRRORY),
		DTA_LegacyRenderStyle, (flags & DI_ALPHAMAPPED) ? STYLE_Shaded : style,
		TAG_DONE);
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::DrawRotated(FTextureID texture, double x, double y, int flags, double angle, double Alpha, double scaleX, double scaleY, PalEntry color, int translation, ERenderStyle style)
{
	if (!texture.isValid())
		return;

	FGameTexture* tex = TexMan.GetGameTexture(texture, !(flags & DI_DONTANIMATE));
	DrawRotated(tex, x, y, flags, angle, Alpha, scaleX, scaleY, color, translation, style);
}

void DStatusBarCore::DrawRotated(FGameTexture* tex, double x, double y, int flags, double angle, double Alpha, double scaleX, double scaleY, PalEntry color, int translation, ERenderStyle style)
{
	double texwidth = tex->GetDisplayWidth() * scaleX;
	double texheight = tex->GetDisplayHeight() * scaleY;

	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	Alpha *= this->Alpha;
	if (Alpha <= 0) return;
	x += drawOffset.X;
	y += drawOffset.Y;
	DVector2 Scale = GetHUDScale();

	scaleX = 1 / scaleX;
	scaleY = 1 / scaleY;

	if (!fullscreenOffsets)
	{
		StatusbarToRealCoords(x, y, texwidth, texheight);
	}
	else
	{
		double orgx, orgy;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		x *= Scale.X;
		y *= Scale.Y;
		scaleX *= Scale.X;
		scaleY *= Scale.Y;
		x += orgx;
		y += orgy;
	}
	DrawTexture(twod, tex, x, y,
		DTA_ScaleX, scaleX,
		DTA_ScaleY, scaleY,
		DTA_Color, color,
		DTA_CenterOffsetRel, !!(flags & DI_ITEM_RELCENTER),
		DTA_Rotate, angle,
		DTA_TranslationIndex, translation ? translation : (flags & DI_TRANSLATABLE) ? GetTranslation().index() : 0,
		DTA_ColorOverlay, (flags & DI_DIM) ? MAKEARGB(170, 0, 0, 0) : 0,
		DTA_Alpha, Alpha,
		DTA_AlphaChannel, !!(flags & DI_ALPHAMAPPED),
		DTA_FillColor, (flags & DI_ALPHAMAPPED) ? 0 : -1,
		DTA_FlipX, !!(flags & DI_MIRROR),
		DTA_FlipY, !!(flags & DI_MIRRORY),
		DTA_LegacyRenderStyle, style,
		TAG_DONE);
}


//============================================================================
//
// draw a string
//
//============================================================================

void DStatusBarCore::DrawString(FFont* font, const FString& cstring, double x, double y, int flags, double Alpha, int translation, int spacing, EMonospacing monospacing, int shadowX, int shadowY, double scaleX, double scaleY, FTranslationID pt, int style)
{
	bool monospaced = monospacing != EMonospacing::Off;
	double dx = 0;
	int spacingparm = monospaced ? -spacing : spacing;

	switch (flags & DI_TEXT_ALIGN)
	{
	default:
		break;
	case DI_TEXT_ALIGN_RIGHT:
		dx = font->StringWidth(cstring, spacingparm);
		break;
	case DI_TEXT_ALIGN_CENTER:
		dx = font->StringWidth(cstring, spacingparm) / 2;
		break;
	}

	// Take text scale into account
	x -= dx * scaleX;

	const uint8_t* str = (const uint8_t*)cstring.GetChars();
	const EColorRange boldTranslation = EColorRange(translation ? translation - 1 : NumTextColors - 1);
	int fontcolor = translation;
	double orgx = 0, orgy = 0;
	DVector2 Scale;

	if (fullscreenOffsets)
	{
		Scale = GetHUDScale();
		shadowX *= (int)Scale.X;
		shadowY *= (int)Scale.Y;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}
	}
	else
	{
		Scale = { 1.,1. };
	}
	int ch;
	while (ch = GetCharFromString(str), ch != '\0')
	{
		if (ch == ' ')
		{
			x += (monospaced ? spacing : font->GetSpaceWidth() + spacing) * scaleX;
			continue;
		}
		else if (ch == TEXTCOLOR_ESCAPE)
		{
			EColorRange newColor = V_ParseFontColor(str, translation, boldTranslation);
			if (newColor != CR_UNDEFINED)
				fontcolor = newColor;
			continue;
		}

		int width;
		FGameTexture* c = font->GetChar(ch, fontcolor, &width);
		if (c == NULL) //missing character.
		{
			continue;
		}
		width += font->GetDefaultKerning();

		if (!monospaced) //If we are monospaced lets use the offset
			x += (c->GetDisplayLeftOffset() * scaleX + 1); //ignore x offsets since we adapt to character size

		double rx, ry, rw, rh;
		rx = x + drawOffset.X;
		ry = y + drawOffset.Y;
		rw = c->GetDisplayWidth();
		rh = c->GetDisplayHeight();

		if (monospacing == EMonospacing::CellCenter)
			rx += (spacing - rw) / 2;
		else if (monospacing == EMonospacing::CellRight)
			rx += (spacing - rw);

		if (!fullscreenOffsets)
		{
			StatusbarToRealCoords(rx, ry, rw, rh);
		}
		else
		{
			rx *= Scale.X;
			ry *= Scale.Y;
			rw *= Scale.X;
			rh *= Scale.Y;

			rx += orgx;
			ry += orgy;
		}

		// Apply text scale
		rw *= scaleX;
		rh *= scaleY;

		// This is not really such a great way to draw shadows because they can overlap with previously drawn characters.
		// This may have to be changed to draw the shadow text up front separately.
		if ((shadowX != 0 || shadowY != 0) && !(flags & DI_NOSHADOW))
		{
			DrawChar(twod, font, CR_UNTRANSLATED, rx + shadowX, ry + shadowY, ch,
				DTA_DestWidthF, rw,
				DTA_DestHeightF, rh,
				DTA_Alpha, (Alpha * 0.4),
				DTA_FillColor, 0,
				TAG_DONE);
		}
		DrawChar(twod, font, pt == NO_TRANSLATION? fontcolor : CR_NATIVEPAL, rx, ry, ch,
			DTA_DestWidthF, rw,
			DTA_DestHeightF, rh,
			DTA_Alpha, Alpha,
			DTA_TranslationIndex, pt.index(),
			DTA_LegacyRenderStyle, ERenderStyle(style),
			TAG_DONE);

		// Take text scale into account
		dx = monospaced
			? spacing * scaleX
			: (double(width) + spacing - c->GetDisplayLeftOffset()) * scaleX - 1;

		x += dx;
	}
}

void SBar_DrawString(DStatusBarCore* self, DHUDFont* font, const FString& string, double x, double y, int flags, int trans, double alpha, int wrapwidth, int linespacing, double scaleX, double scaleY, int pt_, int style)
{
	if (font == nullptr || font->mFont == nullptr) ThrowAbortException(X_READ_NIL, nullptr);
	if (!twod->HasBegun2D()) ThrowAbortException(X_OTHER, "Attempt to draw to screen outside a draw function");
	auto pt = FTranslationID::fromInt(pt_);

	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	if (wrapwidth > 0)
	{
		auto brk = V_BreakLines(font->mFont, int(wrapwidth * scaleX), string, true);
		for (auto& line : brk)
		{
			self->DrawString(font->mFont, line.Text, x, y, flags, alpha, trans, font->mSpacing, font->mMonospacing, font->mShadowX, font->mShadowY, scaleX, scaleY, pt, style);
			y += (font->mFont->GetHeight() + linespacing) * scaleY;
		}
	}
	else
	{
		self->DrawString(font->mFont, string, x, y, flags, alpha, trans, font->mSpacing, font->mMonospacing, font->mShadowX, font->mShadowY, scaleX, scaleY, pt, style);
	}
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::TransformRect(double& x, double& y, double& w, double& h, int flags)
{
	// resolve auto-alignment before making any adjustments to the position values.
	if (!(flags & DI_SCREEN_MANUAL_ALIGN))
	{
		if (x < 0) flags |= DI_SCREEN_RIGHT;
		else flags |= DI_SCREEN_LEFT;
		if (y < 0) flags |= DI_SCREEN_BOTTOM;
		else flags |= DI_SCREEN_TOP;
	}

	x += drawOffset.X;
	y += drawOffset.Y;

	if (!fullscreenOffsets)
	{
		StatusbarToRealCoords(x, y, w, h);
	}
	else
	{
		double orgx, orgy;

		switch (flags & DI_SCREEN_HMASK)
		{
		default: orgx = 0; break;
		case DI_SCREEN_HCENTER: orgx = twod->GetWidth() / 2; break;
		case DI_SCREEN_RIGHT:   orgx = twod->GetWidth(); break;
		}

		switch (flags & DI_SCREEN_VMASK)
		{
		default: orgy = 0; break;
		case DI_SCREEN_VCENTER: orgy = twod->GetHeight() / 2; break;
		case DI_SCREEN_BOTTOM: orgy = twod->GetHeight(); break;
		}

		DVector2 Scale = GetHUDScale();

		x *= Scale.X;
		y *= Scale.Y;
		w *= Scale.X;
		h *= Scale.Y;
		x += orgx;
		y += orgy;
	}
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::Fill(PalEntry color, double x, double y, double w, double h, int flags)
{
	double Alpha = color.a * this->Alpha / 255;
	if (Alpha <= 0) return;

	TransformRect(x, y, w, h, flags);

	int x1 = int(x);
	int y1 = int(y);
	int ww = int(x + w - x1);	// account for scaling to non-integers. Truncating the values separately would fail for cases like 
	int hh = int(y + h - y1);	// y=3.5, height = 5.5 where adding both values gives a larger integer than adding the two integers.

	Dim(twod, color, float(Alpha), x1, y1, ww, hh);
}


//============================================================================
//
// draw stuff
//
//============================================================================

void DStatusBarCore::SetClipRect(double x, double y, double w, double h, int flags)
{
	TransformRect(x, y, w, h, flags);
	int x1 = int(x);
	int y1 = int(y);
	int ww = int(x + w - x1);	// account for scaling to non-integers. Truncating the values separately would fail for cases like 
	int hh = int(y + h - y1); // y=3.5, height = 5.5 where adding both values gives a larger integer than adding the two integers.
	twod->SetClipRect(x1, y1, ww, hh);
}


