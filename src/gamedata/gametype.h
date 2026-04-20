/*
** gametype.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Christoph Oelckers
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

#ifndef EGAMETYPE
#define EGAMETYPE
enum EGameType
{
	GAME_Any	 = 0,
	GAME_Doom	 = 1,
	GAME_Heretic = 2,
	GAME_Hexen	 = 4,
	GAME_Strife	 = 8,
	GAME_Chex	 = 16, //Chex is basically Doom, but we need to have a different set of actors.

	GAME_Raven			= GAME_Heretic|GAME_Hexen,
	GAME_DoomChex		= GAME_Doom|GAME_Chex,
	GAME_DoomStrifeChex	= GAME_Doom|GAME_Strife|GAME_Chex
};
#endif

