/*
** t_fs.h
**
** global FS interface
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2017 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef T_FS_H
#define T_FS_H

struct MapData;
class AActor;

void T_PreprocessScripts(FLevelLocals *Level);
void T_LoadScripts(FLevelLocals *Level, MapData * map);
void T_AddSpawnedThing(FLevelLocals *Level, AActor * );
bool T_RunScript(FLevelLocals *l, int snum, AActor * t_trigger);

#endif
