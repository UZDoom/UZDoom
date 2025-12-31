/*
** chicken.zs
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

// Beak puff ----------------------------------------------------------------

class BeakPuff : StaffPuff
{
	Default
	{
		Mass 5;
		Renderstyle "Translucent";
		Alpha 0.4;
		AttackSound "chicken/attack";
		VSpeed 1;
	}
}

// Beak ---------------------------------------------------------------------

class Beak : Weapon
{
	Default
	{
		Weapon.SelectionOrder 10000;
		+WEAPON.DONTBOB
		+WEAPON.MELEEWEAPON
		Weapon.YAdjust 15;
		Weapon.SisterWeapon "BeakPowered";
	}
	

	States
	{
	Ready:
		BEAK A 1 A_WeaponReady;
		Loop;
	Deselect:
		BEAK A 1 A_Lower;
		Loop;
	Select:
		BEAK A 1 A_BeakRaise;
		Loop;
	Fire:
		BEAK A 18 A_BeakAttackPL1;
		Goto Ready;
	}
	
	//---------------------------------------------------------------------------
	//
	// PROC A_BeakRaise
	//
	//---------------------------------------------------------------------------

	action void A_BeakRaise ()
	{

		if (player == null)
		{
			return;
		}
		let psp = player.GetPSprite(PSP_WEAPON);
		if (psp)
		{
			psp.y = WEAPONTOP;
			ResetPSprite(psp);
		}
		player.SetPsprite(PSP_WEAPON, player.ReadyWeapon.GetReadyState());
	}

	//----------------------------------------------------------------------------
	//
	// PROC A_BeakAttackPL1
	//
	//----------------------------------------------------------------------------

	action void A_BeakAttackPL1()
	{
		FTranslatedLineTarget t;

		if (player == null)
		{
			return;
		}

		int damage = random[BeakAtk](1,3);
		double ang = angle;
		double slope = AimLineAttack (ang, DEFMELEERANGE);
		LineAttack (ang, DEFMELEERANGE, slope, damage, 'Melee', "BeakPuff", true, t);
		if (t.linetarget)
		{
			angle = t.angleFromSource;
		}
		A_StartSound ("chicken/peck", CHAN_VOICE);
		player.chickenPeck = 12;
		let psp = player.GetPSprite(PSP_WEAPON);
		if (psp)
		{
			psp.Tics -= random[BeakAtk](0,7);
		}
	}
}


// BeakPowered ---------------------------------------------------------------------

class BeakPowered : Beak
{
	Default
	{
		+WEAPON.POWERED_UP
		Weapon.SisterWeapon "Beak";
	}


	States
	{
	Fire:
		BEAK A 12 A_BeakAttackPL2;
		Goto Ready;
	}
	
	//----------------------------------------------------------------------------
	//
	// PROC A_BeakAttackPL2
	//
	//----------------------------------------------------------------------------

	action void A_BeakAttackPL2()
	{
		FTranslatedLineTarget t;

		if (player == null)
		{
			return;
		}

		int damage = random[BeakAtk](1,8) * 4;
		double ang = angle;
		double slope = AimLineAttack (ang, DEFMELEERANGE);
		LineAttack (ang, DEFMELEERANGE, slope, damage, 'Melee', "BeakPuff", true, t);
		if (t.linetarget)
		{
			angle = t.angleFromSource;
		}
		A_StartSound ("chicken/peck", CHAN_VOICE);
		player.chickenPeck = 12;
		let psp = player.GetPSprite(PSP_WEAPON);
		if (psp)
		{
			psp.Tics -= random[BeakAtk](0,3);
		}
	}
	
}

// Chicken player -----------------------------------------------------------

class ChickenPlayer : PlayerPawn
{
	Default
	{
		Health 30;
		ReactionTime 0;
		PainChance 255;
		Radius 16;
		Height 24;
		Speed 1;
		Gravity 0.125;
		+NOSKIN
		+PLAYERPAWN.CANSUPERMORPH
		PainSound "chicken/pain";
		DeathSound "chicken/death";
		Player.JumpZ 1;
		Player.Viewheight 21;
		Player.ForwardMove 1.22, 1.22;
		Player.SideMove 1.22, 1.22;
		Player.SpawnClass "Chicken";
		Player.SoundClass "Chicken";
		Player.DisplayName "Chicken";
		Player.MorphWeapon "Beak";
		-PICKUP
	}

	States
	{
	Spawn:
		CHKN A -1;
		Stop;
	See:
		CHKN ABAB 3;
		Loop;
	Melee:
	Missile:
		CHKN C 12;
		Goto Spawn;
	Pain:
		CHKN D 4 A_Feathers;
		CHKN C 4 A_Pain;
		Goto Spawn;
	Death:
		CHKN E 6 A_Scream;
		CHKN F 6 A_Feathers;
		CHKN G 6;
		CHKN H 6 A_NoBlocking;
		CHKN IJK 6;
		CHKN L -1;
		Stop;
	}
	
	//---------------------------------------------------------------------------
	//
	// PROC P_UpdateBeak
	//
	//---------------------------------------------------------------------------

	override void MorphPlayerThink ()
	{
		if (health > 0)
		{ // Handle beak movement
			PSprite pspr;
			if (player != null && (pspr = player.FindPSprite(PSP_WEAPON)) != null)
			{
				pspr.y = WEAPONTOP + player.chickenPeck / 2;
			}
		}
		if ((player.MorphTics ? player.MorphTics : Random[ChickenPlayerThink]()) & 15)
		{
			return;
		}
		if (Vel.X == 0 && Vel.Y == 0 && random[ChickenPlayerThink]() < 160)
		{ // Twitch view ang
			angle += Random2[ChickenPlayerThink]() * (360. / 256. / 32.);
		}
		if ((pos.z <= floorz) && (random[ChickenPlayerThink]() < 32))
		{ // Jump and noise
			Vel.Z += JumpZ;

			State painstate = FindState('Pain');
			if (painstate != null) SetState (painstate);
		}
		if (random[ChickenPlayerThink]() < 48)
		{ // Just noise
			A_StartSound ("chicken/active", CHAN_VOICE);
		}
	}

}



// Chicken (non-player) -----------------------------------------------------

class Chicken : MorphedMonster
{
	Default
	{
		Health 10;
		Radius 9;
		Height 22;
		Mass 40;
		Speed 4;
		Painchance 200;
		Monster;
		-COUNTKILL
		+WINDTHRUST
		+DONTMORPH
		+FLOORCLIP
		SeeSound "chicken/pain";
		AttackSound "chicken/attack";
		PainSound "chicken/pain";
		DeathSound "chicken/death";
		ActiveSound "chicken/active";
		Obituary "$OB_CHICKEN";
		Tag "$FN_CHICKEN";
	}
	States
	{
	Spawn:
		CHKN AB 10 A_Look;
		Loop;
	See:
		CHKN AB 3 A_Chase;
		Loop;
	Pain:
		CHKN D 5 A_Feathers;
		CHKN C 5 A_Pain;
		Goto See;
	Melee:
		CHKN A 8 A_FaceTarget;
		CHKN C 10 A_CustomMeleeAttack(random[ChicAttack](1,2));
		Goto See;
	Death:
		CHKN E 6 A_Scream;
		CHKN F 6 A_Feathers;
		CHKN G 6;
		CHKN H 6 A_NoBlocking;
		CHKN IJK 6;
		CHKN L -1;
		Stop;
	}
}		


// Feather ------------------------------------------------------------------

class Feather : Actor
{
	Default
	{
		Radius 2;
		Height 4;
		+MISSILE +DROPOFF
		+NOTELEPORT +CANNOTPUSH
		+WINDTHRUST +DONTSPLASH
		Gravity 0.125;
	}

	States
	{
	Spawn:
		CHKN MNOPQPON 3;
		Loop;
	Death:
		CHKN N 6;
		Stop;
	}
}

extend class Actor
{
	//----------------------------------------------------------------------------
	//
	// PROC A_Feathers
	// This is used by both the chicken player and monster and must be in the
	// common base class to be accessible by both
	//
	//----------------------------------------------------------------------------

	void A_Feathers()
	{
		int count;

		if (health > 0)
		{ // Pain
			count = random[Feathers]() < 32 ? 2 : 1;
		}
		else
		{ // Death
			count = 5 + (random[Feathers](0, 3));
		}
		for (int i = 0; i < count; i++)
		{
			Actor mo = Spawn("Feather", pos + (0, 0, 20), NO_REPLACE);
			if (mo != null)
			{
				mo.target = self;
				mo.Vel.X = Random2[Feathers]() / 256.;
				mo.Vel.Y = Random2[Feathers]() / 256.;
				mo.Vel.Z = 1. + random[Feathers]() / 128.;
				mo.SetState (mo.SpawnState + (random[Feathers](0, 7)));
			}
		}
	}

}
