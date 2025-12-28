/*
** weapondagger.zs
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

// Punch Dagger -------------------------------------------------------------

class PunchDagger : StrifeWeapon
{
	Default
	{
		Weapon.SelectionOrder 3900;
		+WEAPON.NOALERT
		Obituary "$OB_MPPUNCHDAGGER";
		Tag "$TAG_PUNCHDAGGER";
	}
	
	States
	{
	Ready:
		PNCH A 1 A_WeaponReady;
		Loop;
	Deselect:
		PNCH A 1 A_Lower;
		Loop;
	Select:
		PNCH A 1 A_Raise;
		Loop;
	Fire:
		PNCH B 4;
		PNCH C 4 A_JabDagger;
		PNCH D 5;
		PNCH C 4;
		PNCH B 5 A_ReFire;
		Goto Ready;
	}

	//============================================================================
	//
	// A_JabDagger
	//
	//============================================================================

	action void A_JabDagger ()
	{
		FTranslatedLineTarget t;
		int damage;
		
		if (FindInventory("SVETalismanPowerup"))
		{
			damage = 1000;
		}
		else
		{
			int power = MIN(10, stamina / 10);
			damage = random[JabDagger](0, power + 7) * (power + 2);

			if (FindInventory("PowerStrength"))
			{
				damage *= 10;
			}
		}

		double angle = angle + random2[JabDagger]() * (5.625 / 256);
		double pitch = AimLineAttack (angle, 80.);
		LineAttack (angle, 80., pitch, damage, 'Melee', "StrifeSpark", true, t);

		// turn to face target
		if (t.linetarget)
		{
			A_StartSound (t.linetarget.bNoBlood ? sound("misc/metalhit") : sound("misc/meathit"), CHAN_WEAPON);
			angle = t.angleFromSource;
			bJustAttacked = true;
			t.linetarget.DaggerAlert (self);
		}
		else
		{
			A_StartSound ("misc/swish", CHAN_WEAPON);
		}
	}
}	
