/*
** r_visiblespritelist.h
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
	struct DrawSegment;
	class VisibleSprite;

	class VisibleSpriteList
	{
	public:
		void Clear();
		void PushPortal();
		void PopPortal();
		void Push(VisibleSprite *sprite);
		void Sort(RenderThread *thread);

		TArray<VisibleSprite *> SortedSprites;

	private:
		uint32_t FindSubsectorDepth(RenderThread *thread, const DVector2 &worldPos);
		uint32_t FindSubsectorDepth(RenderThread *thread, const DVector2 &worldPos, void *node);

		TArray<VisibleSprite *> Sprites;
		TArray<unsigned int> StartIndices;
	};
}
