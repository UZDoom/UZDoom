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
	private bool wasUseDown;
	private bool wasUser4Down;
	private int lastUser4Tick;

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

		int buttons = p.cmd.buttons;
		bool user1Down = (buttons & BT_USER1) != 0;
		bool user2Down = (buttons & BT_USER2) != 0;
		bool user3Down = (buttons & BT_USER3) != 0;
		bool backDown = (buttons & BT_BACK) != 0;
		bool forwardDown = (buttons & BT_FORWARD) != 0;
		bool useDown = (buttons & BT_USE) != 0;
		bool user4Down = (buttons & BT_USER4) != 0;

		if (user1Down && !wasUser1Down)
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
			if (user2Down && !wasUser2Down)
			{
				activeTab--;
				if (activeTab < 0) activeTab = TAB_COUNT - 1;
				scrollOffset = 0;
				selectedAbsolute = 0;
			}
			if (user3Down && !wasUser3Down)
			{
				activeTab++;
				if (activeTab >= TAB_COUNT) activeTab = 0;
				scrollOffset = 0;
				selectedAbsolute = 0;
			}

			int listCount = 0;
			Inventory selectedItem = null;
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

			if (backDown && !wasBackDown)
			{
				selectedAbsolute--;
				if (selectedAbsolute < 0) selectedAbsolute = 0;
				scrollOffset = selectedAbsolute - MAX_VISIBLE_ROWS + 1;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxOffset) scrollOffset = maxOffset;
			}
			if (forwardDown && !wasForwardDown)
			{
				selectedAbsolute++;
				if (selectedAbsolute >= listCount) selectedAbsolute = listCount - 1;
				if (selectedAbsolute < 0) selectedAbsolute = 0;
				scrollOffset = selectedAbsolute - MAX_VISIBLE_ROWS + 1;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxOffset) scrollOffset = maxOffset;
			}

			if (useDown && !wasUseDown)
			{
				if (selectedItem != null && selectedItem.Amount > 0)
				{
					p.mo.UseInventory(selectedItem);
					if (selectedAbsolute >= listCount && listCount > 0) selectedAbsolute = listCount - 1;
					if (selectedAbsolute < 0) selectedAbsolute = 0;
				}
			}

			if (user4Down && !wasUser4Down)
			{
				if (selectedItem != null && selectedItem.Amount > 0)
				{
					String itemClass = selectedItem.GetClassName();
						int maptime = Level.maptime;
						bool doubleTap = (maptime - lastUser4Tick < 35);
						lastUser4Tick = maptime;
					if (doubleTap)
						Console.Printf("To send to clan, run: star send_clan \"<clan_name>\" \"%s\"\n", itemClass);
					else
						Console.Printf("To send to avatar, run: star send_avatar \"<username>\" \"%s\"\n", itemClass);
				}
			}
		}

		wasUser1Down = user1Down;
		wasUser2Down = user2Down;
		wasUser3Down = user3Down;
		wasBackDown = backDown;
		wasForwardDown = forwardDown;
		wasUseDown = useDown;
		wasUser4Down = user4Down;
	}

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

		Font f = "SmallFont";

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

		screen.DrawText(f, Font.CR_DARKGRAY, 38, 46, "Back/Fwd=Select  Use=Use  User4=Send (2nd=clan)  I=Toggle  O/P=Tabs", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		int y = 58;
		for (int i = 0; i < MAX_VISIBLE_ROWS; i++)
		{
			int idx = drawOffset + i;
			if (idx >= tabItems.Size()) break;

			bool selected = (i == selectedRow);
			if (selected)
			{
				TextureID hiliteTex = TexMan.CheckForTexture("ConsoleBack", TexMan.Type_Any);
				if (!hiliteTex.IsValid()) hiliteTex = TexMan.CheckForTexture("STBAR", TexMan.Type_Any);
				if (hiliteTex.IsValid())
					screen.DrawTexture(hiliteTex, true, 38, y - 1, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43, DTA_DestWidth, 270, DTA_DestHeight, 14, DTA_Alpha, 0.5);
			}

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
}
