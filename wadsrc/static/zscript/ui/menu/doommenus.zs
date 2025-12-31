/*
** doommenus.zs
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
*/

class GameplayMenu : OptionMenu
{
	override void Drawer ()
	{
		Super.Drawer();

		String s = String.Format("dmflags = %d  dmflags2 = %d  dmflags3 = %d", dmflags, dmflags2, dmflags3);
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);
	}
}

class CompatibilityMenu : OptionMenu
{
	override void Drawer ()
	{
		Super.Drawer();

		String s = String.Format("compatflags = %d  compatflags2 = %d", compatflags, compatflags2);
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);
	}
}

//=============================================================================
//
// Placeholder classes for overhauled video mode menu. Do not use!
// Their sole purpose is to support mods with full copy of embedded MENUDEF
//
//=============================================================================

class OptionMenuItemScreenResolution : OptionMenuItem
{
	String mResTexts[3];
	int mSelection;
	int mHighlight;
	int mMaxValid;

	enum EValues
	{
		SRL_INDEX = 0x30000,
		SRL_SELECTION = 0x30003,
		SRL_HIGHLIGHT = 0x30004,
	};

	OptionMenuItemScreenResolution Init(String command)
	{
		return self;
	}

	override bool Selectable()
	{
		return false;
	}
}

class VideoModeMenu : OptionMenu
{
	static bool SetSelectedSize()
	{
		return false;
	}
}

class DoomMenuDelegate : MenuDelegateBase
{
	override void PlaySound(Name snd)
	{
		String s = snd;
		S_StartSound (s, CHAN_VOICE, CHANF_UI, snd_menuvolume); 	
	}
} 
