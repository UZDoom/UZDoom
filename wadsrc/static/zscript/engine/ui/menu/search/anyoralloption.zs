/*
** anyoralloption.zs
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
// os_AnyOrAllOption class represents an Option Item for Option Search menu.
// Changing the value of this option causes the menu to refresh the search
// results.
//
//=============================================================================

class os_AnyOrAllOption : OptionMenuItemOption
{
	os_AnyOrAllOption Init(os_Menu menu)
	{
		Super.Init("", "os_isanyof", "os_isanyof_values");

		mMenu = menu;

		return self;
	}

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		bool result = Super.MenuEvent(mkey, fromcontroller);

		if (mKey == Menu.MKEY_Left || mKey == Menu.MKEY_Right || mkey == Menu.MKEY_Enter)
		{
			mMenu.search();
		}

		return result;
	}

	private os_Menu mMenu;
}
