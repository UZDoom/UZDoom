/*
** statnums.h
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
** These are different statnums for thinkers. The idea to maintain multiple
** lists for different types of thinkers is taken from Build. Every thinker
** is ticked by statnum, so a thinker with a low statnum will always tick
** before a thinker with a high statnum. If a thinker is not explicitly
** created with a statnum, it will be given STAT_DEFAULT
*/

#ifndef __STATNUMS_H
#define __STATNUMS_H

enum
{ // Thinkers that don't actually think
	STAT_INFO,								// An info queue
	STAT_DECAL,								// A decal
	STAT_AUTODECAL,							// A decal that can be automatically deleted
	STAT_CORPSEPOINTER,						// An entry in Hexen's corpse queue
	STAT_TRAVELLING,						// An actor temporarily travelling to a new map
	STAT_STATIC,							// persistent across maps.

  // Thinkers that do think
	STAT_FIRST_THINKING=32,
	STAT_SCROLLER=STAT_FIRST_THINKING,		// A DScroller thinker
	STAT_PLAYER,							// A player actor
	STAT_BOSSTARGET,						// A boss brain target
	STAT_LIGHTNING,							// The lightning thinker
	STAT_DECALTHINKER,						// An object that thinks for a decal
	STAT_INVENTORY,							// An inventory item
	STAT_LIGHT,								// A sector light effect
	STAT_LIGHTTRANSFER,						// A sector light transfer. These must be ticked after the light effects!!!
	STAT_EARTHQUAKE,						// Earthquake actors
	STAT_MAPMARKER,							// Map marker actors
	STAT_DLIGHT,
	
	STAT_USER = 70,
	STAT_USER_MAX = 90,

	STAT_DEFAULT = 100,						// Thinkers go here unless specified otherwise.
	STAT_SECTOREFFECT,						// All sector effects that cause floor and ceiling movement
	STAT_ACTORMOVER,						// actor movers
	STAT_SCRIPTS,							// The ACS thinker. This is to ensure that it can't tick before all actors called PostBeginPlay
	STAT_BOT,								// Bot thinker
	STAT_VISUALTHINKER,							// VisualThinker Thinker
};

#endif
