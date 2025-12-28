/*
** types.zs
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

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
struct WBPlayerStruct native version("2.4")
{
	// Player stats, kills, collected items etc.
	native int			skills;
	native int			sitems;
	native int			ssecret;
	native int			stime;
	native int			frags[MAXPLAYERS];
	native int			fragcount;	// [RH] Cumulative frags for this player
}

struct WBStartStruct native version("2.4")
{
	native int			finished_ep;
	native int			next_ep;
	native @WBPlayerStruct plyr[MAXPLAYERS];

	native String		current;	// [RH] Name of map just finished
	native String		next;		// next level, [RH] actual map name
	native String		nextname;	// next level, printable name
	native String		thisname;	// this level, printable name
	native String		nextauthor;	// next level, printable name
	native String		thisauthor;	// this level, printable name

	native TextureID	LName0;
	native TextureID	LName1;

	native int			totalkills;
	native int			maxkills;
	native int			maxitems;
	native int			maxsecret;
	native int			maxfrags;

	// the par time and sucktime
	native int			partime;	// in tics
	native int			sucktime;	// in minutes

	// total time for the entire current game
	native int			totaltime;

	// index of this player in game
	native int			pnum;
}

