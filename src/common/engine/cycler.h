/*
** cycler.h
**
** Implements the cycler for dynamic lights and texture shaders.
**
**---------------------------------------------------------------------------
**
** Copyright 2003 Timothy Stump
** Copyright 2013-2016 Christoph Oelckers
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

#ifndef __GL_CYCLER_H
#define __GL_CYCLER_H

class FSerializer;

enum CycleType
{
	CYCLE_Linear,
	CYCLE_Sin,
	CYCLE_Cos,
	CYCLE_SawTooth,
	CYCLE_Square
};

class FCycler;
FSerializer &Serialize(FSerializer &arc, const char *key, FCycler &c, FCycler *def);

class FCycler
{
	friend FSerializer &Serialize(FSerializer &arc, const char *key, FCycler &c, FCycler *def);

public:
	FCycler() = default;
	FCycler(const FCycler &other) = default;
	FCycler &operator=(const FCycler &other) = default;

   void Update(double diff);
   void SetParams(double start, double end, double cycle, bool update = false);
   void ShouldCycle(bool sc) { m_shouldCycle = sc; }
   void SetCycleType(CycleType ct) { m_cycleType = ct; }
   double GetVal() { return m_current; }

   inline operator double () const { return m_current; }

   double m_start, m_end, m_current;
   double m_time, m_cycle;
   bool m_increment, m_shouldCycle;

   CycleType m_cycleType;
};


#endif
