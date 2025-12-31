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
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
