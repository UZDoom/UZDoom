/*
** dsectoreffect.h
**
** Base class for effects on sectors.
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

#ifndef __DSECTOREFFECT_H__
#define __DSECTOREFFECT_H__

#include "dthinker.h"
#include "r_defs.h"

class DSectorEffect : public DThinker
{
	DECLARE_CLASS (DSectorEffect, DThinker)
public:
	static const int DEFAULT_STAT = STAT_SECTOREFFECT;
	void Construct(sector_t *sector);

	
	void Serialize(FSerializer &arc);
	void OnDestroy() override;

	sector_t *GetSector() const { return m_Sector; }

	sector_t *m_Sector;
};

class DMover : public DSectorEffect
{
	DECLARE_CLASS (DMover, DSectorEffect)
	HAS_OBJECT_POINTERS
protected:
	void Construct(sector_t *sector);

	TObjPtr<DInterpolation*> interpolation;
public:
	void StopInterpolation(bool force = false);

protected:

	void Serialize(FSerializer &arc);
	void OnDestroy() override;
};

class DMovingFloor : public DMover
{
	DECLARE_CLASS (DMovingFloor, DMover)
protected:
	void Construct(sector_t *sector);
};

class DMovingCeiling : public DMover
{
	DECLARE_CLASS (DMovingCeiling, DMover)
protected:
	void Construct(sector_t *sector, bool interpolate = true);
};

#endif //__DSECTOREFFECT_H__
