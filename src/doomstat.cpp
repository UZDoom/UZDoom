/*
** doomstat.cpp
**
** Put all global state variables here.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "stringtable.h"
#include "doomstat.h"
#include "i_system.h"
#include "g_level.h"
#include "g_levellocals.h"

int SaveVersion;

// Localizable strings

// Game speed
EGameSpeed		GameSpeed = SPEED_Normal;

int NextSkill = -1;

int SinglePlayerClass[MAXPLAYERS];

FString LumpFilterIWAD;
