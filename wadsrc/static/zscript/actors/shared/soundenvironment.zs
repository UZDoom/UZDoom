/*
** soundenvironment.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

class SoundEnvironment : Actor
{
	default
	{
		+NOSECTOR
		+NOBLOCKMAP
		+NOGRAVITY
		+DONTSPLASH
		+NOTONAUTOMAP
	}
	
	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		if (!bDormant)
		{
			Activate (self);
		}
	}

	override void Activate (Actor activator)
	{
		CurSector.SetEnvironmentID((args[0]<<8) | (args[1]));
	}

	// Deactivate just exists so that you can flag the thing as dormant in an editor
	// and not have it take effect. This is so you can use multiple environments in
	// a single zone, with only one set not-dormant, so you know which one will take
	// effect at the start.
	override void Deactivate (Actor deactivator)
	{
		bDormant = true;
	}
	
}
