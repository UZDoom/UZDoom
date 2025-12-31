/*
** a_pusher.h
**
** Initializes and implements BOOM linedef triggers for Wind/Current
**
**---------------------------------------------------------------------------
**
** Copyright 1998-1998 Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
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
*/

#pragma once

// phares 3/20/98: added new model of Pushers for push/pull effects

class DPusher : public DThinker
{
	DECLARE_CLASS (DPusher, DThinker)
	HAS_OBJECT_POINTERS
	
	enum
	{
		PUSH_FACTOR = 128
	};

public:
	enum EPusher
	{
		p_push,
		p_pull,
		p_wind,
		p_current
	};

	void Construct(EPusher type, line_t *l, int magnitude, int angle, AActor *source, int affectee);
	void Serialize(FSerializer &arc);
	int CheckForSectorMatch (EPusher type, int tag);
	void ChangeValues (int magnitude, int angle)
	{
		DAngle ang =  DAngle::fromDeg(angle * (360. / 256.));
		m_PushVec = ang.ToVector(magnitude);
		m_Magnitude = magnitude;
	}

	void Tick ();

protected:
	EPusher m_Type;
	TObjPtr<AActor*> m_Source;// Point source if point pusher
	DVector2 m_PushVec;
	double m_Magnitude;		// Vector strength for point pusher
	double m_Radius;		// Effective radius for point pusher
	int m_Affectee;			// Number of affected sector

	friend bool PIT_PushThing (AActor *thing);
};

class DThruster : public DThinker
{
	DECLARE_CLASS(DThruster, DThinker)

	DVector2 m_PushVec;
	sector_t* m_Sector;
	int m_Type;
	int m_Location;

public:
	void Construct(sector_t* sec, double dx, double dy, int type, int location);
	void Serialize(FSerializer& arc);
	void Tick();

};
