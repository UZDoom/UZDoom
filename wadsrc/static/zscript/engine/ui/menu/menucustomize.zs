/*
** menucustomize.zs
**
** This class allows global customization of certain menu aspects,
** e.g. replacing the menu caption.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

class MenuDelegateBase ui
{
	virtual int DrawCaption(String title, Font fnt, int y, bool drawit)
	{
		if (drawit) screen.DrawText(fnt, OptionMenuSettings.mTitleColor, (screen.GetWidth() - fnt.StringWidth(title) * CleanXfac_1) / 2, 10 * CleanYfac_1, title, DTA_CleanNoMove_1, true);
		return (y + fnt.GetHeight()) * CleanYfac_1;	// return is spacing in screen pixels.
	}

	virtual void PlaySound(Name sound)
	{
	}

	virtual bool DrawSelector(ListMenuDescriptor desc)
	{
		return false;
	}

	virtual void MenuDismissed()
	{
		// overriding this allows to execute special actions when the menu closes
	}

	virtual Font PickFont(Font fnt)
	{
		if (generic_ui || !fnt) return NewSmallFont;
		if (fnt == SmallFont) return AlternativeSmallFont;
		if (fnt == BigFont) return AlternativeBigFont;
		return fnt;
	}
}
