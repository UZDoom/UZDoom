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
	Default { Tag "Rottweiler"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM1 A 10 A_Look;
		Loop;
	See:
		OQM1 A 4 A_Chase;
		Loop;
	Melee:
		OQM1 A 8 A_FaceTarget;
		OQM1 A 8 A_SargAttack;
		Goto See;
	Pain:
		OQM1 A 3 A_Pain;
		Goto See;
	Death:
		OQM1 A 5 A_Scream;
		OQM1 A 5 A_NoBlocking;
		OQM1 A -1;
		Stop;
	XDeath:
		OQM1 A 5 A_XScream;
		OQM1 A 5 A_NoBlocking;
		OQM1 A -1;
		Stop;
	}
}

class OQMonsterZombie : ZombieMan
{
	Default { Tag "Zombie"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM2 A 10 A_Look;
		Loop;
	See:
		OQM2 A 4 A_Chase;
		Loop;
	Missile:
		OQM2 A 8 A_FaceTarget;
		OQM2 A 8 A_PosAttack;
		Goto See;
	Pain:
		OQM2 A 3 A_Pain;
		Goto See;
	Death:
		OQM2 A 5 A_Scream;
		OQM2 A 5 A_NoBlocking;
		OQM2 A -1;
		Stop;
	XDeath:
		OQM2 A 5 A_XScream;
		OQM2 A 5 A_NoBlocking;
		OQM2 A -1;
		Stop;
	}
}

class OQMonsterDemon : Demon
{
	Default { Tag "Demon"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM3 A 10 A_Look;
		Loop;
	See:
		OQM3 A 4 A_Chase;
		Loop;
	Melee:
		OQM3 A 8 A_FaceTarget;
		OQM3 A 8 A_SargAttack;
		Goto See;
	Pain:
		OQM3 A 3 A_Pain;
		Goto See;
	Death:
		OQM3 A 5 A_Scream;
		OQM3 A 5 A_NoBlocking;
		OQM3 A -1;
		Stop;
	XDeath:
		OQM3 A 5 A_XScream;
		OQM3 A 5 A_NoBlocking;
		OQM3 A -1;
		Stop;
	}
}

class OQMonsterShambler : BaronOfHell
{
	Default { Tag "Shambler"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM4 A 10 A_Look;
		Loop;
	See:
		OQM4 A 4 A_Chase;
		Loop;
	Melee:
		OQM4 A 8 A_FaceTarget;
		OQM4 A 8 A_BruisAttack;
		Goto See;
	Missile:
		OQM4 A 8 A_FaceTarget;
		OQM4 A 8 A_BruisAttack;
		Goto See;
	Pain:
		OQM4 A 3 A_Pain;
		Goto See;
	Death:
		OQM4 A 5 A_Scream;
		OQM4 A 5 A_NoBlocking;
		OQM4 A -1;
		Stop;
	XDeath:
		OQM4 A 5 A_XScream;
		OQM4 A 5 A_NoBlocking;
		OQM4 A -1;
		Stop;
	}
}

class OQMonsterGrunt : ZombieMan
{
	Default { Tag "Grunt"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM5 A 10 A_Look;
		Loop;
	See:
		OQM5 A 4 A_Chase;
		Loop;
	Missile:
		OQM5 A 8 A_FaceTarget;
		OQM5 A 8 A_PosAttack;
		Goto See;
	Pain:
		OQM5 A 3 A_Pain;
		Goto See;
	Death:
		OQM5 A 5 A_Scream;
		OQM5 A 5 A_NoBlocking;
		OQM5 A -1;
		Stop;
	XDeath:
		OQM5 A 5 A_XScream;
		OQM5 A 5 A_NoBlocking;
		OQM5 A -1;
		Stop;
	}
}

