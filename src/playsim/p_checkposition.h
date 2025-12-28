/*
** p_checkposition.h
**
**
**
**---------------------------------------------------------------------------
**
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

#ifndef P_CHECKPOS_H
#define P_CHECKPOS_H

//============================================================================
//
// Used by P_CheckPosition and P_TryMove in place of the original
// set of global variables.
//
//============================================================================

struct FCheckPosition
{
	// in
	AActor			*thing;
	DVector3		pos;

	// out
	sector_t		*sector;
	double			floorz;
	double			ceilingz;
	double			dropoffz;
	FTextureID		floorpic;
	int				floorterrain;
	sector_t		*floorsector;
	FTextureID		ceilingpic;
	sector_t		*ceilingsector;
	bool			touchmidtex;
	bool			abovemidtex;
	bool			floatok;
	bool			FromPMove;
	line_t			*ceilingline;
	AActor			*stepthing;
	// [RH] These are used by PIT_CheckThing and P_XYMovement to apply
	// ripping damage once per tic instead of once per move.
	TMap<AActor*, bool> LastRipped;
	bool			DoRipping;
	bool			portalstep;
	bool			dropoffisportal;
	int				portalgroup;

	int				PushTime;

	FCheckPosition(bool rip=false)
	{
		DoRipping = rip;
		PushTime = 0;
		FromPMove = false;
	}
};


#endif
