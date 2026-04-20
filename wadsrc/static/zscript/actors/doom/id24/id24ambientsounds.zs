/*
** id24ambientsounds.zs
**
** id1 - ambient sounds
**
**---------------------------------------------------------------------------
**
** Copyright 1993-2024 id Software LLC, a ZeniMax Media company.
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** converted from DECOHACK
*/

class ID24AmbientKlaxon : Actor
{
	Default
	{
		Radius 8;
		Height 16;
	}
	States
	{
	Spawn:
		TNT1 A 1 A_Look;
		Loop;
	See:
		TNT1 A 35 A_StartSound("ambient/klaxon");
		Loop;
	}
}

class ID24AmbientPortalOpen : ID24AmbientKlaxon
{
	States
	{
	See:
		TNT1 A 250 A_StartSound("ambient/portalopen", 1);
		Stop;
	}
}

class ID24AmbientPortalLoop : ID24AmbientKlaxon
{
	States
	{
	See:
		TNT1 A 139 A_StartSound("ambient/portalloop");
		Loop;
	}
}

class ID24AmbientPortalClose : ID24AmbientKlaxon
{
	States
	{
	See:
		TNT1 A 105 A_StartSound("ambient/portalclose", 1);
		Stop;
	}
}
