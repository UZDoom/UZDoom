/*
** magecone.zs
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

// The Mage's Frost Cone ----------------------------------------------------

class MWeapFrost : MageWeapon
{
	Default
	{
		+BLOODSPLATTER
		Weapon.SelectionOrder 1700;
		Weapon.AmmoUse1 3;
		Weapon.AmmoGive1 25;
		Weapon.KickBack 150;
		Weapon.YAdjust 20;
		Weapon.AmmoType1 "Mana1";
		Inventory.PickupMessage "$TXT_WEAPON_M2";
		Obituary "$OB_MPMWEAPFROST";
		Tag "$TAG_MWEAPFROST";
	}

	States
	{
	Spawn:
		WMCS ABC 8 Bright;
		Loop;
	Select:
		CONE A 1 A_Raise;
		Loop;
	Deselect:
		CONE A 1 A_Lower;
		Loop;
	Ready:
		CONE A 1 A_WeaponReady;
		Loop;
	Fire:
		CONE B 3;
		CONE C 4;
	Hold:
		CONE D 3;
		CONE E 5;
		CONE F 3 A_FireConePL1;
		CONE G 3;
		CONE A 9;
		CONE A 10 A_ReFire;
		Goto Ready;
	}
	
	//============================================================================
	//
	// A_FireConePL1
	//
	//============================================================================

	action void A_FireConePL1()
	{
		bool conedone=false;
		FTranslatedLineTarget t;

		if (player == null)
		{
			return;
		}

		Weapon weapon = player.ReadyWeapon;
		if (weapon != null)
		{
			if (!weapon.DepleteAmmo (weapon.bAltFire))
				return;
		}
		A_StartSound ("MageShardsFire", CHAN_WEAPON);

		int damage = random[MageCone](90, 105);
		for (int i = 0; i < 16; i++)
		{
			double ang = angle + i*(45./16);
			double slope = AimLineAttack (ang, DEFMELEERANGE, t, 0., ALF_CHECK3D);
			if (t.linetarget)
			{
				t.linetarget.DamageMobj (self, self, damage, 'Ice', DMG_USEANGLE, t.angleFromSource);
				conedone = true;
				break;
			}
		}

		// didn't find any creatures, so fire projectiles
		if (!conedone)
		{
			Actor mo = SpawnPlayerMissile ("FrostMissile");
			if (mo)
			{
				mo.special1 = FrostMissile.SHARDSPAWN_LEFT|FrostMissile.SHARDSPAWN_DOWN|FrostMissile.SHARDSPAWN_UP|FrostMissile.SHARDSPAWN_RIGHT;
				mo.special2 = 3; // Set sperm count (levels of reproductivity)
				mo.target = self;
				mo.args[0] = 3;		// Mark Initial shard as super damage
			}
		}
	}
}

// Frost Missile ------------------------------------------------------------

class FrostMissile : Actor
{
	const SHARDSPAWN_LEFT	= 1;
	const SHARDSPAWN_RIGHT	= 2;
	const SHARDSPAWN_UP		= 4;
	const SHARDSPAWN_DOWN	= 8;
	
	Default
	{
		Speed 25;
		Radius 13;
		Height 8;
		Damage 1;
		DamageType "Ice";
		Projectile;
		DeathSound "MageShardsExplode";
		Obituary "$OB_MPMWEAPFROST";
	}

	States
	{
	Spawn:
		SHRD A 2 Bright;
		SHRD A 3 Bright A_ShedShard;
		SHRD B 3 Bright;
		SHRD C 3 Bright;
		Loop;
	Death:
		SHEX ABCDE 5 Bright;
		Stop;
	}
	
	override int DoSpecialDamage (Actor victim, int damage, Name damagetype)
	{
		if (special2 > 0)
		{
			damage <<= special2;
		}
		return damage;
	}

	//============================================================================
	//
	// A_ShedShard
	//
	//============================================================================

	void A_ShedShard()
	{
		int spawndir = special1;
		int spermcount = special2;
		Actor mo;

		if (spermcount <= 0)
		{
			return;				// No sperm left
		}
		special2 = 0;
		spermcount--;

		// every so many calls, spawn a new missile in its set directions
		if (spawndir & SHARDSPAWN_LEFT)
		{
			mo = SpawnMissileAngleZSpeed(pos.z, "FrostMissile", angle + 5, 0, (20. + 2 * spermcount), target);
			if (mo)
			{
				mo.special1 = SHARDSPAWN_LEFT;
				mo.special2 = spermcount;
				mo.Vel.Z = Vel.Z;
				mo.args[0] = (spermcount==3)?2:0;
			}
		}
		if (spawndir & SHARDSPAWN_RIGHT)
		{
			mo = SpawnMissileAngleZSpeed(pos.z, "FrostMissile",	angle - 5, 0, (20. + 2 * spermcount), target);
			if (mo)
			{
				mo.special1 = SHARDSPAWN_RIGHT;
				mo.special2 = spermcount;
				mo.Vel.Z = Vel.Z;
				mo.args[0] = (spermcount==3)?2:0;
			}
		}
		if (spawndir & SHARDSPAWN_UP)
		{
			mo = SpawnMissileAngleZSpeed(pos.z + 8., "FrostMissile", angle, 0, (15. + 2 * spermcount), target);
			if (mo)
			{
				mo.Vel.Z = Vel.Z;
				if (spermcount & 1)			// Every other reproduction
					mo.special1 = SHARDSPAWN_UP | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
				else
					mo.special1 = SHARDSPAWN_UP;
				mo.special2 = spermcount;
				mo.args[0] = (spermcount==3)?2:0;
			}
		}
		if (spawndir & SHARDSPAWN_DOWN)
		{
			mo = SpawnMissileAngleZSpeed(pos.z - 4., "FrostMissile", angle, 0, (15. + 2 * spermcount), target);
			if (mo)
			{
				mo.Vel.Z = Vel.Z;
				if (spermcount & 1)			// Every other reproduction
					mo.special1 = SHARDSPAWN_DOWN | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
				else
					mo.special1 = SHARDSPAWN_DOWN;
				mo.special2 = spermcount;
				mo.target = target;
				mo.args[0] = (spermcount==3)?2:0;
			}
		}
	}
}

// Ice Shard ----------------------------------------------------------------

class IceShard : FrostMissile
{
	Default
	{
		DamageType "Ice";
		-ACTIVATEIMPACT
		-ACTIVATEPCROSS
	}
	States
	{
	Spawn:
		SHRD ABC 3 Bright;
		Loop;
	}
}
