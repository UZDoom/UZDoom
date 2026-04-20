/*
** a_flashfader.cpp
**
** User settable screen blends
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

#include "a_sharedglobal.h"
#include "d_player.h"
#include "serializer.h"
#include "serialize_obj.h"
#include "g_levellocals.h"

IMPLEMENT_CLASS(DFlashFader, false, true)

IMPLEMENT_POINTERS_START(DFlashFader)
	IMPLEMENT_POINTER(ForWho)
IMPLEMENT_POINTERS_END

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::Construct (float r1, float g1, float b1, float a1,
						  float r2, float g2, float b2, float a2,
						  float time, AActor *who, bool terminate)
{
	TotalTics = (int)(time*TICRATE);
	RemainingTics = TotalTics;
	ForWho = who;
	Blends[0][0]=r1; Blends[0][1]=g1; Blends[0][2]=b1; Blends[0][3]=a1;
	Blends[1][0]=r2; Blends[1][1]=g2; Blends[1][2]=b2; Blends[1][3]=a2;
	Terminate = terminate;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::OnDestroy ()
{
	if (Terminate) Blends[1][3] = 0.f; // Needed in order to cancel out the secondary fade.
	SetBlend (1.f);
	Super::OnDestroy();
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::Serialize(FSerializer &arc)
{
	Super::Serialize (arc);
	arc("totaltics", TotalTics)
		("remainingtics", RemainingTics)
		("forwho", ForWho)
		.Array("blends", Blends[0], 8);
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::Tick ()
{
	if (ForWho == NULL || ForWho->player == NULL)
	{
		Destroy ();
		return;
	}
	if (--RemainingTics <= 0)
	{
		SetBlend (1.f);
		Destroy ();
		return;
	}
	SetBlend (1.f - (float)RemainingTics / (float)TotalTics);
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::SetBlend (float time)
{
	if (ForWho == NULL || ForWho->player == NULL)
	{
		return;
	}
	player_t *player = ForWho->player;
	float iT = 1.f - time;
	player->BlendR = Blends[0][0]*iT + Blends[1][0]*time;
	player->BlendG = Blends[0][1]*iT + Blends[1][1]*time;
	player->BlendB = Blends[0][2]*iT + Blends[1][2]*time;
	player->BlendA = Blends[0][3]*iT + Blends[1][3]*time;
}

//----------------------------------------------------------------------------
//
//
//
//----------------------------------------------------------------------------

void DFlashFader::Cancel ()
{
	RemainingTics = 0;
	Blends[1][3] = 0.f;
}
