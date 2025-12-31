/*
** hw_fakeflat.h
**
** Fake flat functions to render stacked sectors
**
**---------------------------------------------------------------------------
**
** Copyright 2001-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

enum area_t : int
{
	area_normal,
	area_below,
	area_above,
	area_default
};


// Global functions.
bool hw_CheckClip(side_t * sidedef, sector_t * frontsector, sector_t * backsector);
void hw_ClearFakeFlat();
sector_t * hw_FakeFlat(sector_t * sec, area_t in_area, bool back, sector_t *localcopy = nullptr);
area_t hw_CheckViewArea(vertex_t *v1, vertex_t *v2, sector_t *frontsector, sector_t *backsector);

