/*
** a_lights.h
**
** Handle Sector base lighting effects.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
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

class DLighting : public DSectorEffect
{
	DECLARE_CLASS(DLighting, DSectorEffect)
public:
	static const int DEFAULT_STAT = STAT_LIGHT;
};

class DFireFlicker : public DLighting
{
	DECLARE_CLASS(DFireFlicker, DLighting)
public:
	void Construct(sector_t *sector);
	void Construct(sector_t *sector, int upper, int lower);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int 		m_Count;
	int 		m_MaxLight;
	int 		m_MinLight;
};

class DFlicker : public DLighting
{
	DECLARE_CLASS(DFlicker, DLighting)
public:
	void Construct(sector_t *sector, int upper, int lower);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int 		m_Count;
	int 		m_MaxLight;
	int 		m_MinLight;
};

class DLightFlash : public DLighting
{
	DECLARE_CLASS(DLightFlash, DLighting)
public:
	void Construct(sector_t *sector);
	void Construct(sector_t *sector, int min, int max);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int 		m_Count;
	int 		m_MaxLight;
	int 		m_MinLight;
	int 		m_MaxTime;
	int 		m_MinTime;
};

class DStrobe : public DLighting
{
	DECLARE_CLASS(DStrobe, DLighting)
public:
	void Construct(sector_t *sector, int utics, int ltics, bool inSync);
	void Construct(sector_t *sector, int upper, int lower, int utics, int ltics);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int 		m_Count;
	int 		m_MinLight;
	int 		m_MaxLight;
	int 		m_DarkTime;
	int 		m_BrightTime;
};

class DGlow : public DLighting
{
	DECLARE_CLASS(DGlow, DLighting)
public:
	void Construct(sector_t *sector);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int 		m_MinLight;
	int 		m_MaxLight;
	int 		m_Direction;
};

// [RH] Glow from Light_Glow and Light_Fade specials
class DGlow2 : public DLighting
{
	DECLARE_CLASS(DGlow2, DLighting)
public:
	void Construct(sector_t *sector, int start, int end, int tics, bool oneshot);
	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	int			m_Start;
	int			m_End;
	int			m_MaxTics;
	int			m_Tics;
	bool		m_OneShot;
};

// [RH] Phased light thinker
class DPhased : public DLighting
{
	DECLARE_CLASS(DPhased, DLighting)
public:
	void Construct(sector_t *sector, int baselevel = 0, int phase = 0);
	// These are for internal use only but the Create template needs access to them.
	void Construct();
	void Propagate();

	void		Serialize(FSerializer &arc);
	void		Tick();
protected:
	uint8_t		m_BaseLevel;
	uint8_t		m_Phase;
private:
	int PhaseHelper(sector_t *sector, int index, int light, sector_t *prev);
};
