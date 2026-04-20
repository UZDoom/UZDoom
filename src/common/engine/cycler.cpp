/*
** cycler.cpp
**
** Implements the cycler for dynamic lights and texture shaders.
**
**---------------------------------------------------------------------------
**
** Copyright 2003 Timothy Stump
** Copyright 2006-2016 Christoph Oelckers
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

#include <math.h>
#include "serializer.h"
#include "cycler.h"

//==========================================================================
//
// This will never be called with a null-def, so don't bother with that case.
//
//==========================================================================

FSerializer &Serialize(FSerializer &arc, const char *key, FCycler &c, FCycler *def)
{
	if (arc.BeginObject(key))
	{
		arc("start", c.m_start, def->m_start)
			("end", c.m_end, def->m_end)
			("current", c.m_current, def->m_current)
			("time", c.m_time, def->m_time)
			("cycle", c.m_cycle, def->m_cycle)
			("increment", c.m_increment, def->m_increment)
			("shouldcycle", c.m_shouldCycle, def->m_shouldCycle)
			.Enum("type", c.m_cycleType)
			.EndObject();
	}
	return arc;
}


//==========================================================================
//
//
//
//==========================================================================

void FCycler::SetParams(double start, double end, double cycle, bool update)
{
	if (!update || cycle != m_cycle)
	{
		m_cycle = cycle;
		m_time = 0.;
		m_increment = true;
		m_current = start;
	}
	else
	{
		// When updating and keeping the same cycle, scale the current light size to the new dimensions.
		double fact = (m_current - m_start) / (m_end - m_start);
		m_current = start + fact *(end - start);
	}
	m_start = start;
	m_end = end;
}


//==========================================================================
//
//
//
//==========================================================================

void FCycler::Update(double diff)
{
	double mult, angle;
	double step = m_end - m_start;

	if (!m_shouldCycle)
	{
		return;
	}

	m_time += diff;
	if (m_time >= m_cycle)
	{
		m_time = m_cycle;
	}

	mult = m_time / m_cycle;

	switch (m_cycleType)
	{
	case CYCLE_Linear:
		if (m_increment)
		{
			m_current = m_start + (step * mult);
		}
		else
		{
			m_current = m_end - (step * mult);
		}
		break;
	case CYCLE_Sin:
		angle = double(M_PI * 2. * mult);
		mult = g_sin(angle);
		mult = (mult + 1.) / 2.;
		m_current = m_start + (step * mult);
		break;
	case CYCLE_Cos:
		angle = double(M_PI * 2. * mult);
		mult = g_cos(angle);
		mult = (mult + 1.) / 2.;
		m_current = m_start + (step * mult);
		break;
	case CYCLE_SawTooth:
		m_current = m_start + (step * mult);
		break;
	case CYCLE_Square:
		if (m_increment)
		{
			m_current = m_start;
		}
		else
		{
			m_current = m_end;
		}
		break;
	}

	if (m_time == m_cycle)
	{
		m_time = 0.;
		m_increment = !m_increment;
	}
}


