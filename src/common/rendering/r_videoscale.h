/*
** r_videoscale.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2017 Magnus Norddahl
** Copyright 2017-2020 Rachael Alexanderson
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

#ifndef __VIDEOSCALE_H__
#define __VIDEOSCALE_H__
EXTERN_CVAR (Int, vid_scalemode)
bool ViewportLinearScale();
int ViewportScaledWidth(int width, int height);
int ViewportScaledHeight(int width, int height);
float ViewportPixelAspect();
#endif //__VIDEOSCALE_H__
