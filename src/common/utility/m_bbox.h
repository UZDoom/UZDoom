/*
** m_bbox.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Marisa Heit
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
** Code written prior to 2026 is also licensed under:
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

#ifndef __M_BBOX_H__
#define __M_BBOX_H__

#include <float.h>
#include "vectors.h"

enum
{
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
};		// bbox coordinates


class FBoundingBox
{
public:
	FBoundingBox()
	{
		ClearBox();
	}

	FBoundingBox(double left, double bottom, double right, double top)
	{
		m_Box[BOXTOP] = top;
		m_Box[BOXLEFT] = left;
		m_Box[BOXRIGHT] = right;
		m_Box[BOXBOTTOM] = bottom;
	}

	FBoundingBox(double x, double y, double radius)
	{
		setBox(x, y, radius);
	}


	void setBox(double x, double y, double radius)
	{
		m_Box[BOXTOP] = y + radius;
		m_Box[BOXLEFT] = x - radius;
		m_Box[BOXRIGHT] = x + radius;
		m_Box[BOXBOTTOM] = y - radius;
	}

	void ClearBox ()
	{
		m_Box[BOXTOP] = m_Box[BOXRIGHT] = -FLT_MAX;
		m_Box[BOXBOTTOM] = m_Box[BOXLEFT] = FLT_MAX;
	}

	// Returns a bounding box that encloses both bounding boxes
	FBoundingBox operator | (const FBoundingBox &box2) const
	{
		return FBoundingBox(m_Box[BOXLEFT] < box2.m_Box[BOXLEFT] ? m_Box[BOXLEFT] : box2.m_Box[BOXLEFT],
							m_Box[BOXBOTTOM] < box2.m_Box[BOXBOTTOM] ? m_Box[BOXBOTTOM] : box2.m_Box[BOXBOTTOM],
							m_Box[BOXRIGHT] > box2.m_Box[BOXRIGHT] ? m_Box[BOXRIGHT] : box2.m_Box[BOXRIGHT],
							m_Box[BOXTOP] > box2.m_Box[BOXTOP] ? m_Box[BOXTOP] : box2.m_Box[BOXTOP]);
	}

	void AddToBox(const DVector2 &pos)
	{
		if (pos.X < m_Box[BOXLEFT])
			m_Box[BOXLEFT] = pos.X;
		if (pos.X > m_Box[BOXRIGHT])
			m_Box[BOXRIGHT] = pos.X;

		if (pos.Y < m_Box[BOXBOTTOM])
			m_Box[BOXBOTTOM] = pos.Y;
		if (pos.Y > m_Box[BOXTOP])
			m_Box[BOXTOP] = pos.Y;
	}

	bool CheckOverlap(const FBoundingBox &box2)
	{
		bool hori = (Left() > box2.Right()) || (Right() < box2.Left());
		bool vert = (Bottom() > box2.Top()) || (Top() < box2.Bottom());
		return !(hori || vert); // [DVR] For alternative space partition
	}

	inline double Top () const { return m_Box[BOXTOP]; }
	inline double Bottom () const { return m_Box[BOXBOTTOM]; }
	inline double Left () const { return m_Box[BOXLEFT]; }
	inline double Right () const { return m_Box[BOXRIGHT]; }

	void Set(int index, double value) {m_Box[index] = value;}

protected:
	double m_Box[4];
};


#endif //__M_BBOX_H__
