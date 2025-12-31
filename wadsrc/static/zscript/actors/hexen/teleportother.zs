/*
** teleportother.zs
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
*/

// Teleport Other Artifact --------------------------------------------------

class ArtiTeleportOther : Inventory
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		+WEAPONSPAWN
		+INVENTORY.INVBAR
		+INVENTORY.FANCYPICKUPSOUND
		Inventory.PickupFlash "PickupFlash";
		Inventory.DefMaxAmount;
		Inventory.Icon "ARTITELO";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTITELEPORTOTHER";
		Tag "$TAG_ARTITELEPORTOTHER";
	}
	States
	{
	Spawn:
		TELO ABCD 5;
		Loop;
	}
	
	//===========================================================================
	//
	// Activate Teleport Other
	//
	//===========================================================================

	override bool Use (bool pickup)
	{
		Owner.SpawnPlayerMissile ("TelOtherFX1");
		return true;
	}

	
}


// Teleport Other FX --------------------------------------------------------

class TelOtherFX1 : Actor
{
	const TELEPORT_LIFE = 1;

	Default
	{
		Damage 10001;
		Projectile;
		-ACTIVATEIMPACT
		-ACTIVATEPCROSS
		+BLOODLESSIMPACT
		Radius 16;
		Height 16;
		Speed 20;
	}


	States
	{
	Spawn:
		TRNG E 5 Bright;
		TRNG D 4 Bright;
		TRNG C 3 Bright A_TeloSpawnC;
		TRNG B 3 Bright A_TeloSpawnB;
		TRNG A 3 Bright A_TeloSpawnA;
		TRNG B 3 Bright A_TeloSpawnB;
		TRNG C 3 Bright A_TeloSpawnC;
		TRNG D 3 Bright A_TeloSpawnD;
		Goto Spawn+2;
	Death:
		TRNG E 3 Bright;
		Stop;
	}
		
	private void TeloSpawn (class<Actor> type)
	{
		Actor fx = Spawn (type, pos, ALLOW_REPLACE);
		if (fx)
		{
			fx.special1 = TELEPORT_LIFE;			// Lifetime countdown
			fx.angle = angle;
			fx.target = target;
			fx.Vel = Vel / 2;
		}
	}

	void A_TeloSpawnA()
	{
		TeloSpawn ("TelOtherFX2");
	}

	void A_TeloSpawnB()
	{
		TeloSpawn ("TelOtherFX3");
	}

	void A_TeloSpawnC()
	{
		TeloSpawn ("TelOtherFX4");
	}

	void A_TeloSpawnD()
	{
		TeloSpawn ("TelOtherFX5");
	}

	void A_CheckTeleRing ()
	{
		if (self.special1-- <= 0)
		{
			self.SetStateLabel("Death");
		}
	}

	//===========================================================================
	//
	// Perform Teleport Other
	//
	//===========================================================================

	override int DoSpecialDamage (Actor target, int damage, Name damagetype)
	{
		if ((target.bIsMonster || target.player != NULL) &&
			!target.bBoss && !target.bNoTeleOther)
		{
			if (target.player)
			{
				if (deathmatch)
					P_TeleportToDeathmatchStarts (target);
				else
					P_TeleportToPlayerStarts (target);
			}
			else
			{
				// If death action, run it upon teleport
				if (target.bIsMonster && target.special)
				{
					target.RemoveFromHash ();
					Actor caller = level.ActOwnSpecial? target : self.target;
					caller.A_CallSpecial(target.special, target.args[0], target.args[1], target.args[2], target.args[3], target.args[4]);
					target.special = 0;
				}

				// Send all monsters to deathmatch spots
				P_TeleportToDeathmatchStarts (target);
			}
		}
		return -1;
	}

	//===========================================================================
	//
	// P_TeleportToPlayerStarts
	//
	//===========================================================================

	private void P_TeleportToPlayerStarts (Actor victim)
	{
		Vector3 dest;
		double destAngle;

		[dest, destAngle] = level.PickPlayerStart(0, PPS_FORCERANDOM | PPS_NOBLOCKINGCHECK);
		if (!level.useplayerstartz)
			dest.Z = ONFLOORZ;
		victim.Teleport (dest, destangle, TELF_SOURCEFOG | TELF_DESTFOG);
	}

	//===========================================================================
	//
	// P_TeleportToDeathmatchStarts
	//
	//===========================================================================

	private void P_TeleportToDeathmatchStarts (Actor victim)
	{
		Vector3 dest;
		double destAngle;

		[dest, destAngle] = level.PickDeathmatchStart();
		if (destAngle < 65536) victim.Teleport((dest.xy, ONFLOORZ), destangle, TELF_SOURCEFOG | TELF_DESTFOG);
		else P_TeleportToPlayerStarts(victim);
	}
	
}


class TelOtherFX2 : TelOtherFX1
{
	Default
	{
		Speed 16;
	}
	States
	{
	Spawn:
		TRNG BCDCB 4 Bright;
		TRNG A 4 Bright A_CheckTeleRing;
		Loop;
	}
}

class TelOtherFX3 : TelOtherFX1
{
	Default
	{
		Speed 16;
	}
	States
	{
	Spawn:
		TRNG CDCBA 4 Bright;
		TRNG B 4 Bright A_CheckTeleRing;
		Loop;
	}
}

class TelOtherFX4 : TelOtherFX1
{
	Default
	{
		Speed 16;
	}
	States
	{
	Spawn:
		TRNG DCBAB 4 Bright;
		TRNG C 4 Bright A_CheckTeleRing;
		Loop;
	}

}

class TelOtherFX5 : TelOtherFX1
{
	Default
	{
		Speed 16;
	}
	States
	{
	Spawn:
		TRNG CBABC 4 Bright;
		TRNG D 4 Bright A_CheckTeleRing;
		Loop;
	}
}


