/*
** searchfield.zs
**
**
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

//=============================================================================
//
// Option Search Field class.
//
// When the search query is entered, makes Search Menu perform a search.
//
//=============================================================================

class os_SearchField : OptionMenuItemTextField
{
	os_SearchField Init(String label, os_Menu menu, string query)
	{
		Super.Init(label, "");

		mMenu = menu;

		mText = query;

		return self;
	}

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		if (mkey == Menu.MKEY_Enter)
		{
			Menu.MenuSound("menu/choose");
			mEnter = TextEnterMenu.OpenTextEnter(Menu.GetCurrentMenu(), Menu.OptionFont(), mText, -1, fromcontroller);
			mEnter.ActivateMenu();
			return true;
		}
		if (mkey == Menu.MKEY_Input)
		{
			mtext = mEnter.GetText();

			mMenu.search();
		}

		return Super.MenuEvent(mkey, fromcontroller);
	}

	override String Represent()
	{
		return mEnter
			? mEnter.GetText() .. NewSmallFont.GetCursor()
			: mText;
	}

	String GetText() { return mText; }

	private os_Menu mMenu;
	private string  mText;
}
