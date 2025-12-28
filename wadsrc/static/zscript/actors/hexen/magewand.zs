/*
** magewand.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
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

// The Mage's Wand ----------------------------------------------------------

class MWeapWand : MageWeapon
{
	Default
	{
		Weapon.SelectionOrder 3600;
		Weapon.KickBack 0;
		Weapon.YAdjust 9;
		Tag "$TAG_MWEAPWAND";
	}
	States
	{
	Select:
		MWND A 1 A_Raise;
		Loop;
	Deselect:
		MWND A 1 A_Lower;
		Loop;
	Ready:
		MWND A 1 A_WeaponReady;
		Loop;
	Fire:
		MWND A 6;
		MWND B 6 Bright Offset (0, 48) A_FireProjectile ("MageWandMissile");
		MWND A 3 Offset (0, 40);
		MWND A 3 Offset (0, 36) A_ReFire;
		Goto Ready;
	}
}

// Wand Smoke ---------------------------------------------------------------

class MageWandSmoke : Actor
{
	Default
	{
	+NOBLOCKMAP +NOGRAVITY +SHADOW
	+NOTELEPORT +CANNOTPUSH +NODAMAGETHRUST
	RenderStyle "Translucent";
	Alpha 0.6;
	}
	States
	{
	Spawn:
		MWND CDCD 4;
		Stop;
	}
}

// Wand Missile -------------------------------------------------------------

class MageWandMissile : FastProjectile
{
	Default
	{
	Speed 184;
	Radius 12;
	Height 8;
	Damage 2;
	+RIPPER +CANNOTPUSH +NODAMAGETHRUST
	+SPAWNSOUNDSOURCE
	MissileType "MageWandSmoke";
	SeeSound "MageWandFire";
	Obituary "$OB_MPMWEAPWAND";
	}
	States
	{
	Spawn:
		MWND CD 4 Bright;
		Loop;
	Death:
		MWND E 4 Bright;
		MWND F 3 Bright;
		MWND G 4 Bright;
		MWND H 3 Bright;
		MWND I 4 Bright;
		Stop;
	}
}
