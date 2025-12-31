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

