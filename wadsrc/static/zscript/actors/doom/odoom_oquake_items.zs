/*
 * ODOOM OQUAKE runtime actors for non-key OQUAKE thing IDs.
 * These classes let ODOOM spawn/use OQUAKE 5xxx/30xx monster and item types.
 */

class OQShotgun : Shotgun
{
	States { Spawn: OQW1 A -1; Stop; }
}

class OQSuperShotgun : SuperShotgun
{
	States { Spawn: OQW2 A -1; Stop; }
}

class OQNailgun : Chaingun
{
	States { Spawn: OQW3 A -1; Stop; }
}

class OQSuperNailgun : Chaingun
{
	States { Spawn: OQW4 A -1; Stop; }
}

class OQGrenadeLauncher : RocketLauncher
{
	States { Spawn: OQW5 A -1; Stop; }
}

class OQRocketLauncher : RocketLauncher
{
	States { Spawn: OQW6 A -1; Stop; }
}

class OQThunderbolt : PlasmaRifle
{
	States { Spawn: OQW7 A -1; Stop; }
}

class OQNails : Clip
{
	Default { Inventory.PickupMessage "Picked up Nails"; }
	States { Spawn: OQA1 A -1; Stop; }
}

class OQShells : Shell
{
	Default { Inventory.PickupMessage "Picked up Shells"; }
	States { Spawn: OQA2 A -1; Stop; }
}

class OQRockets : RocketAmmo
{
	Default { Inventory.PickupMessage "Picked up Rockets"; }
	States { Spawn: OQA3 A -1; Stop; }
}

class OQCells : Cell
{
	Default { Inventory.PickupMessage "Picked up Cells"; }
	States { Spawn: OQA4 A -1; Stop; }
}

class OQHealth : Medikit
{
	Default { Inventory.PickupMessage "Picked up Health"; }
	States { Spawn: OQH1 A -1; Stop; }
}

class OQHealthSmall : Stimpack
{
	Default { Inventory.PickupMessage "Picked up Small Health"; }
	States { Spawn: OQH2 A -1; Stop; }
}

class OQArmorGreen : GreenArmor
{
	Default { Inventory.PickupMessage "Picked up Green Armor"; }
	States { Spawn: OQH3 A -1; Stop; }
}

class OQArmorYellow : GreenArmor
{
	Default { Inventory.PickupMessage "Picked up Yellow Armor"; }
	States { Spawn: OQH4 A -1; Stop; }
}

class OQArmorMega : BlueArmor
{
	Default { Inventory.PickupMessage "Picked up Mega Armor"; }
	States { Spawn: OQH5 A -1; Stop; }
}

class OQMonsterDog : Demon
{
	Default
	{
		Tag "Rottweiler";
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_dog");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM1 A 10 A_Look;
		Loop;
	See:
		OQM1 B 4 A_Chase;
		OQM1 C 4 A_Chase;
		OQM1 D 4 A_Chase;
		OQM1 E 4 A_Chase;
		Loop;
	Melee:
		OQM1 F 8 A_FaceTarget;
		OQM1 G 8 A_SargAttack;
		OQM1 H 8;
		Goto See;
	Pain:
		OQM1 I 3 A_Pain;
		Goto See;
	Death:
		OQM1 J 5 A_Scream;
		OQM1 K 5 A_NoBlocking;
		OQM1 L 5;
		OQM1 M 5;
		OQM1 N -1;
		Stop;
	XDeath:
		OQM1 O 5 A_XScream;
		OQM1 P 5 A_NoBlocking;
		OQM1 Q 5;
		OQM1 R 5;
		OQM1 S 5;
		OQM1 T 5;
		OQM1 U 5;
		OQM1 V 5;
		OQM1 W -1;
		Stop;
	}
}

class OQMonsterZombie : ZombieMan
{
	Default
	{
		Tag "Zombie";
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_zombie");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM2 A 10 A_Look;
		Loop;
	See:
		OQM2 B 4 A_Chase;
		OQM2 C 4 A_Chase;
		OQM2 D 4 A_Chase;
		OQM2 E 4 A_Chase;
		Loop;
	Missile:
		OQM2 F 8 A_FaceTarget;
		OQM2 G 8 A_PosAttack;
		OQM2 H 8;
		Goto See;
	Pain:
		OQM2 I 3 A_Pain;
		Goto See;
	Death:
		OQM2 J 0 A_SetRoll(90);
		OQM2 J 5 A_Scream;
		OQM2 K 5 A_NoBlocking;
		OQM2 L 5;
		OQM2 M 5;
		OQM2 N -1;
		Stop;
	XDeath:
		OQM2 O 5 A_XScream;
		OQM2 P 5 A_NoBlocking;
		OQM2 Q 5;
		OQM2 R 5;
		OQM2 S 5;
		OQM2 T 5;
		OQM2 U 5;
		OQM2 V 5;
		OQM2 W -1;
		Stop;
	}
}

