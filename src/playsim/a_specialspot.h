/*
** a_specialspot.h
**
** Handling for special spot actors like BrainTargets, MaceSpawners etc.
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

#ifndef __A_SPECSPOT_H
#define __A_SPECSPOT_H

#include "actor.h"
#include "tarray.h"

struct FSpotList;


class DSpotState : public DObject
{
	DECLARE_CLASS(DSpotState, DObject)
	TArray<FSpotList> SpotLists;

public:


	DSpotState ();
	void OnDestroy() override;
	void Tick ();
	static DSpotState *GetSpotState(bool create = true);
	FSpotList *FindSpotList(PClassActor *type);
	bool AddSpot(AActor *spot);
	bool RemoveSpot(AActor *spot);
	void Serialize(FSerializer &arc);
	AActor *GetNextInList(PClassActor *type, int skipcounter);
	AActor *GetSpotWithMinMaxDistance(PClassActor *type, double x, double y, double mindist, double maxdist);
	AActor *GetRandomSpot(PClassActor *type, bool onlyonce = false);
};


#endif

