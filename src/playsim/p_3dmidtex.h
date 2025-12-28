/*
** p_3dmidtex.h
**
** Eternity-style 3D-midtex handling (No original Eternity code here!)
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Christoph Oelckers
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
