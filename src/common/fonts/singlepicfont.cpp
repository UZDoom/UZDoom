/*
** singlepicfont.cpp
**
** Font management
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#include "engineerrors.h"
#include "textures.h"
#include "v_font.h"
#include "filesystem.h"
#include "texturemanager.h"

class FSinglePicFont : public FFont
{
public:
	FSinglePicFont(const char *picname);

	// FFont interface
	FGameTexture *GetChar(int code, int translation, int *const width) const override;
	int GetCharWidth (int code) const override;

protected:
	FTextureID PicNum;
};

//==========================================================================
//
// FSinglePicFont :: FSinglePicFont
//
// Creates a font to wrap a texture so that you can use hudmessage as if it
// were a hudpic command. It does not support translation, but animation
// is supported, unlike all the real fonts.
//
//==========================================================================

FSinglePicFont::FSinglePicFont(const char *picname) :
	FFont(-1) // Since lump is only needed for priority information we don't need to worry about this here.
{
	FTextureID texid = TexMan.CheckForTexture (picname, ETextureType::Any);

	if (!texid.isValid())
	{
		I_FatalError ("%s is not a font or texture", picname);
	}

	auto pic = TexMan.GetGameTexture(texid);

	FontName = picname;
	FontHeight = (int)pic->GetDisplayHeight();
	SpaceWidth = (int)pic->GetDisplayWidth();
	GlobalKerning = 0;
	FirstChar = LastChar = 'A';
	PicNum = texid;
}

//==========================================================================
//
// FSinglePicFont :: GetChar
//
// Returns the texture if code is 'a' or 'A', otherwise nullptr.
//
//==========================================================================

FGameTexture *FSinglePicFont::GetChar (int code, int translation, int *const width) const
{
	*width = SpaceWidth;
	if (code == 'a' || code == 'A')
	{
		return TexMan.GetGameTexture(PicNum, true);
	}
	else
	{
		return nullptr;
	}
}

//==========================================================================
//
// FSinglePicFont :: GetCharWidth
//
// Don't expect the text functions to work properly if I actually allowed
// the character width to vary depending on the animation frame.
//
//==========================================================================

int FSinglePicFont::GetCharWidth (int code) const
{
	return SpaceWidth;
}

FFont *CreateSinglePicFont(const char *picname)
{
	return new FSinglePicFont(picname);
}
