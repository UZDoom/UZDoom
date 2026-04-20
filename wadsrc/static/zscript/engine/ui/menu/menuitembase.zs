/*
** menuitembase.zs
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
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

//=============================================================================
//
// base class for menu items
//
//=============================================================================

class MenuItemBase : Object native ui version("2.4")
{
	protected native string mTooltip;
	protected native double mXpos, mYpos;
	protected native Name mAction;
	native int mEnabled;

	void Init(double xpos = 0, double ypos = 0, Name actionname = 'None')
	{
		mXpos = xpos;
		mYpos = ypos;
		mAction = actionname;
		mEnabled = true;
	}

	virtual bool CheckCoordinate(int x, int y) { return false; }
	virtual void Ticker() {}
	virtual void Drawer(bool selected) {}
	virtual bool Selectable() { return false; }
	virtual bool Visible() { return true; }
	virtual bool Activate() { return false; }
	virtual Name, int GetAction() { return mAction, 0; }
	virtual bool SetString(int i, String s) { return false; }
	virtual bool, String GetString(int i) { return false, ""; }
	virtual bool SetValue(int i, int value) { return false; }
	virtual bool, int GetValue(int i)  { return false, 0; }
	virtual void Enable(bool on) { mEnabled = on; }
	virtual bool MenuEvent (int mkey, bool fromcontroller) { return false; }
	virtual bool MouseEvent(int type, int x, int y) { return false; }
	virtual bool CheckHotkey(int c) { return false; }
	virtual int GetWidth() { return 0; }
	virtual int GetIndent() { return 0; }
	virtual int Draw(OptionMenuDescriptor desc, int y, int indent, bool selected) { return indent; }

	version("4.15.1") string GetTooltip() const { return mTooltip; }
	version("4.15.1") void SetTooltip(string tooltip) { mTooltip = tooltip; }
	void OffsetPositionY(double ydelta) { mYpos += ydelta; }
	double GetY() { return mYpos; }
	double GetX() { return mXpos; }
	void SetX(double x) { mXpos = x; }
	void SetY(double x) { mYpos = x; }
	virtual void OnMenuCreated() {}
}

// this is only used to parse font color ranges in MENUDEF
enum MenudefColorRange
{
	NO_COLOR = -1
}
