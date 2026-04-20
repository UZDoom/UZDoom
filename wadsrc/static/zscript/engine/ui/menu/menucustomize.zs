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
