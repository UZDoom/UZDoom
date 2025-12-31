/*
** a_decalfx.h
**
** Decal animation thinkers
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

#pragma once

#include "dthinker.h"

struct DDecalThinker : public DThinker
{
	DECLARE_CLASS (DDecalThinker, DThinker)
	HAS_OBJECT_POINTERS
public:
	static const int DEFAULT_STAT = STAT_DECALTHINKER;
	void Construct(DBaseDecal *decal)
	{
		TheDecal = decal;
	}
	void Serialize(FSerializer &arc);
	TObjPtr<DBaseDecal*> TheDecal;
};

class DDecalFader : public DDecalThinker
{
	DECLARE_CLASS (DDecalFader, DDecalThinker)
public:
	void Construct(DBaseDecal *decal)
	{
		Super::Construct(decal);
	}
	void Serialize(FSerializer &arc);
	void Tick ();

	int TimeToStartDecay;
	int TimeToEndDecay;
	double StartTrans;
};

class DDecalColorer : public DDecalThinker
{
	DECLARE_CLASS (DDecalColorer, DDecalThinker)
public:
	void Construct(DBaseDecal *decal)
	{
		Super::Construct(decal);
	}
	void Serialize(FSerializer &arc);
	void Tick ();

	int TimeToStartDecay;
	int TimeToEndDecay;
	PalEntry StartColor;
	PalEntry GoalColor;
};

class DDecalStretcher : public DDecalThinker
{
	DECLARE_CLASS (DDecalStretcher, DDecalThinker)
public:
	void Construct(DBaseDecal *decal)
	{
		Super::Construct(decal);
	}
	void Serialize(FSerializer &arc);
	void Tick ();

	int TimeToStart;
	int TimeToStop;
	double GoalX;
	double StartX;
	double GoalY;
	double StartY;
	bool bStretchX;
	bool bStretchY;
	bool bStarted;
};

class DDecalSlider : public DDecalThinker
{
	DECLARE_CLASS (DDecalSlider, DDecalThinker)
public:
	void Construct(DBaseDecal *decal)
	{
		Super::Construct(decal);
	}
	void Serialize(FSerializer &arc);
	void Tick ();

	int TimeToStart;
	int TimeToStop;
/*	double DistX; */
	double DistY;
	double StartX;
	double StartY;
	bool bStarted;
};

