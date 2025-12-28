/*
** actions.zs
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

extend class Actor
{

	private void CheckStopped()
	{
		let player = self.player;
		if (player && player.mo == self && !(player.cheats & CF_PREDICTING) && Vel == (0, 0, 0))
		{
			player.mo.PlayIdle();
			player.Vel = (0, 0);
		}
	}

	//===========================================================================
	//
	// A_Stop
	// resets all velocity of the actor to 0
	//
	//===========================================================================
	void A_Stop()
	{
		let player = self.player;
		Vel = (0, 0, 0);
		CheckStopped();
	}

	//===========================================================================
	//
	// A_ScaleVelocity
	//
	// Scale actor's velocity.
	//
	//===========================================================================

	void A_ScaleVelocity(double scale, int ptr = AAPTR_DEFAULT)
	{

		let ref = GetPointer(ptr);

		if (ref == NULL)
		{
			return;
		}

		bool was_moving = ref.Vel != (0, 0, 0);

		ref.Vel *= scale;

		// If the actor was previously moving but now is not, and is a player,
		// update its player variables. (See A_Stop.)
		if (was_moving)
		{
			ref.CheckStopped();
		}
	}

	//===========================================================================
	//
	// A_ChangeVelocity
	//
	//===========================================================================

	void A_ChangeVelocity(double x = 0, double y = 0, double z = 0, int flags = 0, int ptr = AAPTR_DEFAULT)
	{
		let ref = GetPointer(ptr);

		if (ref == NULL)
		{
			return;
		}

		bool was_moving = ref.Vel != (0, 0, 0);

		let newvel = (x, y, z);
		double sina = sin(ref.Angle);
		double cosa = cos(ref.Angle);

		if (flags & 1)	// relative axes - make x, y relative to actor's current angle
		{
			newvel.X = x * cosa - y * sina;
			newvel.Y = x * sina + y * cosa;
		}
		if (flags & 2)	// discard old velocity - replace old velocity with new velocity
		{
			ref.Vel = newvel;
		}
		else	// add new velocity to old velocity
		{
			ref.Vel += newvel;
		}

		if (was_moving)
		{
			ref.CheckStopped();
		}
	}

	void A_SpriteOffset(double ox = 0.0, double oy = 0.0)
	{
		SpriteOffset.X = ox;
		SpriteOffset.Y = oy;
	}
}
