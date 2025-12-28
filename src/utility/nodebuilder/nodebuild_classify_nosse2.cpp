/*
** nodebuild_classify_nosse2.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2002-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
** 4. When not used as part of ZDoom or a ZDoom derivative, this code will be
**    covered by the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or (at
**    your option) any later version.
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

#include "doomtype.h"
#include "nodebuild.h"

#define FAR_ENOUGH 17179869184.f		// 4<<32

int FNodeBuilder::ClassifyLine(node_t &node, const FPrivVert *v1, const FPrivVert *v2, int sidev[2])
{
	double d_x1 = double(node.x);
	double d_y1 = double(node.y);
	double d_dx = double(node.dx);
	double d_dy = double(node.dy);
	double d_xv1 = double(v1->x);
	double d_xv2 = double(v2->x);
	double d_yv1 = double(v1->y);
	double d_yv2 = double(v2->y);

	double s_num1 = (d_y1 - d_yv1) * d_dx - (d_x1 - d_xv1) * d_dy;
	double s_num2 = (d_y1 - d_yv2) * d_dx - (d_x1 - d_xv2) * d_dy;

	int nears = 0;

	if (s_num1 <= -FAR_ENOUGH)
	{
		if (s_num2 <= -FAR_ENOUGH)
		{
			sidev[0] = sidev[1] = 1;
			return 1;
		}
		if (s_num2 >= FAR_ENOUGH)
		{
			sidev[0] = 1;
			sidev[1] = -1;
			return -1;
		}
		nears = 1;
	}
	else if (s_num1 >= FAR_ENOUGH)
	{
		if (s_num2 >= FAR_ENOUGH)
		{
			sidev[0] = sidev[1] = -1;
			return 0;
		}
		if (s_num2 <= -FAR_ENOUGH)
		{
			sidev[0] = -1;
			sidev[1] = 1;
			return -1;
		}
		nears = 1;
	}
	else
	{
		nears = 2 | int(fabs(s_num2) < FAR_ENOUGH);
	}

	if (nears)
	{
		double l = 1.f / (d_dx*d_dx + d_dy*d_dy);
		if (nears & 2)
		{
			double dist = s_num1 * s_num1 * l;
			if (dist < SIDE_EPSILON*SIDE_EPSILON)
			{
				sidev[0] = 0;
			}
			else
			{
				sidev[0] = s_num1 > 0.0 ? -1 : 1;
			}
		}
		else
		{
			sidev[0] = s_num1 > 0.0 ? -1 : 1;
		}
		if (nears & 1)
		{
			double dist = s_num2 * s_num2 * l;
			if (dist < SIDE_EPSILON*SIDE_EPSILON)
			{
				sidev[1] = 0;
			}
			else
			{
				sidev[1] = s_num2 > 0.0 ? -1 : 1;
			}
		}
		else
		{
			sidev[1] = s_num2 > 0.0 ? -1 : 1;
		}
	}
	else
	{
		sidev[0] = s_num1 > 0.0 ? -1 : 1;
		sidev[1] = s_num2 > 0.0 ? -1 : 1;
	}

	if ((sidev[0] | sidev[1]) == 0)
	{ // seg is coplanar with the splitter, so use its orientation to determine
	  // which child it ends up in. If it faces the same direction as the splitter,
	  // it goes in front. Otherwise, it goes in back.

		if (node.dx != 0)
		{
			if ((node.dx > 0 && v2->x > v1->x) || (node.dx < 0 && v2->x < v1->x))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			if ((node.dy > 0 && v2->y > v1->y) || (node.dy < 0 && v2->y < v1->y))
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	else if (sidev[0] <= 0 && sidev[1] <= 0)
	{
		return 0;
	}
	else if (sidev[0] >= 0 && sidev[1] >= 0)
	{
		return 1;
	}
	return -1;
}
