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
