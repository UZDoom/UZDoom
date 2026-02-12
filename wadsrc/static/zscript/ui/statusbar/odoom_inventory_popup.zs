class OASISInventoryOverlayHandler : EventHandler
{
	private bool popupOpen;
	private int activeTab;
	private int scrollOffset;
	private int mouseX;
	private int mouseY;
	private bool hasMouse;

	const TAB_KEYS = 0;
	const TAB_POWERUPS = 1;
	const TAB_WEAPONS = 2;
	const TAB_ARMOR = 3;
	const TAB_ITEMS = 4;
	const TAB_COUNT = 5;
	const MAX_VISIBLE_ROWS = 7;

	override void OnRegister()
	{
		IsUiProcessor = true;
		RequireMouse = true;
	}

	private void ClampScroll(int listCount)
	{
		int maxOffset = listCount - MAX_VISIBLE_ROWS;
		if (maxOffset < 0) maxOffset = 0;
		if (scrollOffset < 0) scrollOffset = 0;
		if (scrollOffset > maxOffset) scrollOffset = maxOffset;
	}

	private String TabName(int tabIndex)
	{
		switch (tabIndex)
		{
		case TAB_KEYS: return "Keys";
		case TAB_POWERUPS: return "Powerups";
		case TAB_WEAPONS: return "Weapons";
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
		if (tabIndex == TAB_ARMOR) return item is "Armor";
		return !(item is "Key") && !(item is "Powerup") && !(item is "Weapon") && !(item is "Armor") && !(item is "Ammo");
	}

	private void BuildTabInventory(Actor owner, out Array<Inventory> outItems)
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

	private String ItemDisplayName(Inventory item)
	{
		if (item == null) return "";
		if (item is "OQGoldKey") return "Golden Key";
		if (item is "OQSilverKey") return "Silver Key";
		String tag = item.GetTag("");
		if (tag.Len() > 0) return tag;
		return item.GetClassName();
	}

	private Vector2 MouseToVirtual()
	{
		double sw = screen.GetWidth();
		double sh = screen.GetHeight();
		if (sw <= 0 || sh <= 0) return (0, 0);
		return (mouseX * (320.0 / sw), mouseY * (200.0 / sh));
	}

	private bool PointInRect(Vector2 p, int x, int y, int w, int h)
	{
		return p.X >= x && p.X <= x + w && p.Y >= y && p.Y <= y + h;
	}

	private bool HandleMouseClick()
	{
		if (!popupOpen || !hasMouse) return false;
		Vector2 vp = MouseToVirtual();

		// Close button [X]
		if (PointInRect(vp, 266, 17, 14, 10))
		{
			popupOpen = false;
			return true;
		}

		// Tabs
		int tabX = 40;
		for (int i = 0; i < TAB_COUNT; i++)
		{
			if (PointInRect(vp, tabX, 32, 46, 10))
			{
				activeTab = i;
				scrollOffset = 0;
				return true;
			}
			tabX += 48;
		}

		return false;
	}

	private bool HandleOpenPopupKey(InputEvent e)
	{
		if (e.Type != InputEvent.Type_KeyDown) return false;
		return e.KeyChar == int("i") || e.KeyChar == int("I");
	}

	override ui bool InputProcess(InputEvent e)
	{
		if (e.Type == InputEvent.Type_Mouse)
		{
			mouseX = e.MouseX;
			mouseY = e.MouseY;
			hasMouse = true;
			return popupOpen;
		}

		if (HandleOpenPopupKey(e))
		{
			popupOpen = !popupOpen;
			if (popupOpen)
			{
				activeTab = TAB_KEYS;
				scrollOffset = 0;
			}
			return true;
		}

		if (!popupOpen || e.Type != InputEvent.Type_KeyDown) return false;

		if (e.KeyScan == InputEvent.Key_Mouse1)
		{
			return HandleMouseClick();
		}

		if (e.KeyScan == InputEvent.Key_Escape)
		{
			popupOpen = false;
			return true;
		}

		let p = players[consoleplayer];
		if (!p || !p.mo) return true;

		Array<Inventory> tabItems;
		BuildTabInventory(p.mo, tabItems);
		ClampScroll(tabItems.Size());

		if (e.KeyScan == InputEvent.Key_LeftArrow)
		{
			activeTab = (activeTab + TAB_COUNT - 1) % TAB_COUNT;
			scrollOffset = 0;
			return true;
		}
		if (e.KeyScan == InputEvent.Key_RightArrow)
		{
			activeTab = (activeTab + 1) % TAB_COUNT;
			scrollOffset = 0;
			return true;
		}
		if (e.KeyScan == InputEvent.Key_UpArrow || e.KeyScan == InputEvent.Key_MWheelUp)
		{
			scrollOffset--;
			ClampScroll(tabItems.Size());
			return true;
		}
		if (e.KeyScan == InputEvent.Key_DownArrow || e.KeyScan == InputEvent.Key_MWheelDown)
		{
			scrollOffset++;
			ClampScroll(tabItems.Size());
			return true;
		}

		return true;
	}

	override ui void RenderOverlay(RenderEvent e)
	{
		if (!popupOpen) return;
		let p = players[consoleplayer];
		if (!p || !p.mo) return;

		Array<Inventory> tabItems;
		BuildTabInventory(p.mo, tabItems);
		ClampScroll(tabItems.Size());

		Font f = "SmallFont";

		Screen.DrawFrame(28, 12, 264, 176);
		screen.DrawText(f, Font.CR_GOLD, 36, 18, "OASIS Inventory", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
		screen.DrawText(f, Font.CR_RED, 268, 18, "[X]", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		int tabX = 40;
		for (int i = 0; i < TAB_COUNT; i++)
		{
			int tabColor = (i == activeTab) ? Font.CR_GREEN : Font.CR_GRAY;
			screen.DrawText(f, tabColor, tabX, 33, TabName(i), DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
			tabX += 48;
		}

		screen.DrawText(f, Font.CR_DARKGRAY, 38, 46, "I=Close  Left/Right=Tab  Up/Down/Wheel=Scroll", DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

		int y = 58;
		for (int i = 0; i < MAX_VISIBLE_ROWS; i++)
		{
			int idx = scrollOffset + i;
			if (idx >= tabItems.Size()) break;

			let item = tabItems[idx];
			TextureID icon = item.Icon;
			if (icon.IsValid())
			{
				screen.DrawTexture(icon, true, 40, y, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43, DTA_DestWidth, 10, DTA_DestHeight, 10);
			}

			String label = ItemDisplayName(item);
			String desc = label;
			if (item.Amount > 1) desc = String.Format("%s  x%d", label, item.Amount);
			screen.DrawText(f, Font.CR_UNTRANSLATED, 54, y + 1, desc, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);

			y += 16;
		}

		String footer = String.Format("%s  (%d items)", TabName(activeTab), tabItems.Size());
		screen.DrawText(f, Font.CR_GOLD, 38, 172, footer, DTA_VirtualWidth, 320, DTA_VirtualHeight, 200, DTA_FullscreenScale, FSMode_ScaleToFit43);
	}
}

