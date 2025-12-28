/*
** secrettrigger.zs
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

class SecretTrigger : Actor
{
	default
	{
		+NOBLOCKMAP
		+NOSECTOR
		+NOGRAVITY
		+DONTSPLASH
		+NOTONAUTOMAP
	}

	override void PostBeginPlay ()
	{
		Super.PostBeginPlay ();
		level.total_secrets++;
	}

	override void Activate (Actor activator)
	{
		Level.GiveSecret(activator, args[0] <= 1, (args[0] == 0 || args[0] == 2));
		Destroy ();
	}

	
}

