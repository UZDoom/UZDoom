/*
** readthis.zs
**
** Help screens
**
**---------------------------------------------------------------------------
**
** Copyright 2001-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
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

class ReadThisMenu : GenericMenu
{
	int mScreen;
	int mInfoTic;
	int mDirection;

	//=============================================================================
	//
	//
	//
	//=============================================================================
	
	override void Init(Menu parent)
	{
		Super.Init(parent);
		mScreen = 1;
		mInfoTic = gametic;
		mDirection = 1;
	}

	override void Drawer()
	{
		double alpha;
		TextureID tex, prevpic;
		
		// Did the mapper choose a custom help page via MAPINFO?
		if (Level.F1Pic.Length() != 0)
		{
			tex = TexMan.CheckForTexture(Level.F1Pic, TexMan.Type_MiscPatch);
			mScreen = 1;
		}
		
		if (!tex.IsValid())
		{
			tex = TexMan.CheckForTexture(gameinfo.infoPages[mScreen - 1], TexMan.Type_MiscPatch);
		}

		if (mScreen - mDirection >= 1 &&  mScreen - mDirection <= gameinfo.infoPages.Size())
		{
			prevpic = TexMan.CheckForTexture(gameinfo.infoPages[mScreen - 1 - mDirection], TexMan.Type_MiscPatch);
		}

		screen.Dim(0, 1.0, 0,0, screen.GetWidth(), screen.GetHeight());
		alpha = MIN((gametic - mInfoTic) * (3. / GameTicRate), 1.);
		if (alpha < 1. && prevpic.IsValid())
		{
			screen.DrawTexture (prevpic, false, 0, 0, DTA_Fullscreen, true);
		}
		else alpha = 1;
		screen.DrawTexture (tex, false, 0, 0, DTA_Fullscreen, true, DTA_Alpha, alpha);

	}


	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		if (mkey == MKEY_Enter)
		{
			MenuSound("menu/choose");
			mScreen++;
			mInfoTic = gametic;
			mDirection = 1;
			if (Level.F1Pic.Length() != 0 || mScreen > gameinfo.infoPages.Size())
			{
				Close();
			}
			return true;
		}
		else if (mkey == MKEY_Clear)
		{
			MenuSound("menu/choose");
			mScreen--;
			mInfoTic = gametic;
			mDirection = -1;
			if (Level.F1Pic.Length() != 0 || mScreen <= 0)
			{
				Close();
			}
			return true;
		}
		else return Super.MenuEvent(mkey, fromcontroller);
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MouseEvent(int type, int x, int y)
	{
		if (type == MOUSE_Click)
		{
			return MenuEvent(MKEY_Enter, true);
		}
		return false;
	}

}
