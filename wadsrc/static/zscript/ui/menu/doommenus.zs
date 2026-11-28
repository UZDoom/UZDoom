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
	clearscope static native int GetComplvlInfo();

	override void Drawer ()
	{
		Super.Drawer();

		String s1, s2;
		int mask1 = (gamestate == GS_LEVEL && level.info != null) ? level.info.compatmask : 0;
		int mask2 = (gamestate == GS_LEVEL && level.info != null) ? level.info.compatmask2 : 0;
		if (gamestate == GS_LEVEL)
		{
			bool pureWad1 = (level.compatflags != compatflags) && ((compatflags & ~mask1) == 0);
			bool pureWad2 = (level.compatflags2 != compatflags2) && ((compatflags2 & ~mask2) == 0);

			s1 = pureWad1
				? String.Format("compatflags = (WAD: %d)", level.compatflags)
				: String.Format("compatflags = %d", level.compatflags);
			s2 = pureWad2
				? String.Format("compatflags2 = (WAD: %d)", level.compatflags2)
				: String.Format("compatflags2 = %d", level.compatflags2);
		}
		else
		{
			s1 = String.Format("compatflags = %d", compatflags);
			s2 = String.Format("compatflags2 = %d", compatflags2);
		}
		String s = String.Format("%s  %s", s1, s2);
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);

		int clInfo = GetComplvlInfo();
		if (gamestate != GS_LEVEL && clInfo == 0) return;

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
				String tag;
				int col = OptionMenuSettings.mFontColorValue;

				if (clInfo != 0 && opt.mCVar == CVar.FindCVar('compatmode'))
				{
					if (opt.mCVar.GetInt() == 0)
					{
						bool outOfSpec = ((compatflags & ~mask1) != 0) || ((compatflags2 & ~mask2) != 0);
						bool ign = (clInfo < 0) || outOfSpec;
						String lvlName;
						switch (abs(clInfo))
						{
						case 1: lvlName = "Vanilla"; break;
						case 2: lvlName = "Boom"; break;
						case 3: lvlName = "MBF"; break;
						case 4: lvlName = "MBF21"; break;
						}
						if (lvlName.Length() > 0)
						{
							tag = ign ? String.Format("(%s, ignored)", lvlName) : String.Format("(%s)", lvlName);
							col = ign ? Font.CR_DARKGRAY : OptionMenuSettings.mFontColorValue;
						}
					}
				}
				else if (gamestate == GS_LEVEL)
				{
					int ov = GetCompatOverride(opt.mCVar);
					if (ov != 0)
					{
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
					}
				}

				if (tag.Length() > 0)
				{
					int sel = opt.GetSelection();
					String valText = StringTable.Localize(OptionValues.GetText(opt.mValues, sel));
					if (valText.Length() == 0) valText = StringTable.Localize("$TXT_UNKNOWN");

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
