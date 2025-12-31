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
** Copyright 2025 UZDoom Maintainers and Contributors
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
