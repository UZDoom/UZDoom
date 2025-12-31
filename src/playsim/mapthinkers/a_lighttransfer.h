/*
** a_lighttransfer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2003-2018 Christoph Oelckers
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

class DLightTransfer : public DThinker
{
	DECLARE_CLASS (DLightTransfer, DThinker)

public:
	static const int DEFAULT_STAT = STAT_LIGHTTRANSFER;
	void Construct(sector_t *srcSec, int target, bool copyFloor);
	void Serialize(FSerializer &arc);
	void Tick ();

protected:
	void DoTransfer (int level, int target, bool floor);

	sector_t *Source;
	int TargetTag;
	bool CopyFloor;
	short LastLight;
};

class DWallLightTransfer : public DThinker
{
	enum
	{
		WLF_SIDE1=1,
		WLF_SIDE2=2,
		WLF_NOFAKECONTRAST=4
	};

	DECLARE_CLASS (DWallLightTransfer, DThinker)
public:
	static const int DEFAULT_STAT = STAT_LIGHTTRANSFER;
	void Construct(sector_t *srcSec, int target, uint8_t flags);
	void Serialize(FSerializer &arc);
	void Tick ();

protected:
	void DoTransfer (short level, int target, uint8_t flags);

	sector_t *Source;
	int TargetID;
	short LastLight;
	uint8_t Flags;
};

