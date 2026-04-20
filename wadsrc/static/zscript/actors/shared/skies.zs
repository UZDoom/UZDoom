/*
** skies.zs
**
** Skybox-related actors
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2006-2017 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

class SkyViewpoint : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
	}
	
	// arg0 = Visibility*4 for self skybox

	// If self actor has no TID, make it the default sky box
	override void BeginPlay ()
	{
		Super.BeginPlay ();

		if (tid == 0 && level.sectorPortals[0].mSkybox == null)
		{
			level.sectorPortals[0].mSkybox = self;
			level.sectorPortals[0].mDestination = CurSector;
		}
	}

	override void OnDestroy ()
	{
		// remove all sector references to ourselves.
		for (int i = 0; i < level.sectorPortals.Size(); i++)
		{
			SectorPortal s = level.sectorPortals[i];
			if (s.mSkybox == self)
			{
				s.mSkybox = null;
				// This is necessary to entirely disable EE-style skyboxes
				// if their viewpoint gets deleted.
				s.mFlags |= SectorPortal.FLAG_SKYFLATONLY;
			}
		}

		Super.OnDestroy();
	}
	
}

//---------------------------------------------------------------------------

// arg0 = tid of matching SkyViewpoint
// A value of 0 means to use a regular stretched texture, in case
// there is a default SkyViewpoint in the level.
//
// arg1 = 0: set both floor and ceiling skybox
//		= 1: set only ceiling skybox
//		= 2: set only floor skybox

class SkyPicker : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
	}
	
	override void PostBeginPlay ()
	{
		Actor box;
		Super.PostBeginPlay ();

		if (args[0] == 0)
		{
			box = null;
		}
		else
		{
			let it = Level.CreateActorIterator(args[0], "SkyViewpoint");
			box = it.Next ();
		}

		if (box == null && args[0] != 0)
		{
			A_Log(String.Format("Can't find SkyViewpoint %d for sector %d\n", args[0], CurSector.Index()));
		}
		else
		{
			int boxindex = level.GetSkyboxPortal(box);
			// Do not override special portal types, only regular skies.
			if (0 == (args[1] & 2))
			{
				if (CurSector.GetPortalType(sector.ceiling) == SectorPortal.TYPE_SKYVIEWPOINT)
					CurSector.Portals[sector.ceiling] = boxindex;
			}
			if (0 == (args[1] & 1))
			{
				if (CurSector.GetPortalType(sector.floor) == SectorPortal.TYPE_SKYVIEWPOINT)
					CurSector.Portals[sector.floor] = boxindex;
			}
		}
		Destroy ();
	}
	
}

class SkyCamCompat : SkyViewpoint
{
	override void BeginPlay ()
	{
		// Skip SkyViewpoint's initialization, Actor's is not needed here.
	}
}

class StackPoint : SkyViewpoint
{
	override void BeginPlay ()
	{
		// Skip SkyViewpoint's initialization, Actor's is not needed here.
	}

}

class UpperStackLookOnly : StackPoint
{
}

class LowerStackLookOnly : StackPoint
{
}


class SectorSilencer : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		RenderStyle "None";
	}
	
	override void BeginPlay ()
	{
		Super.BeginPlay ();
		CurSector.Flags |= Sector.SECF_SILENT;
	}

	override void OnDestroy ()
	{
		if (CurSector != null)
		{
			CurSector.Flags &= ~Sector.SECF_SILENT;
		}
		Super.OnDestroy();
	}
}
