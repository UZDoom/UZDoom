/*
** sbarinfowrapper.zs
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
*/

struct SBarInfo native ui
{
	native void Destroy();
	native void AttachToPlayer(PlayerInfo player);
	native void Draw(int state);
	native void NewGame();
	native bool MustDrawLog(int state);
	native void Tick();
	native void ShowPop(int popnum);
	native int GetProtrusion(double scalefac);
}


// The sole purpose of this wrapper is to eliminate the native dependencies of the status bar object
// because those would seriously impede the script conversion of the base class.

class SBarInfoWrapper : BaseStatusBar
{
	private clearscope SBarInfo core;

	override void OnDestroy()
	{
		if (core != null) core.Destroy();	// note that the core is not a GC'd object!
		Super.OnDestroy();
	}

	override void AttachToPlayer(PlayerInfo player)
	{
		Super.AttachToPlayer(player);
		core.AttachToPlayer(player);
	}

	override void Draw(int state, double TicFrac)
	{
		Super.Draw(state, TicFrac);
		core.Draw(state);
	}

	override void NewGame()
	{
		Super.NewGame();
		if (CPlayer != NULL)
		{
			core.NewGame();
		}
	}

	override bool MustDrawLog(int state)
	{
		return core.MustDrawLog(state);
	}

	override void Tick()
	{
		Super.Tick();
		core.Tick();
	}

	override void ShowPop(int popnum)
	{
		Super.ShowPop(popnum);
		core.ShowPop(popnum);
	}

	override int GetProtrusion(double scaleratio) const
	{
		return core.GetProtrusion(scaleratio);
	}


}
