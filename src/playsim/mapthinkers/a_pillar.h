/*
** a_pillar.h
**
** Handles pillars
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

//
// [RH]
// P_PILLAR
//

class DPillar : public DMover
{
	DECLARE_CLASS (DPillar, DMover)
	HAS_OBJECT_POINTERS
public:
	enum EPillar
	{
		pillarBuild,
		pillarOpen

	};

	void Construct (sector_t *sector, EPillar type, double speed, double height, double height2, int crush, bool hexencrush);

	void Serialize(FSerializer &arc);
	void Tick ();
	void OnDestroy() override;

protected:
	EPillar		m_Type;
	double		m_FloorSpeed;
	double		m_CeilingSpeed;
	double		m_FloorTarget;
	double		m_CeilingTarget;
	int			m_Crush;
	bool		m_Hexencrush;
	TObjPtr<DInterpolation*> m_Interp_Ceiling;
	TObjPtr<DInterpolation*> m_Interp_Floor;
};

