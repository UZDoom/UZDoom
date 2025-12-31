/*
** custommessagebox.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2017 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2022 Jay
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

class CustomMessageBoxMenuBase : Menu abstract
{
	BrokenLines mMessage;
	uint messageSelection;
	int mMouseLeft, mMouseRight, mMouseY;

	Font textFont, arrowFont;
	int destWidth, destHeight;
	String selector;
	
	abstract uint OptionCount();
	abstract String OptionName(uint index);
	abstract int OptionXOffset(uint index);
	
	abstract int OptionForShortcut(int char_key, out bool activate); // -1 for no shortcut, activate = true if this executes the option immediately

	//=============================================================================
	//
	//
	//
	//=============================================================================

	virtual void Init(Menu parent, String message, bool playsound = false)
	{
		Super.Init(parent);
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
		if (playsound)
		{
			MenuSound ("menu/prompt");
		}
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

		y += fontheight;
		mMouseY = int(y);
		
		let n = optionCount();
		for(uint i = 0; i < n; i++)
        {
			screen.DrawText(textFont, messageSelection == i? OptionMenuSettings.mFontColorSelection : OptionMenuSettings.mFontColor, (destWidth / 2) + OptionXOffset(i), y + (fontheight * i), Stringtable.Localize(optionName(i)), DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, 	true, DTA_ScaleX, NotifyFontScale, DTA_ScaleY, NotifyFontScale);
		}

		if (messageSelection >= 0)
		{
			if ((MenuTime() % 8) < 6)
			{
				screen.DrawText(arrowFont, OptionMenuSettings.mFontColorSelection,
					(destWidth/2 - 3 - arrowFont.StringWidth(selector)) + OptionXOffset(messageSelection), y + fontheight * messageSelection, selector, DTA_VirtualWidth, destWidth, DTA_VirtualHeight, destHeight, DTA_KeepRatio, true);
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

	abstract void HandleResult(int index); // -1 = escape

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool OnUIEvent(UIEvent ev)
	{
		if (ev.type == UIEvent.Type_KeyDown)
		{
			// tolower
			int ch = ev.KeyChar;
			ch = ch >= 65 && ch <91? ch + 32 : ch;
			
			bool activate;
			int opt = optionForShortcut(ch,activate);
			
			if(opt >= 0){
				if(activate || opt == messageSelection) {
					HandleResult(messageSelection);
				} else {
					messageSelection = opt;
				}
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
		if (mkey == MKEY_Up)
		{
			MenuSound("menu/cursor");
			if (messageSelection == 0) messageSelection = optionCount();
			messageSelection--;
			return true;
		}
		else if (mkey == MKEY_Down)
		{
			MenuSound("menu/cursor");
			messageSelection++;
			if (messageSelection == optionCount()) messageSelection = 0;
			return true;
		}
		else if (mkey == MKEY_Enter)
		{
			HandleResult(messageSelection);
			return true;
		}
		else if (mkey == MKEY_Back)
		{
			HandleResult(-1);
			return true;
		}
		return false;
	}

	//=============================================================================
	//
	//
	//
	//=============================================================================

	override bool MouseEvent(int type, int x, int y)
	{
		int fh = textFont.GetHeight() + 1;

		// convert x/y from screen to virtual coordinates, according to CleanX/Yfac use in DrawTexture
		x = x * destWidth / screen.GetWidth();
		y = y * destHeight / screen.GetHeight();
		
		int n = OptionCount();

		if (x >= mMouseLeft && x <= mMouseRight && y >= mMouseY && y < mMouseY + (n * fh))
		{
			messageSelection = (y - mMouseY) / fh;
		}
		if (type == MOUSE_Release)
		{
			return MenuEvent(MKEY_Enter, true);
		}
		return true;
	}


}
