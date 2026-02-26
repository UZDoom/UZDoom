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
	private String sendItemDisplayLabel;  // e.g. "Silver Key (OQUAKE) x2" - shown above name box
	private int sendMaxQty;
	private String sendInputLine;  // name built from odoom_send_last_char (C++ sets one char per frame)

	// Cached list for RenderOverlay (ui cannot call play-context; no array members in this ZScript build)
	private int cachedStarCount;
	private String cachedStarListForTab;   // "name\tdesc\tgame\n" per STAR item in current tab
	private int cachedLocalCount;
	private String cachedLocalListForTab;  // "displayName\tamount\n" per actor item in current tab

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

			int starCount = 0;
			array<String> starNames, starDescs, starTypes, starGames;
			starCount = BuildStarItemsForTab(starNames, starDescs, starTypes, starGames);
			array<Inventory> tabItems;
			BuildTabInventory(p.mo, tabItems);

			// Group STAR by short label (like OQuake): same label = one row, sum count; keep first raw name per group for send
			array<String> starGroupLabels;
			array<int> starGroupCounts;
			array<String> starGroupFirstNames;
			for (int i = 0; i < starCount; i++)
			{
				String label = StarItemShortLabel(starNames[i], starGames[i]);
				int r = 0;
				for (r = 0; r < starGroupLabels.Size(); r++)
					if (starGroupLabels[r] == label) break;
				if (r >= starGroupLabels.Size())
				{
					starGroupLabels.Push(label);
					starGroupCounts.Push(1);
					starGroupFirstNames.Push(starNames[i]);
				}
				else
					starGroupCounts[r]++;
			}
			int starGroupCount = starGroupLabels.Size();
			cachedStarCount = starGroupCount;
			cachedStarListForTab = "";
			for (int i = 0; i < starGroupCount; i++)
			{
				String line = (starGroupCounts[i] > 1) ? String.Format("%s x%d", starGroupLabels[i], starGroupCounts[i]) : starGroupLabels[i];
				cachedStarListForTab = String.Format("%s%s\n", cachedStarListForTab, line);
			}

			// Group local items by class name (like OQuake): same item type = one row, sum amounts
			array<String> localGroupClass;
			array<String> localGroupDisp;
			array<int> localGroupAmount;
			array<int> localGroupRepIdx;
			for (int i = 0; i < tabItems.Size(); i++)
			{
				String cls = tabItems[i].GetClassName();
				int amt = tabItems[i].Amount;
				int r = 0;
				for (r = 0; r < localGroupClass.Size(); r++)
					if (localGroupClass[r] == cls) break;
				if (r >= localGroupClass.Size())
				{
					localGroupClass.Push(cls);
					localGroupDisp.Push(GetItemDisplayNamePlay(tabItems[i]));
					localGroupAmount.Push(amt);
					localGroupRepIdx.Push(i);
				}
				else
					localGroupAmount[r] += amt;
			}
			int localGroupCount = localGroupClass.Size();
			cachedLocalCount = localGroupCount;
			cachedLocalListForTab = "";
			for (int i = 0; i < localGroupCount; i++)
			{
				String line = (localGroupAmount[i] > 1) ? String.Format("%s  x%d", localGroupDisp[i], localGroupAmount[i]) : localGroupDisp[i];
				cachedLocalListForTab = String.Format("%s%s\n", cachedLocalListForTab, line);
			}

			int listCount = starGroupCount + localGroupCount;
			int maxOffset = listCount - MAX_VISIBLE_ROWS;
			if (maxOffset < 0) maxOffset = 0;
			if (selectedAbsolute >= listCount && listCount > 0) selectedAbsolute = listCount - 1;
			if (selectedAbsolute < 0) selectedAbsolute = 0;
			Inventory selectedItem = null;
			int groupAmountForSend = 0;
			if (selectedAbsolute >= starGroupCount && selectedAbsolute - starGroupCount < localGroupCount)
			{
				int gidx = selectedAbsolute - starGroupCount;
				selectedItem = tabItems[localGroupRepIdx[gidx]];
				groupAmountForSend = localGroupAmount[gidx];
			}
			if (sendPopupMode == 0)
			{

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

			// A or Z = Send to Avatar, C or X = Send to Clan - open send popup for STAR or local items
			bool canSendStar = (selectedAbsolute < starGroupCount && starGroupCount > 0 && starGroupCounts[selectedAbsolute] > 0);
			bool canSendLocal = (selectedItem != null && (selectedItem.Amount > 0 || groupAmountForSend > 0));
			if ((keyAPressed || keyZPressed) && (canSendStar || canSendLocal))
			{
				sendPopupMode = 1;
				if (canSendStar)
				{
					sendMaxQty = starGroupCounts[selectedAbsolute];
					sendItemClass = String.Format("STAR:%s", starGroupFirstNames[selectedAbsolute]);
					sendItemDisplayLabel = (starGroupCounts[selectedAbsolute] > 1) ? String.Format("%s x%d", starGroupLabels[selectedAbsolute], starGroupCounts[selectedAbsolute]) : starGroupLabels[selectedAbsolute];
				}
				else
				{
					sendMaxQty = groupAmountForSend > 0 ? groupAmountForSend : selectedItem.Amount;
					if (sendMaxQty < 1) sendMaxQty = 1;
					sendItemClass = selectedItem.GetClassName();
					String dispName = GetItemDisplayNamePlay(selectedItem);
					sendItemDisplayLabel = (sendMaxQty > 1) ? String.Format("%s x%d", dispName, sendMaxQty) : dispName;
				}
				sendQuantity = sendMaxQty;
				sendButtonFocus = 0;
				sendInputLine = "";
				CVar lineVar = CVar.FindCVar("odoom_send_input_line");
				if (lineVar != null) lineVar.SetString("");
				CVar cv = CVar.FindCVar("odoom_send_popup_open");
				if (cv != null) cv.SetInt(1);
			}
			if ((keyCPressed || keyXPressed) && (canSendStar || canSendLocal))
			{
				sendPopupMode = 2;
				if (canSendStar)
				{
					sendMaxQty = starGroupCounts[selectedAbsolute];
					sendItemClass = String.Format("STAR:%s", starGroupFirstNames[selectedAbsolute]);
					sendItemDisplayLabel = (starGroupCounts[selectedAbsolute] > 1) ? String.Format("%s x%d", starGroupLabels[selectedAbsolute], starGroupCounts[selectedAbsolute]) : starGroupLabels[selectedAbsolute];
				}
				else
				{
					sendMaxQty = groupAmountForSend > 0 ? groupAmountForSend : selectedItem.Amount;
					if (sendMaxQty < 1) sendMaxQty = 1;
					sendItemClass = selectedItem.GetClassName();
					String dispName = GetItemDisplayNamePlay(selectedItem);
					sendItemDisplayLabel = (sendMaxQty > 1) ? String.Format("%s x%d", dispName, sendMaxQty) : dispName;
				}
				sendQuantity = sendMaxQty;
				sendButtonFocus = 0;
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
			// Enter or E = confirm (Send if focus 0, else just close). When result is shown, Enter/E/I closes.
			else if (keyEnterPressed || keyUsePressed)
			{
				CVar statusCv = CVar.FindCVar("odoom_send_status");
				String sendStatus = (statusCv != null) ? statusCv.GetString() : "";
				bool isResult = (sendStatus.Length() > 0 && sendStatus.Compare("Sending...") != 0);
				if (isResult)
				{
					sendPopupMode = 0;
					CVar cv = CVar.FindCVar("odoom_send_popup_open");
					if (cv != null) cv.SetInt(0);
				}
				else if (sendButtonFocus == 0)
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
						// Keep popup open; C++ will set Sending... then Item sent./Send failed (we show and close on key)
					}
					else
					{
						sendPopupMode = 0;
						CVar cv = CVar.FindCVar("odoom_send_popup_open");
						if (cv != null) cv.SetInt(0);
					}
				}
				else
				{
					sendPopupMode = 0;
					CVar cv = CVar.FindCVar("odoom_send_popup_open");
					if (cv != null) cv.SetInt(0);
				}
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

	private bool IsItemInActiveTab(Inventory item, int tabIndex)
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
	private bool IsStarItemInTab(String itemType, String itemName, int tabIndex)
	{
		String t = itemType;
		String n = itemName;
		if (tabIndex == TAB_KEYS) return t.IndexOf("Key") >= 0 || n.IndexOf("key") >= 0;
		if (tabIndex == TAB_POWERUPS) return t.IndexOf("Powerup") >= 0 || t == "Powerup";
		if (tabIndex == TAB_WEAPONS) return t.IndexOf("Weapon") >= 0 || t == "Weapon";
		if (tabIndex == TAB_AMMO) return t.IndexOf("Ammo") >= 0 || t == "Ammo";
		if (tabIndex == TAB_ARMOR) return t.IndexOf("Armor") >= 0 || t == "Armor";
		// TAB_ITEMS: only items that don't fit Keys, Powerups, Weapons, Ammo, or Armor
		if (tabIndex == TAB_ITEMS)
			return (t.IndexOf("Key") < 0 && n.IndexOf("key") < 0) && (t.IndexOf("Powerup") < 0 && t != "Powerup") && (t.IndexOf("Weapon") < 0 && t != "Weapon") && (t.IndexOf("Ammo") < 0 && t != "Ammo") && (t.IndexOf("Armor") < 0 && t != "Armor");
		return true;
	}

	// Short display label for STAR items: "Shells (OQUAKE)" – item name (game in brackets). Names are already short (Shells, Shotgun, etc.).
	private String StarItemShortLabel(String name, String game)
	{
		String n = name;
		String g = (game.Length() > 0) ? game : "STAR";
		if (g == "QUAKE" || g == "Quake" || g == "quake") g = "OQUAKE";
		// ODOOM/Doom-specific mappings
		if (n.IndexOf("Clip") >= 0 || n.IndexOf("clip") >= 0 || n.IndexOf("Bullet") >= 0) return String.Format("Bullets (%s)", g);
		if (n.IndexOf("Shell") >= 0 || n.IndexOf("shell") >= 0) return String.Format("Shells (%s)", g);
		if (n.IndexOf("Cell") >= 0 || n.IndexOf("cell") >= 0) return String.Format("Cells (%s)", g);
		if (n.IndexOf("Armor") >= 0 || n.IndexOf("armor") >= 0) return String.Format("Armor (%s)", g);
		if (n.IndexOf("Stimpack") >= 0 || n.IndexOf("stimpack") >= 0) return String.Format("Stimpack (%s)", g);
		if (n.IndexOf("Medikit") >= 0 || n.IndexOf("medikit") >= 0) return String.Format("Medikit (%s)", g);
		if (n.IndexOf("Backpack") >= 0 || n.IndexOf("backpack") >= 0) return String.Format("Backpack (%s)", g);
		if (n.IndexOf("Weapon") >= 0 || n.IndexOf("weapon") >= 0) return String.Format("Weapon (%s)", g);
		if (n.IndexOf("red_keycard") >= 0 || n.IndexOf("red_key") >= 0) return String.Format("Red Keycard (%s)", g);
		if (n.IndexOf("blue_keycard") >= 0 || n.IndexOf("blue_key") >= 0) return String.Format("Blue Keycard (%s)", g);
		if (n.IndexOf("yellow_keycard") >= 0 || n.IndexOf("yellow_key") >= 0) return String.Format("Yellow Keycard (%s)", g);
		if (n.Length() > 24) return String.Format("%s (%s)", n.Left(21), g);
		return String.Format("%s (%s)", n, g);
	}

	private void BuildTabInventory(Actor owner, out array<Inventory> outItems)
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
	private int BuildStarItemsForTab(out array<String> starNames, out array<String> starDescs, out array<String> starTypes, out array<String> starGames)
	{
		starNames.Clear();
		starDescs.Clear();
		starTypes.Clear();
		starGames.Clear();
		CVar listVar = CVar.FindCVar("odoom_star_inventory_list");
		if (listVar == null) return 0;
		String listStr = listVar.GetString();
		if (listStr.Length() == 0) return 0;
		array<String> lines;
		listStr.Split(lines, "\n", false);
		for (int i = 0; i < lines.Size(); i++)
		{
			array<String> parts;
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

	// Play-context version for building cachedLocalListForTab in WorldTick
	private String GetItemDisplayNamePlay(Inventory item)
	{
		if (item == null) return "";
		if (item is "OQGoldKey") return "Golden Key";
		if (item is "OQSilverKey") return "Silver Key";
		String tag = item.GetTag("");
		if (tag.Length() > 0) return tag;
		return item.GetClassName();
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
		// Use cached list from WorldTick (ui cannot call play-context; cache is string-based)
		int starCount = cachedStarCount;
		int tabSize = cachedLocalCount;
		int listCount = starCount + tabSize;
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

		// Parse cache strings into lines for indexing (local arrays only; no array members)
		array<String> starLines;
		array<String> localLines;
		if (cachedStarListForTab.Length() > 0) cachedStarListForTab.Split(starLines, "\n", false);
		if (cachedLocalListForTab.Length() > 0) cachedLocalListForTab.Split(localLines, "\n", false);

		int y = 58;
		for (int i = 0; i < MAX_VISIBLE_ROWS; i++)
		{
			int idx = drawOffset + i;
			if (idx >= listCount) break;

			bool selected = (i == selectedRow);

			if (idx < starCount && idx < starLines.Size())
			{
				String line = starLines[idx];
				screen.DrawText(f, selected ? Font.CR_GOLD : Font.CR_TAN, 54, y + 1, line, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			}
			else
			{
				int localIdx = idx - starCount;
				if (localIdx >= 0 && localIdx < localLines.Size())
				{
					String line = localLines[localIdx];
					screen.DrawText(f, selected ? Font.CR_GOLD : Font.CR_UNTRANSLATED, 54, y + 1, line, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				}
			}
			y += 16;
		}
		}

		// Send popup overlay (OQuake-style): show Sending... / Item sent. / Send failed like Quake
		if (sendPopupMode != 0)
		{
			CVar statusCv = CVar.FindCVar("odoom_send_status");
			String sendStatus = (statusCv != null) ? statusCv.GetString() : "";
			bool showingResult = (sendStatus.Length() > 0 && sendStatus.Compare("Sending...") != 0);

			String title = (sendPopupMode == 2) ? "SEND TO CLAN" : "SEND TO AVATAR";
			String label = (sendPopupMode == 2) ? "Clan" : "Username";
			int popupW = 200;
			int popupH = 98;
			int popupX = (320 - popupW) / 2;
			int popupY = (200 - popupH) / 2;
			screen.DrawText(f, Font.CR_GOLD, popupX + 8, popupY + 4, title, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			if (sendStatus.Compare("Sending...") == 0)
				screen.DrawText(f, Font.CR_GREEN, popupX + 8, popupY + 16, "Sending...", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			else if (showingResult)
			{
				int cr = (sendStatus.IndexOf("Send failed") >= 0) ? Font.CR_RED : Font.CR_GREEN;
				screen.DrawText(f, cr, popupX + 8, popupY + 16, sendStatus, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				screen.DrawText(f, Font.CR_DARKGRAY, popupX + 8, popupY + 28, "Press Enter or I to close", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			}
			else
			{
				if (sendItemDisplayLabel.Length() > 0)
					screen.DrawText(f, Font.CR_WHITE, popupX + 8, popupY + 16, String.Format("Item: %s", sendItemDisplayLabel), DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				screen.DrawText(f, Font.CR_UNTRANSLATED, popupX + 8, popupY + 26, String.Format("%s: %s_", label, sendInputLine), DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				String qtyText = String.Format("Quantity: %d / %d (Arrows)", sendQuantity, sendMaxQty);
				screen.DrawText(f, Font.CR_UNTRANSLATED, popupX + 8, popupY + 38, qtyText, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				screen.DrawText(f, Font.CR_DARKGRAY, popupX + 8, popupY + 50, "Left=Send  Right=Cancel  Enter=Confirm", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				if (sendButtonFocus == 0)
					screen.DrawText(f, Font.CR_GREEN, popupX + 16, popupY + 66, "[SEND]", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				else
					screen.DrawText(f, Font.CR_GRAY, popupX + 16, popupY + 66, "SEND", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				if (sendButtonFocus == 1)
					screen.DrawText(f, Font.CR_GREEN, popupX + 80, popupY + 66, "[CANCEL]", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
				else
					screen.DrawText(f, Font.CR_GRAY, popupX + 80, popupY + 66, "CANCEL", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			}
		}
	}
}
