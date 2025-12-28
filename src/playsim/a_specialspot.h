/*
** a_specialspot.h
**
** Handling for special spot actors like BrainTargets, MaceSpawners etc.
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

