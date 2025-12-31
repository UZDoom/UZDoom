/*
** announcer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __ANNOUNCER_H__
#define __ANNOUNCER_H__

#ifdef _MSC_VER
#pragma once
#endif

class AActor;

// These all return true if they generated a text message

bool AnnounceGameStart ();
bool AnnounceKill (AActor *killer, AActor *killee);
bool AnnounceTelefrag (AActor *killer, AActor *killee);
bool AnnounceSpree (AActor *who);
bool AnnounceSpreeLoss (AActor *who);
bool AnnounceMultikill (AActor *who);

#endif //__ANNOUNCER_H__
