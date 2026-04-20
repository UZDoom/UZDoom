/*
** strifefunctions.zs
**
** common Strife action functions that are used by multiple different actors
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Rogue Entertainment
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

extend class Actor
{

	//============================================================================

	void A_FLoopActiveSound()
	{
		if (ActiveSound != 0 && !(Level.maptime & 7))
		{
			A_StartSound (ActiveSound, CHAN_VOICE);
		}
	}

	void A_LoopActiveSound()
	{
		A_StartSound(ActiveSound, CHAN_VOICE, CHANF_LOOPING);
	}

	//============================================================================
	//
	// 
	//
	//============================================================================

	void A_Countdown()
	{
		if (--reactiontime <= 0)
		{
			ExplodeMissile ();
			bSkullFly = false;
		}
	}

	//============================================================================
	//
	// A_ClearSoundTarget
	//
	//============================================================================

	void A_ClearSoundTarget()
	{
		CurSector.SoundTarget = null;
		for (Actor mo = CurSector.thinglist; mo != null; mo = mo.snext)
		{
			mo.LastHeard = null;
		}
	}
	
	//==========================================================================
	//
	// A_TossGib
	//
	//==========================================================================

	Actor A_TossGib(class<Actor> gibtype = null, double zOfs = 24.0)
	{
		if (!gibtype)
		{
			if (bNoBlood) gibtype = "Junk";
			else gibtype = "Meat";
		}
		Actor gib = Spawn (gibtype, pos.PlusZ(zOfs), ALLOW_REPLACE);

		if (gib == null)
		{
			return null;
		}

		gib.Angle = random[GibTosser]() * (360 / 256.);
		gib.VelFromAngle(random[GibTosser](0, 15));
		gib.Vel.Z = random[GibTosser](0, 15);
		return gib;
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_ShootGun()
	{
		if (!target) return;
		A_StartSound ("monsters/rifle", CHAN_WEAPON);
		A_FaceTarget ();
		double pitch = AimLineAttack (angle, MISSILERANGE);
		LineAttack (Angle + Random2[ShootGun]() * (11.25 / 256), MISSILERANGE, pitch, 3*random[ShootGun](1, 5), 'Hitscan', "StrifePuff");
	}
	
	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_SetShadow()
	{
		bShadow = true;
		A_SetRenderStyle(HR_SHADOW, STYLE_Translucent);
	}

	void A_ClearShadow()
	{
		bShadow = false;
		A_SetRenderStyle(1, STYLE_Normal);
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_GetHurt()
	{
		bInCombat = true;
		if (random[HurtMe](0, 4) == 0)
		{
			A_StartSound (PainSound, CHAN_VOICE);
			health--;
		}
		if (health <= 0)
		{
			Die (target, target);
		}
	}
	
	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_DropFire()
	{
		Actor drop = Spawn("FireDroplet", pos + (0,0,24), ALLOW_REPLACE);
		if (drop != null)
		{
			drop.Vel.Z = -1.;
		}
		A_Explode(64, 64, XF_NOSPLASH|XF_HURTSOURCE|XF_NOTMISSILE, damagetype: 'Fire');
	}	

	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_RemoveForceField()
	{
		bSpecial = false;
		CurSector.RemoveForceField();
	}
	
	//==========================================================================
	//
	//
	//
	//==========================================================================

	void A_AlertMonsters(double maxdist = 0, int flags = 0)
	{
		Actor target = null;
		Actor emitter = self;

		if (player != null || (Flags & AMF_TARGETEMITTER))
		{
			target = self;
		}
		else if (self.target != null && (self.target.player != null || (Flags & AMF_TARGETNONPLAYER)))
		{
			target = self.target;
		}

		if (Flags & AMF_EMITFROMTARGET) emitter = target;

		if (target != null && emitter != null)
		{
			emitter.SoundAlert(target, false, maxdist);
		}
	}

	//============================================================================
	//
	// A_RocketInFlight
	//
	//============================================================================

	void A_RocketInFlight()
	{
		A_StartSound ("misc/missileinflight", CHAN_VOICE);
		SpawnPuff ("MiniMissilePuff", Pos, Angle - 180, Angle - 180, 2, PF_HITTHING);
		Actor trail = Spawn("RocketTrail", Vec3Offset(-Vel.X, -Vel.Y, 0.), ALLOW_REPLACE);
		if (trail != null)
		{
			trail.Vel.Z = 1;
		}
	}

	
}
