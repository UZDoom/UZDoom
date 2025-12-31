/*
** floatrect.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2013-2020 Christoph Oelckers
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

struct FloatRect
{
	float left,top;
	float width,height;


	void Offset(float xofs,float yofs)
	{
		left+=xofs;
		top+=yofs;
	}
	void Scale(float xfac,float yfac)
	{
		left*=xfac;
		width*=xfac;
		top*=yfac;
		height*=yfac;
	}
};

struct DoubleRect
{
	double left, top;
	double width, height;


	void Offset(double xofs, double yofs)
	{
		left += xofs;
		top += yofs;
	}
	void Scale(double xfac, double yfac)
	{
		left *= xfac;
		width *= xfac;
		top *= yfac;
		height *= yfac;
	}
};
