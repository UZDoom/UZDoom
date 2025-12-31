/*
** a_scroll.h
**
** Initializes and implements BOOM linedef triggers for Scrollers/Conveyors
**
**---------------------------------------------------------------------------
**
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
** For code that originates from ZDoom the following applies:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

enum EScrollAffect
{
	SCROLL_Textures = 1,
	SCROLL_StaticObjects = 2,
	SCROLL_Players = 4,
	SCROLL_Monsters = 8,
	SCROLL_All = 15
};

//-----------------------------------------------------------------------------
//
// killough 3/7/98: Add generalized scroll effects
//
//-----------------------------------------------------------------------------

class DScroller : public DThinker
{
	DECLARE_CLASS (DScroller, DThinker)
	HAS_OBJECT_POINTERS
public:
	static const int DEFAULT_STAT = STAT_SCROLLER;

	void Construct(EScroll type, double dx, double dy, sector_t *control, sector_t *sec, side_t *side, int accel, EScrollPos scrollpos = EScrollPos::scw_all, int aff = SCROLL_All);
	void Construct(double dx, double dy, const line_t *l, sector_t *control, int accel, EScrollPos scrollpos = EScrollPos::scw_all);
	void OnDestroy() override;

	void Serialize(FSerializer &arc);
	void Tick ();

	bool AffectsWall (side_t * wall) const { return m_Side == wall; }
	side_t *GetWall () const { return m_Side; }
	sector_t *GetSector() const { return m_Sector; }
	void SetRate (double dx, double dy) { m_dx = dx; m_dy = dy; }
	bool IsType (EScroll type) const { return type == m_Type; }
	EScrollPos GetScrollParts() const { return m_Parts; }

protected:
	EScroll m_Type;		// Type of scroll effect
	int m_Affect;
	double m_dx, m_dy;		// (dx,dy) scroll speeds
	sector_t *m_Sector;		// Affected sector
	side_t *m_Side;			// ... or side
	sector_t *m_Controller;	// Control sector (nullptr if none) used to control scrolling
	double m_LastHeight;	// Last known height of control sector
	double m_vdx, m_vdy;	// Accumulated velocity if accelerative
	int m_Accel;			// Whether it's accelerative
	EScrollPos m_Parts;			// Which parts of a sidedef are being scrolled?
	TObjPtr<DInterpolation*> m_Interpolations[3];
};

