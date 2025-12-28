/*
** merchants.zs
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

// Base class for the merchants ---------------------------------------------

class Merchant : Actor
{
	Default
	{
		Health 10000000;
		PainChance 256;	// a merchant should always enter the pain state when getting hurt
		Radius 20;
		Height 56;
		Mass 5000;
		CrushPainSound "misc/pcrush";
		+SOLID
		+SHOOTABLE
		+NOTDMATCH
		+NOSPLASHALERT
		+NODAMAGE
	}
	States
	{
	Spawn:
		MRST A 10 A_Look2;
		Loop;
		MRLK A 30 A_ActiveSound;
		Loop;
		MRLK B 30;
		Loop;
		MRBD ABCDEDCB 4;
		MRBD A 5;
		MRBD F 6;
		Loop;
	See:
	Pain:
		MRPN A 1;
		MRPN A 2 A_AlertMonsters;
		MRPN B 3 A_Pain;
		MRPN C 3;
		MRPN D 9 Door_CloseWaitOpen(999, 64, 960);
		MRPN C 4;
		MRPN B 3;
		MRPN A 3 A_ClearSoundTarget;
		Goto Spawn;
	Yes:
		MRYS A 20;
		// Fall through	
	Greetings:
		MRGT ABCDEFGHI 5;
		Goto Spawn;
	No:
		MRNO AB 6;
		MRNO C 10;
		MRNO BA 6;
		Goto Greetings;
	}
}


// Weapon Smith -------------------------------------------------------------

class WeaponSmith : Merchant
{
	Default
	{
		PainSound "smith/pain";
		Tag "$TAG_WEAPONSMITH";
	}
}


// Bar Keep -----------------------------------------------------------------

class BarKeep : Merchant
{
	Default
	{
		Translation 4;
		PainSound "barkeep/pain";
		ActiveSound "barkeep/active";
		Tag "$TAG_BARKEEP";
	}
}


// Armorer ------------------------------------------------------------------

class Armorer : Merchant
{
	Default
	{
		Translation 5;
		PainSound "armorer/pain";
		Tag "$TAG_ARMORER";
	}
}


// Medic --------------------------------------------------------------------

class Medic : Merchant
{
	Default
	{
		Translation 6;
		PainSound "medic/pain";
		Tag "$TAG_MEDIC";
	}
}

