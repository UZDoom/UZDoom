/*
** r_visiblespritelist.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "p_lnspec.h"

#include "doomdef.h"
#include "m_swap.h"

#include "filesystem.h"
#include "g_levellocals.h"
#include "p_maputl.h"
#include "swrenderer/things/r_visiblesprite.h"
#include "swrenderer/things/r_visiblespritelist.h"
#include "r_memory.h"

namespace swrenderer
{
	void VisibleSpriteList::Clear()
	{
		Sprites.Clear();
		StartIndices.Clear();
		SortedSprites.Clear();
	}

	void VisibleSpriteList::PushPortal()
	{
		StartIndices.Push(Sprites.Size());
	}

	void VisibleSpriteList::PopPortal()
	{
		Sprites.Resize(StartIndices.Last());
		StartIndices.Pop();
	}

	void VisibleSpriteList::Push(VisibleSprite *sprite)
	{
		Sprites.Push(sprite);
	}

	void VisibleSpriteList::Sort(RenderThread *thread)
	{
		unsigned int first = StartIndices.Size() == 0 ? 0 : StartIndices.Last();
		unsigned int count = Sprites.Size() - first;

		SortedSprites.Resize(count);

		if (count == 0)
			return;

		if (!(thread->Viewport->Level()->i_compatflags & COMPATF_SPRITESORT))
		{
			for (unsigned int i = 0; i < count; i++)
				SortedSprites[i] = Sprites[first + i];
		}
		else
		{
			// If the compatibility option is on sprites of equal distance need to
			// be sorted in inverse order. This is most easily achieved by
			// filling the sort array backwards before the sort.
			for (unsigned int i = 0; i < count; i++)
				SortedSprites[i] = Sprites[first + count - i - 1];
		}

		{
			std::stable_sort(&SortedSprites[0], &SortedSprites[count], [](VisibleSprite *a, VisibleSprite *b) -> bool
			{
				return a->SortDist() > b->SortDist();
			});
		}
	}

	uint32_t VisibleSpriteList::FindSubsectorDepth(RenderThread *thread, const DVector2 &worldPos)
	{
		auto Level = thread->Viewport->Level();
		if (Level->nodes.Size() == 0)
		{
			subsector_t *sub = &Level->subsectors[0];
			return thread->OpaquePass->GetSubsectorDepth(sub->Index());
		}
		else
		{
			return FindSubsectorDepth(thread, worldPos, Level->HeadNode());
		}
	}

	uint32_t VisibleSpriteList::FindSubsectorDepth(RenderThread *thread, const DVector2 &worldPos, void *node)
	{
		while (!((size_t)node & 1))  // Keep going until found a subsector
		{
			node_t *bsp = (node_t *)node;

			DVector2 planePos(FIXED2DBL(bsp->x), FIXED2DBL(bsp->y));
			DVector2 planeNormal = DVector2(FIXED2DBL(-bsp->dy), FIXED2DBL(bsp->dx));
			double planeD = planeNormal | planePos;

			int side = (worldPos | planeNormal) > planeD;
			node = bsp->children[side];
		}

		subsector_t *sub = (subsector_t *)((uint8_t *)node - 1);
		return thread->OpaquePass->GetSubsectorDepth(sub->Index());
	}
}
