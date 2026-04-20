/*
** intrect.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2018 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#pragma once

struct IntRect
{
	int left, top;
	int width, height;

	void Offset(int xofs, int yofs)
	{
		left += xofs;
		top += yofs;
	}

	void AddToRect(int x, int y)
	{
		if (x < left)
			left = x;
		if (x > left + width)
			width = x - left;

		if (y < top)
			top = y;
		if (y > top + height)
			height = y - top;
	}

	int Left() const
	{
		return left;
	}

	int Top() const
	{
		return top;
	}


	int Right() const
	{
		return left + width;
	}

	int Bottom() const
	{
		return top + height;
	}

	int Width() const
	{
		return width;
	}

	int Height() const
	{
		return height;
	}


};

