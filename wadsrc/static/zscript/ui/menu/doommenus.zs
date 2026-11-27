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
	clearscope static native int GetCompatOverride(CVar cv);

	override void Drawer ()
	{
		Super.Drawer();

		String s;
		if (gamestate == GS_LEVEL && (level.compatflags != compatflags || level.compatflags2 != compatflags2))
		{
			s = String.Format("compatflags = %d (WAD: %d)  compatflags2 = %d (WAD: %d)",
				compatflags, level.compatflags, compatflags2, level.compatflags2);
		}
		else
		{
			s = String.Format("compatflags = %d  compatflags2 = %d", compatflags, compatflags2);
		}
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);

		if (gamestate != GS_LEVEL) return;

		int y = mDesc.mPosition;
		if (y <= 0)
		{
			y = DrawCaption(mDesc.mTitle, -y, false);
		}
		int fontheight = OptionHeight() * CleanYfac_1;
		int indent = GetIndent();

		for (int i = 0; i < mDesc.mItems.Size() && y <= LastRow; i++)
		{
			if (i == mDesc.mScrollTop)
			{
				i += mDesc.mScrollPos;
				if (i >= mDesc.mItems.Size()) break;
				if (i < 0) i = 0;
			}
			if (!mDesc.mItems[i].Visible())
			{
				continue;
			}

			let opt = OptionMenuItemOption(mDesc.mItems[i]);
			if (opt != null && opt.mCVar != null)
			{
				int ov = GetCompatOverride(opt.mCVar);
				if (ov != 0)
				{
					int sel = opt.GetSelection();
					String valText = StringTable.Localize(OptionValues.GetText(opt.mValues, sel));
					if (valText.Length() == 0) valText = StringTable.Localize("$TXT_UNKNOWN");

					String tag;
					int col;
					switch (ov)
					{
					case 1:
						tag = StringTable.Localize("$CMPTMNU_MAP_NO");
						col = OptionMenuSettings.mFontColorValue;
						break;
					case 2:
						tag = StringTable.Localize("$CMPTMNU_MAP_YES");
						col = OptionMenuSettings.mFontColorValue;
						break;
					case 3:
						tag = StringTable.Localize("$CMPTMNU_AUTO_YES");
						col = Font.CR_LIGHTBLUE;
						break;
					case 4:
						tag = StringTable.Localize("$CMPTMNU_MAP_NO_IGN");
						col = Font.CR_DARKGRAY;
						break;
					default:
						tag = StringTable.Localize("$CMPTMNU_MAP_YES_IGN");
						col = Font.CR_DARKGRAY;
						break;
					}

					int itemIndent = opt.mCenter ? (screen.GetWidth() / 2) : indent;
					int x = itemIndent + opt.CursorSpace() + (OptionWidth(valText) + 6) * CleanXfac_1;
					DrawOptionText(x, y, col, tag, false, false);
				}
			}

			y += fontheight;
		}
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