class OQMonsterFish : Cacodemon
{
	Default { Tag "Fish"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM6 A 10 A_Look;
		Loop;
	See:
		OQM6 A 4 A_Chase;
		Loop;
	Missile:
		OQM6 A 8 A_FaceTarget;
		OQM6 A 8 A_HeadAttack;
		Goto See;
	Pain:
		OQM6 A 3 A_Pain;
		Goto See;
	Death:
		OQM6 A 5 A_Scream;
		OQM6 A 5 A_NoBlocking;
		OQM6 A -1;
		Stop;
	XDeath:
		OQM6 A 5 A_XScream;
		OQM6 A 5 A_NoBlocking;
		OQM6 A -1;
		Stop;
	}
}

class OQMonsterOgre : ShotgunGuy
{
	Default { Tag "Ogre"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM7 A 10 A_Look;
		Loop;
	See:
		OQM7 A 4 A_Chase;
		Loop;
	Missile:
		OQM7 A 10 A_FaceTarget;
		OQM7 A 10 A_SPosAttackUseAtkSound;
		Goto See;
	Pain:
		OQM7 A 3 A_Pain;
		Goto See;
	Death:
		OQM7 A 5 A_Scream;
		OQM7 A 5 A_NoBlocking;
		OQM7 A -1;
		Stop;
	XDeath:
		OQM7 A 5 A_XScream;
		OQM7 A 5 A_NoBlocking;
		OQM7 A -1;
		Stop;
	}
}

class OQMonsterEnforcer : ChaingunGuy
{
	Default { Tag "Enforcer"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM8 A 10 A_Look;
		Loop;
	See:
		OQM8 A 4 A_Chase;
		Loop;
	Missile:
		OQM8 A 10 A_FaceTarget;
		OQM8 A 4 BRIGHT A_CPosAttack;
		OQM8 A 1 A_CPosRefire;
		Goto Missile+1;
	Pain:
		OQM8 A 3 A_Pain;
		Goto See;
	Death:
		OQM8 A 5 A_Scream;
		OQM8 A 5 A_NoBlocking;
		OQM8 A -1;
		Stop;
	XDeath:
		OQM8 A 5 A_XScream;
		OQM8 A 5 A_NoBlocking;
		OQM8 A -1;
		Stop;
	}
}

class OQMonsterSpawn : PainElemental
{
	Default { Tag "Spawn"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQM9 A 10 A_Look;
		Loop;
	See:
		OQM9 A 4 A_Chase;
		Loop;
	Missile:
		OQM9 A 8 A_FaceTarget;
		OQM9 A 0 BRIGHT A_PainAttack;
		Goto See;
	Pain:
		OQM9 A 3 A_Pain;
		Goto See;
	Death:
		OQM9 A 5 A_Scream;
		OQM9 A 5 A_NoBlocking;
		OQM9 A -1;
		Stop;
	XDeath:
		OQM9 A 5 A_XScream;
		OQM9 A 5 A_NoBlocking;
		OQM9 A -1;
		Stop;
	}
}

class OQMonsterKnight : HellKnight
{
	Default { Tag "Hell Knight"; }
	override void PostBeginPlay()
	{
		Super.PostBeginPlay();
		SpriteOffset.X = 0;
		SpriteOffset.Y = -48;
	}
	States
	{
	Spawn:
		OQMA A 10 A_Look;
		Loop;
	See:
		OQMA A 4 A_Chase;
		Loop;
	Melee:
		OQMA A 8 A_FaceTarget;
		OQMA A 8 A_BruisAttack;
		Goto See;
	Missile:
		OQMA A 8 A_FaceTarget;
		OQMA A 8 A_BruisAttack;
		Goto See;
	Pain:
		OQMA A 3 A_Pain;
		Goto See;
	Death:
		OQMA A 5 A_Scream;
		OQMA A 5 A_NoBlocking;
		OQMA A -1;
		Stop;
	XDeath:
		OQMA A 5 A_XScream;
		OQMA A 5 A_NoBlocking;
		OQMA A -1;
		Stop;
	}
}

