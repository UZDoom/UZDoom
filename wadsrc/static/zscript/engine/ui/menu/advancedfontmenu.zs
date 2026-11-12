/*
** advancedfontmenu.zs
**
** The font remapping menu
**
**---------------------------------------------------------------------------
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class AdvancedFontMenu : OptionMenu
{
	static native void FillAdvancedFontMenu(OptionMenuDescriptor desc);

	override void Init(Menu parent, OptionMenuDescriptor desc)
	{
		super.Init(parent, desc);
		FillAdvancedFontMenu(desc);
	}
}

class AdvancedFontMenuRemapChoiceMenu : OptionMenu
{
	static native void FillAdvancedFontMenuRemapChoices(OptionMenuDescriptor desc);

	override void Init(Menu parent, OptionMenuDescriptor desc)
	{
		super.Init(parent, desc);
		FillAdvancedFontMenuRemapChoices(desc);
	}
}