class OQMonsterDemon : Demon
{
	Default { Tag "Demon"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_demon");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM3 A 10 A_Look;
		Loop;
	See:
		OQM3 B 4 A_Chase;
		OQM3 C 4 A_Chase;
		OQM3 D 4 A_Chase;
		OQM3 E 4 A_Chase;
		Loop;
	Melee:
		OQM3 F 8 A_FaceTarget;
		OQM3 G 8 A_SargAttack;
		OQM3 H 8;
		Goto See;
	Pain:
		OQM3 I 3 A_Pain;
		Goto See;
	Death:
		OQM3 J 5 A_Scream;
		OQM3 K 5 A_NoBlocking;
		OQM3 L 5;
		OQM3 M 5;
		OQM3 N -1;
		Stop;
	XDeath:
		OQM3 O 5 A_XScream;
		OQM3 P 5 A_NoBlocking;
		OQM3 Q 5;
		OQM3 R 5;
		OQM3 S 5;
		OQM3 T 5;
		OQM3 U 5;
		OQM3 V 5;
		OQM3 W -1;
		Stop;
	}
}

class OQMonsterShambler : BaronOfHell
{
	Default { Tag "Shambler"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_shambler");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM4 A 10 A_Look;
		Loop;
	See:
		OQM4 B 4 A_Chase;
		OQM4 C 4 A_Chase;
		OQM4 D 4 A_Chase;
		OQM4 E 4 A_Chase;
		Loop;
	Melee:
		OQM4 F 8 A_FaceTarget;
		OQM4 G 8 A_BruisAttack;
		OQM4 H 8;
		Goto See;
	Missile:
		OQM4 F 8 A_FaceTarget;
		OQM4 G 8 A_BruisAttack;
		OQM4 H 8;
		Goto See;
	Pain:
		OQM4 I 3 A_Pain;
		Goto See;
	Death:
		OQM4 J 5 A_Scream;
		OQM4 K 5 A_NoBlocking;
		OQM4 L 5;
		OQM4 M 5;
		OQM4 N -1;
		Stop;
	XDeath:
		OQM4 O 5 A_XScream;
		OQM4 P 5 A_NoBlocking;
		OQM4 Q 5;
		OQM4 R 5;
		OQM4 S 5;
		OQM4 T 5;
		OQM4 U 5;
		OQM4 V 5;
		OQM4 W -1;
		Stop;
	}
}

class OQMonsterGrunt : ZombieMan
{
	Default
	{
		Tag "Grunt";
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_grunt");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM5 A 10 A_Look;
		Loop;
	See:
		OQM5 B 4 A_Chase;
		OQM5 C 4 A_Chase;
		OQM5 D 4 A_Chase;
		OQM5 E 4 A_Chase;
		Loop;
	Missile:
		OQM5 F 8 A_FaceTarget;
		OQM5 G 8 A_PosAttack;
		OQM5 H 8;
		Goto See;
	Pain:
		OQM5 I 3 A_Pain;
		Goto See;
	Death:
		OQM5 J 5 A_Scream;
		OQM5 K 0 A_NoBlocking;
		OQM5 K 0 A_Fall();
		OQM5 K 0 A_SetFloorClip();
		OQM5 K 5;
		OQM5 L 5;
		OQM5 M 5;
		OQM5 N -1;
		Stop;
	XDeath:
		OQM5 O 5 A_XScream;
		OQM5 P 5 A_NoBlocking;
		OQM5 Q 5;
		OQM5 R 5;
		OQM5 S 5;
		OQM5 T 5;
		OQM5 U 5;
		OQM5 V 5;
		OQM5 W -1;
		Stop;
	}
}

class OQMonsterFish : Cacodemon
{
	Default
	{
		Tag "Fish";
		-FLOAT;
		-INFLOAT;
		-NOGRAVITY;
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_fish");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM6 A 10 A_Look;
		Loop;
	See:
		OQM6 B 4 A_Chase;
		OQM6 C 4 A_Chase;
		OQM6 D 4 A_Chase;
		OQM6 E 4 A_Chase;
		Loop;
	Missile:
		OQM6 F 8 A_FaceTarget;
		OQM6 G 8 A_HeadAttack;
		OQM6 H 8;
		Goto See;
	Pain:
		OQM6 I 3 A_Pain;
		Goto See;
	Death:
		OQM6 J 5 A_Scream;
		OQM6 K 5 A_NoBlocking;
		OQM6 L 5;
		OQM6 M 5;
		OQM6 N -1;
		Stop;
	XDeath:
		OQM6 O 5 A_XScream;
		OQM6 P 5 A_NoBlocking;
		OQM6 Q 5;
		OQM6 R 5;
		OQM6 S 5;
		OQM6 T 5;
		OQM6 U 5;
		OQM6 V 5;
		OQM6 W -1;
		Stop;
	}
}

