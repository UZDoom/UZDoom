/*
** r_clipsegment.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

namespace swrenderer
{
	typedef bool(*VisibleSegmentCallback)(int x1, int x2);

	class VisibleSegmentRenderer
	{
	public:
		virtual ~VisibleSegmentRenderer() { }
		virtual bool RenderWallSegment(int x1, int x2) { return true; }
	};

	class RenderClipSegment
	{
	public:
		void Clear(short left, short right);
		bool Clip(int x1, int x2, bool solid, VisibleSegmentRenderer *visitor);
		bool Check(int first, int last);
		bool IsVisible(int x1, int x2);
		
	private:
		struct cliprange_t
		{
			short first, last;
		};

		cliprange_t *newend; // newend is one past the last valid seg
		cliprange_t solidsegs[MAXWIDTH / 2 + 2];
	};
}
