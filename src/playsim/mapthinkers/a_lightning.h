/*
** a_lightning.h
**
** Hexen's lightning system
**
**---------------------------------------------------------------------------
**
** Copyright 1994-1996 Raven Software
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

#ifndef __A_LIGHTNING_H__
#define __A_LIGHTNING_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "dthinker.h"
#include "s_soundinternal.h"

class DLightningThinker : public DThinker
{
	DECLARE_CLASS (DLightningThinker, DThinker);
public:
	static const int DEFAULT_STAT = STAT_LIGHTNING;
	void Construct(FSoundID tempSound = NO_SOUND);
	~DLightningThinker ();
	void Serialize(FSerializer &arc);
	void Tick ();
	void ForceLightning (int mode, FSoundID tempSound = NO_SOUND);
	void TerminateLightning();

protected:
	void LightningFlash ();

	int NextLightningFlash;
	int LightningFlashCount;
	bool Stopped;
	FSoundID TempLightningSound;
	TArray<short> LightningLightLevels;
};


#endif //__A_LIGHTNING_H__
