/*
** a_decalfx.h
**
** Decal animation thinkers
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

