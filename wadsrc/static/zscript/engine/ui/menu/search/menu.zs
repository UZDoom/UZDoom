/*
** menu.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
// Option Search Menu class.
// This menu contains search field, and is dynamically filled with search
// results.
//
//=============================================================================

class os_Menu : OptionMenu
{
	override void Init(Menu parent, OptionMenuDescriptor desc)
	{
		Super.Init(parent, desc);

		mDesc.mItems.clear();

		addSearchField();

		mDesc.mScrollPos    = 0;
		mDesc.mSelectedItem = 0;
		mDesc.CalcIndent();
	}

	void search()
	{
		string text             = mSearchField.GetText();
		let    query            = os_Query.fromString(text);
		bool   isAnyTermMatches = mIsAnyOfItem.mCVar.GetBool();

		mDesc.mItems.clear();

		addSearchField(text);

		Dictionary searchedMenus = Dictionary.Create();
		bool found = listOptions(mDesc, "MainMenu", query, "", isAnyTermMatches, searchedMenus);

		if (!found) { addNoResultsItem(mDesc); }

		mDesc.CalcIndent();
	}

	private void addSearchField(string query = "")
	{
		string searchLabel = StringTable.Localize("$OS_LABEL");

		mSearchField = new("os_SearchField").Init(searchLabel, self, query);
		mIsAnyOfItem = new("os_AnyOrAllOption").Init(self);

		mDesc.mItems.push(mSearchField);
		mDesc.mItems.push(mIsAnyOfItem);
		addEmptyLine(mDesc);
	}

	private static bool listOptions(OptionMenuDescriptor targetDesc,
	                                string   menuName,
	                                os_Query query,
	                                string   path,
	                                bool     isAnyTermMatches,
	                                Dictionary searchedMenus)
	{
		if (searchedMenus.At(menuName).length() > 0)
		{
			return false;
		}

		searchedMenus.Insert(menuName, "1");

		let desc         = MenuDescriptor.GetDescriptor(menuName);
		let listMenuDesc = ListMenuDescriptor(desc);

		if (listMenuDesc)
		{
			return listOptionsListMenu(listMenuDesc, targetDesc, query, path, isAnyTermMatches, searchedMenus);
		}

		let optionMenuDesc = OptionMenuDescriptor(desc);

		if (optionMenuDesc)
		{
			return listOptionsOptionMenu(optionMenuDesc, targetDesc, query, path, isAnyTermMatches, searchedMenus);
		}

		return false;
	}

	private static bool listOptionsListMenu(ListMenuDescriptor   sourceDesc,
	                                        OptionMenuDescriptor targetDesc,
	                                        os_Query query,
	                                        string   path,
	                                        bool     isAnyTermMatches,
	                                        Dictionary searchedMenus)
	{
		int  nItems = sourceDesc.mItems.size();
		bool found  = false;

		for (int i = 0; i < nItems; ++i)
		{
			let    item     = sourceDesc.mItems[i];
			string actionN  = item.GetAction();
			let    textItem = ListMenuItemTextItem(item);
			string newPath  = textItem
				? makePath(path, StringTable.Localize(textItem.mText))
				: path;

			found |= listOptions(targetDesc, actionN, query, newPath, isAnyTermMatches, searchedMenus);
		}

		return found;
	}

	private static bool listOptionsOptionMenu(OptionMenuDescriptor sourceDesc,
	                                          OptionMenuDescriptor targetDesc,
	                                          os_Query query,
	                                          string   path,
	                                          bool     isAnyTermMatches,
	                                          Dictionary searchedMenus)
	{
		if (sourceDesc == targetDesc) { return false; }

		int  nItems = sourceDesc.mItems.size();
		bool first  = true;
		bool found  = false;

		for (int i = 0; i < nItems; ++i)
		{
			let item = sourceDesc.mItems[i];

			if (item is "OptionMenuItemStaticText") { continue; }

			string label = StringTable.Localize(item.mLabel);

			if (!query.matches(label, isAnyTermMatches)) { continue; }

			found = true;

			if (first)
			{
				addEmptyLine(targetDesc);
				addPathItem(targetDesc, path);

				first = false;
			}

			let itemOptionBase = OptionMenuItemOptionBase(item);

			if (itemOptionBase)
			{
				itemOptionBase.mCenter = false;
			}

			targetDesc.mItems.push(item);
		}

		for (int i = 0; i < nItems; ++i)
		{
			let    item              = sourceDesc.mItems[i];
			string label             = StringTable.Localize(item.mLabel);
			string optionSearchTitle = StringTable.Localize("$OS_TITLE");

			if (label == optionSearchTitle) { continue; }

			if (item is "OptionMenuItemSubMenu")
			{
				string newPath = makePath(path, label);

				found |= listOptions(targetDesc, item.GetAction(), query, newPath, isAnyTermMatches, searchedMenus);
			}
		}

		return found;
	}

	private static string makePath(string path, string label)
	{
		if (path.length() == 0) { return label; }

		int    pathWidth   = SmallFont.StringWidth(path .. "/" .. label);
		int    screenWidth = Screen.GetWidth();
		bool   isTooWide   = (pathWidth > screenWidth / 3);
		string newPath     = isTooWide
			? path .. "/" .. "\n" .. label
			: path .. "/" ..         label;

		return newPath;
	}

	private static void addPathItem(OptionMenuDescriptor desc, string path)
	{
		Array<String> lines;
		path.split(lines, "\n");

		int nLines = lines.size();

		for (int i = 0; i < nLines; ++i)
		{
			OptionMenuItemStaticText text = new("OptionMenuItemStaticText").Init(lines[i], 1);

			desc.mItems.push(text);
		}
	}

	private static void addEmptyLine(OptionMenuDescriptor desc)
	{
		int nItems = desc.mItems.size();

		if (nItems > 0)
		{
			let staticText = OptionMenuItemStaticText(desc.mItems[nItems - 1]);

			if (staticText != null && staticText.mLabel == "") { return; }
		}

		let item = new("OptionMenuItemStaticText").Init("");

		desc.mItems.push(item);
	}

	private static void addNoResultsItem(OptionMenuDescriptor desc)
	{
		string noResults = StringTable.Localize("$OS_NO_RESULTS");
		let    text      = new("OptionMenuItemStaticText").Init(noResults, 0);

		addEmptyLine(desc);
		desc.mItems.push(text);
	}

	private os_AnyOrAllOption mIsAnyOfItem;
	private os_SearchField    mSearchField;
}
