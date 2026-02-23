class OASISInventoryOverlayHandler : EventHandler
{
	private bool popupOpen;
	private int activeTab;
	private int scrollOffset;
	private int selectedAbsolute;
	private bool wasUser1Down;
	private bool wasUser2Down;
	private bool wasUser3Down;
	private bool wasBackDown;
	private bool wasForwardDown;
	private bool wasLookUpDown;
	private bool wasLookDownDown;
	private bool wasUseDown;
	private bool wasUser4Down;
	private bool wasReloadDown;
	private bool wasJumpDown;
	private bool wasCrouchDown;
	private bool wasKeyUpDown;
	private bool wasKeyDownDown;
	private bool wasKeyLeftDown;
	private bool wasKeyRightDown;
	private bool wasKeyUseDown;
	private bool wasKeyADown;
	private bool wasKeyCDown;
	private bool wasKeyZDown;
	private bool wasKeyXDown;
	private bool wasKeyIDown;
	private bool wasKeyODown;
	private bool wasKeyPDown;
	private bool wasKeyEnterDown;

	// Send popup (OQuake-style)
	private int sendPopupMode;   // 0=none, 1=avatar, 2=clan
	private int sendQuantity;
	private int sendButtonFocus;  // 0=Send, 1=Cancel
	private String sendItemClass;
	private int sendMaxQty;
	private String sendInputLine;  // name built from odoom_send_last_char (C++ sets one char per frame)

	const TAB_KEYS = 0;
	const TAB_POWERUPS = 1;
	const TAB_WEAPONS = 2;
	const TAB_AMMO = 3;
	const TAB_ARMOR = 4;
	const TAB_ITEMS = 5;
	const TAB_COUNT = 6;
	const MAX_VISIBLE_ROWS = 7;

	override void OnRegister()
	{
		IsUiProcessor = false;
		RequireMouse = false;
	}

	override void WorldTick()
	{
		if (!playeringame[consoleplayer]) return;
		let p = players[consoleplayer];
		if (!p || !p.mo) return;

		// Tell C++ whether inventory is open (so it can clear/restore key bindings, OQuake-style)
		CVar openVar = CVar.FindCVar("odoom_inventory_open");
		if (openVar != null)
			openVar.SetInt(popupOpen ? 1 : 0);
		// Tell C++ whether send popup is open every frame so it can capture name typing
		CVar sendOpenVar = CVar.FindCVar("odoom_send_popup_open");
		if (sendOpenVar != null)
			sendOpenVar.SetInt(sendPopupMode != 0 ? 1 : 0);

		int buttons = p.cmd.buttons;
		bool user1Down = (buttons & BT_USER1) != 0;
		bool user2Down = (buttons & BT_USER2) != 0;
		bool user3Down = (buttons & BT_USER3) != 0;
		bool backDown = (buttons & BT_BACK) != 0;
		bool forwardDown = (buttons & BT_FORWARD) != 0;
		bool lookUpDown = (buttons & BT_LOOKUP) != 0;
		bool lookDownDown = (buttons & BT_LOOKDOWN) != 0;
		bool useDown = (buttons & BT_USE) != 0;
		bool user4Down = (buttons & BT_USER4) != 0;
		bool reloadDown = (buttons & BT_RELOAD) != 0;
		bool jumpDown = (buttons & BT_JUMP) != 0;
		bool crouchDown = (buttons & BT_CROUCH) != 0;

		// Keys captured by C++ when inventory open (odoom_key_* CVars). Read every frame so wasKey* stay in sync when closed.
		int keyUp = 0, keyDown = 0, keyLeft = 0, keyRight = 0, keyUse = 0, keyA = 0, keyC = 0, keyZ = 0, keyX = 0, keyI = 0, keyO = 0, keyP = 0, keyEnter = 0;
		CVar v;
		v = CVar.FindCVar("odoom_key_up"); if (v != null) keyUp = v.GetInt();
		v = CVar.FindCVar("odoom_key_down"); if (v != null) keyDown = v.GetInt();
		v = CVar.FindCVar("odoom_key_left"); if (v != null) keyLeft = v.GetInt();
		v = CVar.FindCVar("odoom_key_right"); if (v != null) keyRight = v.GetInt();
		v = CVar.FindCVar("odoom_key_use"); if (v != null) keyUse = v.GetInt();
		v = CVar.FindCVar("odoom_key_a"); if (v != null) keyA = v.GetInt();
		v = CVar.FindCVar("odoom_key_c"); if (v != null) keyC = v.GetInt();
		v = CVar.FindCVar("odoom_key_z"); if (v != null) keyZ = v.GetInt();
		v = CVar.FindCVar("odoom_key_x"); if (v != null) keyX = v.GetInt();
		v = CVar.FindCVar("odoom_key_i"); if (v != null) keyI = v.GetInt();
		v = CVar.FindCVar("odoom_key_o"); if (v != null) keyO = v.GetInt();
		v = CVar.FindCVar("odoom_key_p"); if (v != null) keyP = v.GetInt();
		v = CVar.FindCVar("odoom_key_enter"); if (v != null) keyEnter = v.GetInt();
		bool keyUpPressed = (keyUp != 0) && !wasKeyUpDown;
		bool keyDownPressed = (keyDown != 0) && !wasKeyDownDown;
		bool keyLeftPressed = (keyLeft != 0) && !wasKeyLeftDown;
		bool keyRightPressed = (keyRight != 0) && !wasKeyRightDown;
		bool keyUsePressed = (keyUse != 0) && !wasKeyUseDown;
		bool keyAPressed = (keyA != 0) && !wasKeyADown;
		bool keyCPressed = (keyC != 0) && !wasKeyCDown;
		bool keyZPressed = (keyZ != 0) && !wasKeyZDown;
		bool keyXPressed = (keyX != 0) && !wasKeyXDown;
		bool keyIPressed = (keyI != 0) && !wasKeyIDown;
		bool keyOPressed = (keyO != 0) && !wasKeyODown;
		bool keyPPressed = (keyP != 0) && !wasKeyPDown;
		bool keyEnterPressed = (keyEnter != 0) && !wasKeyEnterDown;
		wasKeyUpDown = (keyUp != 0);
		wasKeyDownDown = (keyDown != 0);
		wasKeyLeftDown = (keyLeft != 0);
		wasKeyRightDown = (keyRight != 0);
		wasKeyUseDown = (keyUse != 0);
		wasKeyADown = (keyA != 0);
		wasKeyCDown = (keyC != 0);
		wasKeyZDown = (keyZ != 0);
		wasKeyXDown = (keyX != 0);
		wasKeyIDown = (keyI != 0);
		wasKeyODown = (keyO != 0);
		wasKeyPDown = (keyP != 0);
		wasKeyEnterDown = (keyEnter != 0);

		if ((user1Down && !wasUser1Down) || keyIPressed)
		{
			popupOpen = !popupOpen;
			if (popupOpen)
			{
				scrollOffset = 0;
				selectedAbsolute = 0;
			}
		}

		if (popupOpen)
		{
			if ((user2Down && !wasUser2Down) || keyLeftPressed || keyOPressed)
			{
				activeTab--;
				if (activeTab < 0) activeTab = TAB_COUNT - 1;
				scrollOffset = 0;
				selectedAbsolute = 0;
			}
			if ((user3Down && !wasUser3Down) || keyRightPressed || keyPPressed)
			{
				activeTab++;
				if (activeTab >= TAB_COUNT) activeTab = 0;
				scrollOffset = 0;
				selectedAbsolute = 0;
			}

			int listCount = 0;
			Inventory selectedItem = null;
			if (sendPopupMode == 0)
			{
			for (let inv = p.mo.Inv; inv != null; inv = inv.Inv)
			{
				if (inv.Amount <= 0) continue;
				bool inTab = false;
				if (activeTab == TAB_KEYS && inv is "Key") inTab = true;
				else if (activeTab == TAB_POWERUPS && inv is "Powerup") inTab = true;
				else if (activeTab == TAB_WEAPONS && inv is "Weapon") inTab = true;
				else if (activeTab == TAB_AMMO && inv is "Ammo") inTab = true;
				else if (activeTab == TAB_ARMOR && inv is "Armor") inTab = true;
				else if (activeTab == TAB_ITEMS && !(inv is "Key") && !(inv is "Powerup") && !(inv is "Weapon") && !(inv is "Armor") && !(inv is "Ammo")) inTab = true;
				if (!inTab) continue;
				if (listCount == selectedAbsolute) selectedItem = inv;
				listCount++;
			}
			int maxOffset = listCount - MAX_VISIBLE_ROWS;
			if (maxOffset < 0) maxOffset = 0;
			if (selectedAbsolute >= listCount && listCount > 0) selectedAbsolute = listCount - 1;
			if (selectedAbsolute < 0) selectedAbsolute = 0;

			// Selection: arrows only (from captured key CVars). Do not use W/S so they don't move list or player.
			bool selUp = keyUpPressed || (lookUpDown && !wasLookUpDown) || (jumpDown && !wasJumpDown);
			bool selDown = keyDownPressed || (lookDownDown && !wasLookDownDown) || (crouchDown && !wasCrouchDown);
			if (selUp)
			{
				selectedAbsolute--;
				if (selectedAbsolute < 0) selectedAbsolute = 0;
				scrollOffset = selectedAbsolute - MAX_VISIBLE_ROWS + 1;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxOffset) scrollOffset = maxOffset;
			}
			if (selDown)
			{
				selectedAbsolute++;
				if (selectedAbsolute >= listCount) selectedAbsolute = listCount - 1;
				if (selectedAbsolute < 0) selectedAbsolute = 0;
				scrollOffset = selectedAbsolute - MAX_VISIBLE_ROWS + 1;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxOffset) scrollOffset = maxOffset;
			}

			if ((useDown && !wasUseDown) || keyUsePressed)
			{
				if (selectedItem != null && selectedItem.Amount > 0)
				{
					p.mo.UseInventory(selectedItem);
					if (selectedAbsolute >= listCount && listCount > 0) selectedAbsolute = listCount - 1;
					if (selectedAbsolute < 0) selectedAbsolute = 0;
				}
			}

			// A or Z = Send to Avatar, C or X = Send to Clan - open send popup (OQuake-style)
			if ((keyAPressed || keyZPressed) && selectedItem != null && selectedItem.Amount > 0)
			{
				sendPopupMode = 1;
				sendQuantity = selectedItem.Amount;
				if (sendQuantity < 1) sendQuantity = 1;
				sendButtonFocus = 0;
				sendItemClass = selectedItem.GetClassName();
				sendMaxQty = selectedItem.Amount;
				sendInputLine = "";
				CVar lineVar = CVar.FindCVar("odoom_send_input_line");
				if (lineVar != null) lineVar.SetString("");
				CVar cv = CVar.FindCVar("odoom_send_popup_open");
				if (cv != null) cv.SetInt(1);
			}
			if ((keyCPressed || keyXPressed) && selectedItem != null && selectedItem.Amount > 0)
			{
				sendPopupMode = 2;
				sendQuantity = selectedItem.Amount;
				if (sendQuantity < 1) sendQuantity = 1;
				sendButtonFocus = 0;
				sendItemClass = selectedItem.GetClassName();
				sendMaxQty = selectedItem.Amount;
				sendInputLine = "";
				CVar lineVar = CVar.FindCVar("odoom_send_input_line");
				if (lineVar != null) lineVar.SetString("");
				CVar cv = CVar.FindCVar("odoom_send_popup_open");
				if (cv != null) cv.SetInt(1);
			}
			} // sendPopupMode == 0

		// Send popup handling when open
		if (sendPopupMode != 0)
		{
			// Build name from C++ odoom_send_last_char (one char per frame; 0=none, 8=backspace)
			CVar lastCharVar = CVar.FindCVar("odoom_send_last_char");
			if (lastCharVar != null)
			{
				int ch = lastCharVar.GetInt();
				if (ch == 8 && sendInputLine.Length() > 0)
					sendInputLine = sendInputLine.Left(sendInputLine.Length() - 1);
				else if (ch != 0 && ch != 8 && sendInputLine.Length() < 64)
				{
					String oneChar = "";
					oneChar.AppendCharacter(ch);
					sendInputLine = String.Format("%s%s", sendInputLine, oneChar);
				}
			}
			// Also sync from full-line cvar so typing still shows if last-char path is unavailable.
			CVar lineVarSync = CVar.FindCVar("odoom_send_input_line");
			if (lineVarSync != null)
			{
				String syncLine = lineVarSync.GetString();
				if (syncLine.Length() > 0 || sendInputLine.Length() == 0)
					sendInputLine = syncLine;
			}
			else
			{
				// Fallback for builds missing odoom_send_last_char: read full line from string CVar.
				CVar lineVar = CVar.FindCVar("odoom_send_input_line");
				if (lineVar != null) sendInputLine = lineVar.GetString();
			}
			if (keyLeftPressed) sendButtonFocus = 0;
			if (keyRightPressed) sendButtonFocus = 1;
			if (keyUpPressed && sendQuantity < sendMaxQty) sendQuantity++;
			if (keyDownPressed && sendQuantity > 1) sendQuantity--;
			// I = close popup without sending (cancel)
			if (keyIPressed)
			{
				sendPopupMode = 0;
				CVar cv = CVar.FindCVar("odoom_send_popup_open");
				if (cv != null) cv.SetInt(0);
			}
			// Enter or E = confirm (Send if focus 0, else just close)
			else if (keyEnterPressed || keyUsePressed)
			{
				if (sendButtonFocus == 0)
				{
					if (sendInputLine.Length() > 0)
					{
						CVar t = CVar.FindCVar("odoom_send_target");
						if (t != null) t.SetString(sendInputLine);
						t = CVar.FindCVar("odoom_send_item_class");
						if (t != null) t.SetString(sendItemClass);
						t = CVar.FindCVar("odoom_send_quantity");
						if (t != null) t.SetInt(sendQuantity);
						t = CVar.FindCVar("odoom_send_to_clan");
						if (t != null) t.SetInt(sendPopupMode == 2 ? 1 : 0);
						t = CVar.FindCVar("odoom_send_do_it");
						if (t != null) t.SetInt(1);
					}
				}
				sendPopupMode = 0;
				CVar cv = CVar.FindCVar("odoom_send_popup_open");
				if (cv != null) cv.SetInt(0);
			}
		}

		wasUser1Down = user1Down;
		wasUser2Down = user2Down;
		wasUser3Down = user3Down;
		wasBackDown = backDown;
		wasForwardDown = forwardDown;
		wasLookUpDown = lookUpDown;
		wasLookDownDown = lookDownDown;
		wasUseDown = useDown;
		wasUser4Down = user4Down;
		wasReloadDown = reloadDown;
		wasJumpDown = jumpDown;
		wasCrouchDown = crouchDown;
	}

	} // end WorldTick

	private ui int GetClampedOffset(int listCount)
	{
		int maxOffset = listCount - MAX_VISIBLE_ROWS;
		if (maxOffset < 0) maxOffset = 0;
		int offset = scrollOffset;
		if (offset < 0) offset = 0;
		if (offset > maxOffset) offset = maxOffset;
		return offset;
	}

	private ui String TabName(int tabIndex)
	{
		switch (tabIndex)
		{
		case TAB_KEYS: return "Keys";
		case TAB_POWERUPS: return "Powerups";
		case TAB_WEAPONS: return "Weapons";
		case TAB_AMMO: return "Ammo";
		case TAB_ARMOR: return "Armor";
		default: return "Items";
		}
	}

	private ui bool IsItemInActiveTab(Inventory item, int tabIndex)
	{
		if (item == null || item.Amount <= 0) return false;
		if (tabIndex == TAB_KEYS) return item is "Key";
		if (tabIndex == TAB_POWERUPS) return item is "Powerup";
		if (tabIndex == TAB_WEAPONS) return item is "Weapon";
		if (tabIndex == TAB_AMMO) return item is "Ammo";
		if (tabIndex == TAB_ARMOR) return item is "Armor";
		return !(item is "Key") && !(item is "Powerup") && !(item is "Weapon") && !(item is "Armor") && !(item is "Ammo");
	}

	// STAR item matches tab (same data as "star inventory" command, from odoom_star_inventory_list).
	private ui bool IsStarItemInTab(String itemType, String itemName, int tabIndex)
	{
		String t = itemType;
		String n = itemName;
		if (tabIndex == TAB_KEYS) return t.IndexOf("Key") >= 0 || n.IndexOf("key") >= 0;
		if (tabIndex == TAB_POWERUPS) return t.IndexOf("Powerup") >= 0 || t == "Powerup";
		if (tabIndex == TAB_WEAPONS) return t.IndexOf("Weapon") >= 0 || t == "Weapon";
		if (tabIndex == TAB_AMMO) return t.IndexOf("Ammo") >= 0 || t == "Ammo";
		if (tabIndex == TAB_ARMOR) return t.IndexOf("Armor") >= 0 || t == "Armor";
		return true; // TAB_ITEMS: everything else
	}

	private ui void BuildTabInventory(Actor owner, out Array<Inventory> outItems)
	{
		outItems.Clear();
		if (owner == null) return;

		for (let inv = owner.Inv; inv != null; inv = inv.Inv)
		{
			if (IsItemInActiveTab(inv, activeTab))
			{
				outItems.Push(inv);
			}
		}
	}

	// Parse odoom_star_inventory_list (format "name\tdesc\ttype\tgame\n" per line) and append STAR items for active tab to display list.
	// Returns number of STAR rows added. Row data appended to starNames, starDescs, starTypes, starGames.
	private ui int BuildStarItemsForTab(out Array<String> starNames, out Array<String> starDescs, out Array<String> starTypes, out Array<String> starGames)
	{
		starNames.Clear();
		starDescs.Clear();
		starTypes.Clear();
		starGames.Clear();
		CVar listVar = CVar.FindCVar("odoom_star_inventory_list");
		if (listVar == null) return 0;
		String listStr = listVar.GetString();
		if (listStr.Length() == 0) return 0;
		Array<String> lines;
		listStr.Split(lines, "\n", false);
		for (int i = 0; i < lines.Size(); i++)
		{
			Array<String> parts;
			lines[i].Split(parts, "\t", false);
			if (parts.Size() < 4) continue;
			String name = parts[0];
			String desc = parts[1];
			String typ = parts[2];
			String game = parts[3];
			if (!IsStarItemInTab(typ, name, activeTab)) continue;
			starNames.Push(name);
			starDescs.Push(desc);
			starTypes.Push(typ);
			starGames.Push(game);
		}
		return starNames.Size();
	}

	private ui String ItemDisplayName(Inventory item)
	{
		if (item == null) return "";
		if (item is "OQGoldKey") return "Golden Key";
		if (item is "OQSilverKey") return "Silver Key";
		String tag = item.GetTag("");
		if (tag.Length() > 0) return tag;
		return item.GetClassName();
	}

	override void RenderOverlay(RenderEvent e)
	{
		if (!popupOpen) return;
		let p = players[consoleplayer];
		if (!p || !p.mo) return;

		Font f = "SmallFont";

		// When send popup is open, draw only the send popup (no inventory list behind it)
		if (sendPopupMode != 0)
		{
			// Draw send popup only - see below
		}
		else
		{
		Array<Inventory> tabItems;
		BuildTabInventory(p.mo, tabItems);
		int listCount = tabItems.Size();
		int maxOffset = listCount - MAX_VISIBLE_ROWS;
		if (maxOffset < 0) maxOffset = 0;
		int drawOffset = scrollOffset;
		if (drawOffset < 0) drawOffset = 0;
		if (drawOffset > maxOffset) drawOffset = maxOffset;
		int sel = selectedAbsolute;
		if (sel >= listCount && listCount > 0) sel = listCount - 1;
		if (sel < 0) sel = 0;
		int selectedRow = sel - drawOffset;

		int headerX = 160 - (f.StringWidth("OASIS Inventory") / 2);
		screen.DrawText(f, Font.CR_GOLD, headerX, 18, "OASIS Inventory", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		int tabGap = 10;
		int tabX = 6;
		String tab0 = "Keys";
		String tab1 = "Powerups";
		String tab2 = "Weapons";
		String tab3 = "Ammo";
		String tab4 = "Armor";
		String tab5 = "Items";
		int tab0X = tabX;
		int tab1X = tab0X + f.StringWidth(tab0) + tabGap;
		int tab2X = tab1X + f.StringWidth(tab1) + tabGap;
		int tab3X = tab2X + f.StringWidth(tab2) + tabGap;
		int tab4X = tab3X + f.StringWidth(tab3) + tabGap;
		int tab5X = tab4X + f.StringWidth(tab4) + tabGap;
		screen.DrawText(f, activeTab == TAB_KEYS ? Font.CR_GREEN : Font.CR_GRAY, tab0X, 33, tab0, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, activeTab == TAB_POWERUPS ? Font.CR_GREEN : Font.CR_GRAY, tab1X, 33, tab1, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, activeTab == TAB_WEAPONS ? Font.CR_GREEN : Font.CR_GRAY, tab2X, 33, tab2, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, activeTab == TAB_AMMO ? Font.CR_GREEN : Font.CR_GRAY, tab3X, 33, tab3, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, activeTab == TAB_ARMOR ? Font.CR_GREEN : Font.CR_GRAY, tab4X, 33, tab4, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, activeTab == TAB_ITEMS ? Font.CR_GREEN : Font.CR_GRAY, tab5X, 33, tab5, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		screen.DrawText(f, Font.CR_DARKGRAY, -16, 46, "Arrows=Select E=Use A=Avatar C=Clan I=Close O/P=Tabs", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		int y = 58;
		for (int i = 0; i < MAX_VISIBLE_ROWS; i++)
		{
			int idx = drawOffset + i;
			if (idx >= tabItems.Size()) break;

			bool selected = (i == selectedRow);

			let item = tabItems[idx];
			TextureID icon = item.Icon;
			if (icon.IsValid())
			{
				screen.DrawTexture(icon, true, 40, y, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43, DTA_DestWidth, 10, DTA_DestHeight, 10);
			}

			String itemLabel = ItemDisplayName(item);
			String itemDesc = itemLabel;
			if (item.Amount > 1) itemDesc = String.Format("%s  x%d", itemLabel, item.Amount);
			screen.DrawText(f, selected ? Font.CR_GOLD : Font.CR_UNTRANSLATED, 54, y + 1, itemDesc, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

			y += 16;
		}
		}

		// Send popup overlay (OQuake-style)
		if (sendPopupMode != 0)
		{
			String title = (sendPopupMode == 2) ? "SEND TO CLAN" : "SEND TO AVATAR";
			String label = (sendPopupMode == 2) ? "Clan" : "Username";
			int popupW = 200;
			int popupH = 88;
			int popupX = (320 - popupW) / 2;
			int popupY = (200 - popupH) / 2;
			screen.DrawText(f, Font.CR_GOLD, popupX + 8, popupY + 4, title, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			screen.DrawText(f, Font.CR_UNTRANSLATED, popupX + 8, popupY + 20, String.Format("%s: %s_", label, sendInputLine), DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			String qtyText = String.Format("Quantity: %d / %d (Arrows)", sendQuantity, sendMaxQty);
			screen.DrawText(f, Font.CR_UNTRANSLATED, popupX + 8, popupY + 32, qtyText, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			screen.DrawText(f, Font.CR_DARKGRAY, popupX + 8, popupY + 44, "Left=Send  Right=Cancel  Enter=Confirm", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			if (sendButtonFocus == 0)
				screen.DrawText(f, Font.CR_GREEN, popupX + 16, popupY + 60, "[SEND]", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			else
				screen.DrawText(f, Font.CR_GRAY, popupX + 16, popupY + 60, "SEND", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			if (sendButtonFocus == 1)
				screen.DrawText(f, Font.CR_GREEN, popupX + 80, popupY + 60, "[CANCEL]", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			else
				screen.DrawText(f, Font.CR_GRAY, popupX + 80, popupY + 60, "CANCEL", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		}
	}
}
