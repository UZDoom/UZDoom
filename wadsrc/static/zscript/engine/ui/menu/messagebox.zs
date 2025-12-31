/*
** messagebox.zs
**
** Confirmation, notification screens
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2017 Christoph Oelckers
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

class MessageBoxMenu : Menu
{
	private readonly voidptr InternalHandler;

	BrokenLines mMessage;
	int mMessageMode;
	int messageSelection;
	int mMouseLeft, mMouseRight, mMouseY;
	Name mAction;

	Font textFont, arrowFont;
	int destWidth, destHeight;
	String selector;

	voidptr Handler; // This is only here for backwards compat.

	native void CallHandler(voidptr hnd = null);


	//=============================================================================
	//
	//
	//
	//=============================================================================

	virtual void Init(Menu parent, String message, int messagemode, bool playsound = false, Name cmd = 'None', voidptr native_handler = null)
	{
		Super.Init(parent);
		mAction = cmd;
		messageSelection = 0;
		mMouseLeft = 140;
		mMouseY = 0x80000000;
		textFont = null;

		if (!generic_ui)
		{
			if (SmallFont && SmallFont.CanPrint(message) && SmallFont.CanPrint("$TXT_YES") && SmallFont.CanPrint("$TXT_NO")) textFont = SmallFont;
			else if (OriginalSmallFont && OriginalSmallFont.CanPrint(message) && OriginalSmallFont.CanPrint("$TXT_YES") && OriginalSmallFont.CanPrint("$TXT_NO")) textFont = OriginalSmallFont;
		}

		if (!textFont)
		{
			arrowFont = textFont = NewSmallFont;
			int factor = (CleanXfac+1) / 2;
			destWidth = screen.GetWidth() / factor;
			destHeight = screen.GetHeight() / factor;
			selector = "▶";
		}
		else
		{
			arrowFont = ((textFont && textFont.GetGlyphHeight(0xd) > 0) ? textFont : ConFont);
			destWidth = CleanWidth;
			destHeight = CleanHeight;
			selector = "\xd";
		}

		int mr1 = destWidth/2 + 10 + textFont.StringWidth(Stringtable.Localize("$TXT_YES"));
		int mr2 = destWidth/2 + 10 + textFont.StringWidth(Stringtable.Localize("$TXT_NO"));
		mMouseRight = MAX(mr1, mr2);
		mParentMenu = parent;
		mMessage = textFont.BreakLines(Stringtable.Localize(message), int(300/NotifyFontScale));
		mMessageMode = messagemode;
		if (playsound)
		{
			MenuSound ("menu/prompt");
		}
		Handler = native_handler; // Try and avoid breaking null checks from existing overrides.
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override void Drawer ()
	{
		int i;
		double y;
		let fontheight = textFont.GetHeight() * NotifyFontScale;

		y = destHeight / 2;

		int c = mMessage.Count();
		y -= c * fontHeight / 2;

		for (i = 0; i < c; i++)
		{
			screen.DrawText (textFont, Font.CR_UNTRANSLATED, destWidth/2 - mMessage.StringWidth(i)*NotifyFontScale/2, y, mMessage.StringAt(i), DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, true, 
				DTA_ScaleX, NotifyFontScale, DTA_ScaleY, NotifyFontScale);
			y += fontheight;
		}

		if (mMessageMode == 0)
		{
			y += fontheight;
			mMouseY = int(y);
			screen.DrawText(textFont, messageSelection == 0? OptionMenuSettings.mFontColorSelection : OptionMenuSettings.mFontColor, destWidth / 2, y, Stringtable.Localize("$TXT_YES"), DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, 	true, DTA_ScaleX, NotifyFontScale, DTA_ScaleY, NotifyFontScale);
			screen.DrawText(textFont, messageSelection == 1? OptionMenuSettings.mFontColorSelection : OptionMenuSettings.mFontColor, destWidth / 2, y + fontheight, Stringtable.Localize("$TXT_NO"), DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, true, DTA_ScaleX, NotifyFontScale, DTA_ScaleY, NotifyFontScale);

			if (messageSelection >= 0)
			{
				if ((MenuTime() % 8) < 6)
				{
					screen.DrawText(arrowFont, OptionMenuSettings.mFontColorSelection,
						destWidth/2 - 3 - arrowFont.StringWidth(selector), y + fontheight * messageSelection, selector, DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, true);
				}
			}
		}
	}


	//=============================================================================
	//
	//
	//
	//=============================================================================

	protected void CloseSound()
	{
		MenuSound (GetCurrentMenu() != NULL? "menu/backup" : "menu/dismiss");
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	virtual void HandleResult(bool res)
	{
		if (Handler != null)
		{
			if (res) 
			{
				CallHandler();
			}
			else
			{
				Close();
				CloseSound();
			}
		}
		else if (mParentMenu != NULL)
		{
			if (mMessageMode == 0)
			{
				if (mAction == 'None') 
				{
					mParentMenu.MenuEvent(res? MKEY_MBYes : MKEY_MBNo, false);
					Close();
				}
				else 
				{
					Close();
					if (res) SetMenu(mAction, -1);
				}
				CloseSound();
			}
		}
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool OnUIEvent(UIEvent ev)
	{
		if (ev.type == UIEvent.Type_KeyDown)
		{
			if (mMessageMode == 0)
			{
				// tolower
				int ch = ev.KeyChar;
				ch = ch >= 65 && ch <91? ch + 32 : ch;

				if (ch == 110 /*'n'*/ || ch == 32) 
				{
					HandleResult(false);		
					return true;
				}
				else if (ch == 121 /*'y'*/) 
				{
					HandleResult(true);
					return true;
				}
			}
			else
			{
				Close();
				return true;
			}
			return false;
		}
		return Super.OnUIEvent(ev);
	}

	override bool OnInputEvent(InputEvent ev)
	{
		if (ev.type == InputEvent.Type_KeyDown)
		{
			Close();
			return true;
		}
		return Super.OnInputEvent(ev);
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		if (mMessageMode == 0)
		{
			if (mkey == MKEY_Up || mkey == MKEY_Down)
			{
				MenuSound("menu/cursor");
				messageSelection = !messageSelection;
				return true;
			}
			else if (mkey == MKEY_Enter)
			{
				// 0 is yes, 1 is no
				HandleResult(!messageSelection);
				return true;
			}
			else if (mkey == MKEY_Back)
			{
				HandleResult(false);
				return true;
			}
			return false;
		}
		else
		{
			Close();
			CloseSound();
			return true;
		}
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MouseEvent(int type, int x, int y)
	{
		if (mMessageMode == 1)
		{
			if (type == MOUSE_Click)
			{
				return MenuEvent(MKEY_Enter, true);
			}
			return false;
		}
		else
		{
			int sel = -1;
			int fh = textFont.GetHeight() + 1;

			// convert x/y from screen to virtual coordinates, according to CleanX/Yfac use in DrawTexture
			x = x * destWidth / screen.GetWidth();
			y = y * destHeight / screen.GetHeight();

			if (x >= mMouseLeft && x <= mMouseRight && y >= mMouseY && y < mMouseY + 2 * fh)
			{
				sel = y >= mMouseY + fh;
			}
			messageSelection = sel;
			if (type == MOUSE_Release)
			{
				return MenuEvent(MKEY_Enter, true);
			}
			return true;
		}
	}


}



