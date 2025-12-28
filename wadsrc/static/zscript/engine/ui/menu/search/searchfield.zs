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