class OQMonsterOgre : ShotgunGuy
{
	Default { Tag "Ogre"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_ogre");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM7 A 10 A_Look;
		Loop;
	See:
		OQM7 B 4 A_Chase;
		OQM7 C 4 A_Chase;
		OQM7 D 4 A_Chase;
		OQM7 E 4 A_Chase;
		Loop;
	Missile:
		OQM7 F 10 A_FaceTarget;
		OQM7 G 10 A_SPosAttackUseAtkSound;
		OQM7 H 8;
		Goto See;
	Pain:
		OQM7 I 3 A_Pain;
		Goto See;
	Death:
		OQM7 J 5 A_Scream;
		OQM7 K 5 A_NoBlocking;
		OQM7 L 5;
		OQM7 M 5;
		OQM7 N -1;
		Stop;
	XDeath:
		OQM7 O 5 A_XScream;
		OQM7 P 5 A_NoBlocking;
		OQM7 Q 5;
		OQM7 R 5;
		OQM7 S 5;
		OQM7 T 5;
		OQM7 U 5;
		OQM7 V 5;
		OQM7 W -1;
		Stop;
	}
}

class OQMonsterEnforcer : ChaingunGuy
{
	Default { Tag "Enforcer"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_enforcer");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM8 A 10 A_Look;
		Loop;
	See:
		OQM8 B 4 A_Chase;
		OQM8 C 4 A_Chase;
		OQM8 D 4 A_Chase;
		OQM8 E 4 A_Chase;
		Loop;
	Missile:
		OQM8 F 10 A_FaceTarget;
		OQM8 G 4 BRIGHT A_CPosAttack;
		OQM8 H 1 A_CPosRefire;
		Goto Missile+1;
	Pain:
		OQM8 I 3 A_Pain;
		Goto See;
	Death:
		OQM8 J 5 A_Scream;
		OQM8 K 5 A_NoBlocking;
		OQM8 L 5;
		OQM8 M 5;
		OQM8 N -1;
		Stop;
	XDeath:
		OQM8 O 5 A_XScream;
		OQM8 P 5 A_NoBlocking;
		OQM8 Q 5;
		OQM8 R 5;
		OQM8 S 5;
		OQM8 T 5;
		OQM8 U 5;
		OQM8 V 5;
		OQM8 W -1;
		Stop;
	}
}

class OQMonsterSpawn : PainElemental
{
	Default
	{
		Tag "Spawn";
		-FLOAT;
		-INFLOAT;
		-NOGRAVITY;
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_spawn");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQM9 A 10 A_Look;
		Loop;
	See:
		OQM9 B 4 A_Chase;
		OQM9 C 4 A_Chase;
		OQM9 D 4 A_Chase;
		OQM9 E 4 A_Chase;
		Loop;
	Missile:
		OQM9 F 8 A_FaceTarget;
		OQM9 G 0 BRIGHT A_PainAttack;
		OQM9 H 8;
		Goto See;
	Pain:
		OQM9 I 3 A_Pain;
		Goto See;
	Death:
		OQM9 J 5 A_Scream;
		OQM9 K 5 A_NoBlocking;
		OQM9 L 5;
		OQM9 M 5;
		OQM9 N -1;
		Stop;
	XDeath:
		OQM9 O 5 A_XScream;
		OQM9 P 5 A_NoBlocking;
		OQM9 Q 5;
		OQM9 R 5;
		OQM9 S 5;
		OQM9 T 5;
		OQM9 U 5;
		OQM9 V 5;
		OQM9 W -1;
		Stop;
	}
}

class OQMonsterKnight : HellKnight
{
	Default
	{
		Tag "Hell Knight";
		Health 120;
	}
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		double s = GetCVar("odoom_oq_monster_scale_global") * GetCVar("odoom_oq_monster_scale_knight");
		if (s < 0.05) s = 0.05;
		Scale.X = s;
		Scale.Y = s;
	}
	States
	{
	Spawn:
		OQMA A 10 A_Look;
		Loop;
	See:
		OQMA B 4 A_Chase;
		OQMA C 4 A_Chase;
		OQMA D 4 A_Chase;
		OQMA E 4 A_Chase;
		Loop;
	Melee:
		OQMA F 8 A_FaceTarget;
		OQMA G 8 A_BruisAttack;
		OQMA H 8;
		Goto See;
	Missile:
		OQMA F 10 A_FaceTarget;
		OQMA G 0 A_CustomMissile("DoomImpBall", 28, 0, 0, CMF_AIMDIRECTION);
		OQMA G 12;
		OQMA H 12;
		Goto See;
	Pain:
		OQMA I 3 A_Pain;
		Goto See;
	Death:
		OQMA J 5 A_Scream;
		OQMA K 5 A_NoBlocking;
		OQMA L 5;
		OQMA M 5;
		OQMA N -1;
		Stop;
	XDeath:
		OQMA O 5 A_XScream;
		OQMA P 5 A_NoBlocking;
		OQMA Q 5;
		OQMA R 5;
		OQMA S 5;
		OQMA T 5;
		OQMA U 5;
		OQMA V 5;
		OQMA W -1;
		Stop;
	}
}

