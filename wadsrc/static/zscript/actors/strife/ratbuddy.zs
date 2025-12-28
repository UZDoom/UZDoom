/*
** ratbuddy.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Rogue Entertainment
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

class RatBuddy : Actor
{
	Default
	{
		Health 5;
		Speed 13;
		Radius 10;
		Height 16;
		+NOBLOOD +FLOORCLIP +CANPASS
		+ISMONSTER +INCOMBAT
		MinMissileChance 150;
		MaxStepHeight 16;
		MaxDropoffHeight 32;
		Tag "$TAG_RATBUDDY";
		SeeSound "rat/sight";
		DeathSound "rat/death";
		ActiveSound "rat/active";
	}
	States
	{
	Spawn:
		RATT A 10 A_Look;
		Loop;
	See:
		RATT AABB 4 A_Chase;
		Loop;
	Melee:
		RATT A 8 A_Wander;
		RATT B 4 A_Wander;
		Goto See;
	Death:
		MEAT Q 700;
		Stop;
	}
}

