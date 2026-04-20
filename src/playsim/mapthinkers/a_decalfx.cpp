/*
** a_decalfx.cpp
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

#include "decallib.h"
#include "a_decalfx.h"
#include "serializer_doom.h"
#include "serialize_obj.h"
#include "a_sharedglobal.h"
#include "g_levellocals.h"
#include "m_fixed.h"


IMPLEMENT_CLASS(DDecalThinker, false, true)

IMPLEMENT_POINTERS_START(DDecalThinker)
	IMPLEMENT_POINTER(TheDecal)
IMPLEMENT_POINTERS_END

void DDecalThinker::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("thedecal", TheDecal);
}

IMPLEMENT_CLASS(DDecalFader, false, false)

void DDecalFader::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("starttime", TimeToStartDecay)
		("endtime", TimeToEndDecay)
		("starttrans", StartTrans);
}

void DDecalFader::Tick ()
{
	if (TheDecal == nullptr)
	{
		Destroy ();
	}
	else
	{
		if (Level->maptime < TimeToStartDecay || Level->isFrozen())
		{
			return;
		}
		else if (Level->maptime >= TimeToEndDecay)
		{
			TheDecal->Expired();		// for impact decal bookkeeping.
			TheDecal->Destroy ();		// remove the decal
			Destroy ();					// remove myself
			return;
		}
		if (StartTrans == -1)
		{
			StartTrans = TheDecal->Alpha;
		}

		int distanceToEnd = TimeToEndDecay - Level->maptime;
		int fadeDistance = TimeToEndDecay - TimeToStartDecay;
		TheDecal->Alpha = StartTrans * distanceToEnd / fadeDistance;
	}
}

IMPLEMENT_CLASS(DDecalStretcher, false, false)

void DDecalStretcher::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("starttime", TimeToStart)
		("endtime", TimeToStop)
		("goalx", GoalX)
		("startx", StartX)
		("stretchx", bStretchX)
		("goaly", GoalY)
		("starty", StartY)
		("stretchy", bStretchY)
		("started", bStarted);
}

void DDecalStretcher::Tick ()
{
	if (TheDecal == nullptr)
	{
		Destroy ();
		return;
	}
	if (Level->maptime < TimeToStart || Level->isFrozen())
	{
		return;
	}
	if (Level->maptime >= TimeToStop)
	{
		if (bStretchX)
		{
			TheDecal->ScaleX = GoalX;
		}
		if (bStretchY)
		{
			TheDecal->ScaleY = GoalY;
		}
		Destroy ();
		return;
	}
	if (!bStarted)
	{
		bStarted = true;
		StartX = TheDecal->ScaleX;
		StartY = TheDecal->ScaleY;
	}

	int distance = Level->maptime - TimeToStart;
	int maxDistance = TimeToStop - TimeToStart;
	if (bStretchX)
	{
		TheDecal->ScaleX = StartX + (GoalX - StartX) * distance / maxDistance;
	}
	if (bStretchY)
	{
		TheDecal->ScaleY = StartY + (GoalY - StartY) * distance / maxDistance;
	}
}

IMPLEMENT_CLASS(DDecalSlider, false, false)

void DDecalSlider::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("starttime", TimeToStart)
		("endtime", TimeToStop)
		("disty", DistY)
		("starty", StartY)
		("started", bStarted);
}

void DDecalSlider::Tick ()
{
	if (TheDecal == nullptr)
	{
		Destroy ();
		return;
	}
	if (Level->maptime < TimeToStart || Level->isFrozen())
	{
		return;
	}
	if (!bStarted)
	{
		bStarted = true;
		/*StartX = TheDecal->LeftDistance;*/
		StartY = TheDecal->Z;
	}
	if (Level->maptime >= TimeToStop)
	{
		/*TheDecal->LeftDistance = StartX + DistX;*/
		TheDecal->Z = StartY + DistY;
		Destroy ();
		return;
	}

	int distance = Level->maptime - TimeToStart;
	int maxDistance = TimeToStop - TimeToStart;
	/*TheDecal->LeftDistance = StartX + DistX * distance / maxDistance);*/
	TheDecal->Z = StartY + DistY * distance / maxDistance;
}

IMPLEMENT_CLASS(DDecalColorer, false, false)

void DDecalColorer::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("starttime", TimeToStartDecay)
		("endtime", TimeToEndDecay)
		("startcolor", StartColor)
		("goalcolor", GoalColor);
}

void DDecalColorer::Tick ()
{
	if (TheDecal == nullptr || !(TheDecal->RenderStyle.Flags & STYLEF_ColorIsFixed))
	{
		Destroy ();
	}
	else
	{
		if (Level->maptime < TimeToStartDecay || Level->isFrozen())
		{
			return;
		}
		else if (Level->maptime >= TimeToEndDecay)
		{
			TheDecal->SetShade (GoalColor);
			Destroy ();					// remove myself
		}
		if (StartColor.a == 255)
		{
			StartColor = TheDecal->AlphaColor & 0xffffff;
			if (StartColor == GoalColor)
			{
				Destroy ();
				return;
			}
		}
		if (Level->maptime & 0)
		{ // Changing the shade can be expensive, so don't do it too often.
			return;
		}

		int distance = Level->maptime - TimeToStartDecay;
		int maxDistance = TimeToEndDecay - TimeToStartDecay;
		int r = StartColor.r + Scale (GoalColor.r - StartColor.r, distance, maxDistance);
		int g = StartColor.g + Scale (GoalColor.g - StartColor.g, distance, maxDistance);
		int b = StartColor.b + Scale (GoalColor.b - StartColor.b, distance, maxDistance);
		TheDecal->SetShade (r, g, b);
	}
}

