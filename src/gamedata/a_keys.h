/*
** a_keys.h
**
** Implements all keys and associated data
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Christoph Oelckers
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

#ifndef A_KEYS_H
#define A_KEYS_H

class AActor;
class PClassActor;

int P_CheckKeys (AActor *owner, int keynum, bool remote, bool quiet = false);
int P_IsLockDefined (int lock);
void P_InitKeyMessages ();
int P_GetMapColorForLock (int lock);
int P_GetMapColorForKey (AActor *key);
int P_GetKeyTypeCount();
PClassActor *P_GetKeyType(int num);

#endif
