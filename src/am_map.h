/*
** am_map.h
**
**
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

#ifndef __AMMAP_H__
#define __AMMAP_H__

#include "dobject.h"

struct event_t;
class FSerializer;
struct FLevelLocals;

class DAutomapBase : public DObject
{
	DECLARE_ABSTRACT_CLASS(DAutomapBase, DObject);
public:
	FLevelLocals *Level;	// temporary location so that it can be set from the outside.

	// Called by main loop.
	virtual bool Responder(event_t* ev, bool last) = 0;

	// Called by main loop.
	virtual void Ticker(void) = 0;

	// Called by main loop,
	// called instead of view drawer if automap active.
	virtual void Drawer(int bottom) = 0;

	virtual void NewResolution() = 0;
	virtual void NewUIScale() = 0;
	virtual void LevelInit() = 0;
	virtual void UpdateShowAllLines() = 0;
	virtual void GoBig() = 0;
	virtual void ResetFollowLocation() = 0;
	virtual int addMark() = 0;
	virtual bool clearMarks() = 0;
	virtual DVector2 GetPosition() = 0;
	virtual void startDisplay() = 0;

};

void AM_StaticInit();
void AM_ClearColorsets();	// reset data for a restart.
DAutomapBase *AM_Create(FLevelLocals *Level);
void AM_Stop();
void AM_ToggleMap();

#endif
