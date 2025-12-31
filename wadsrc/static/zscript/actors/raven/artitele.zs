/*
** artitele.zs
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
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// Teleport (self) ----------------------------------------------------------

class ArtiTeleport : Inventory
{
	Default
	{
		+COUNTITEM
		+FLOATBOB
		+INVENTORY.INVBAR
		Inventory.PickupFlash "PickupFlash";
		+INVENTORY.FANCYPICKUPSOUND
		Inventory.DefMaxAmount;
		Inventory.Icon "ARTIATLP";
		Inventory.PickupSound "misc/p_pkup";
		Inventory.PickupMessage "$TXT_ARTITELEPORT";
		Tag "$TAG_ARTITELEPORT";
	}
	States
	{
	Spawn:
		ATLP ABCB 4;
		Loop;
	}
	
	override bool Use(bool pickup)
	{
		Vector3 dest;
		double destAngle;
		if (deathmatch)
			[dest, destAngle] = Level.PickDeathmatchStart();
		else
			[dest, destAngle] = Level.PickPlayerStart(Owner.PlayerNumber());

		if (!Level.UsePlayerStartZ)
			dest.Z = ONFLOORZ;

		Owner.Teleport(dest, destAngle, TELF_SOURCEFOG | TELF_DESTFOG);

		bool canLaugh = Owner.player != null;
		EMorphFlags mStyle = Owner.GetMorphStyle();
		if (Owner.Alternative && (mStyle & MRF_UNDOBYCHAOSDEVICE))
		{
			// Teleporting away will undo any morph effects (pig).
			if (!Owner.Unmorph(Owner, MRF_UNDOBYCHAOSDEVICE) && (mStyle & MRF_FAILNOLAUGH))
				canLaugh = false;
		}

		if (canLaugh)
			Owner.A_StartSound("*evillaugh", CHAN_VOICE, attenuation: ATTN_NONE);

		return true;
	}
	
}


