/*
** a_plats.h
**
** Plats (i.e. elevator platforms) code, raising/lowering.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
** Copyright 1998-1998 Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
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

#pragma once

//
// P_PLATS
//
class DPlat : public DMovingFloor
{
	DECLARE_CLASS (DPlat, DMovingFloor)
public:
	enum EPlatState
	{
		up,
		down,
		waiting,
		in_stasis
	};

	enum EPlatType
	{
		platPerpetualRaise,
		platDownWaitUpStay,
		platDownWaitUpStayStone,
		platUpWaitDownStay,
		platUpNearestWaitDownStay,
		platDownByValue,
		platUpByValue,
		platUpByValueStay,
		platRaiseAndStay,
		platToggle,
		platDownToNearestFloor,
		platDownToLowestCeiling,
		platRaiseAndStayLockout,
	};

	void Serialize(FSerializer &arc);
	void Tick ();

	bool IsLift() const { return m_Type == platDownWaitUpStay || m_Type == platDownWaitUpStayStone; }
	void Construct(sector_t *sector);

	double	 	m_Speed;
	double	 	m_Low;
	double	 	m_High;
	int 		m_Wait;
	int 		m_Count;
	EPlatState	m_Status;
	EPlatState	m_OldStatus;
	int			m_Crush;
	int 		m_Tag;
	EPlatType	m_Type;
protected:

	void PlayPlatSound (const char *sound);
	const char *GetSoundByType () const;
	void Reactivate ();
	void Stop ();

	friend struct FLevelLocals;
};

