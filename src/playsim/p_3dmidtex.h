/*
** p_3dmidtex.h
**
** Eternity-style 3D-midtex handling (No original Eternity code here!)
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Christoph Oelckers
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

#ifndef __3DMIDTEX_H
#define __3DMIDTEX_H

#include "doomtype.h"

class DInterpolation;
struct sector_t;
struct line_t;
class AActor;

bool P_Scroll3dMidtex(sector_t *sector, int crush, double move, bool ceiling, bool instant = false);
void P_Start3dMidtexInterpolations(TArray<DInterpolation *> &list, sector_t *sec, bool ceiling);
void P_Attach3dMidtexLinesToSector(sector_t *dest, int lineid, int tag, bool ceiling);
bool P_GetMidTexturePosition(const line_t *line, int sideno, double *ptextop, double *ptexbot);
bool P_Check3dMidSwitch(AActor *actor, line_t *line, int side);
bool P_LineOpening_3dMidtex(AActor *thing, const line_t *linedef, struct FLineOpening &open, bool restrict=false);

bool P_MoveLinkedSectors(sector_t *sector, int crush, double move, bool ceiling, bool instant = false);
void P_StartLinkedSectorInterpolations(TArray<DInterpolation *> &list, sector_t *sector, bool ceiling);
bool P_AddSectorLinks(sector_t *control, int tag, INTBOOL ceiling, int movetype);
void P_AddSectorLinksByID(sector_t *control, int id, INTBOOL ceiling);


#endif
