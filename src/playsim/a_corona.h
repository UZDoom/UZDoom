/*
** a_corona.h
**
** Light Coronas
**
**---------------------------------------------------------------------------
**
** Copyright 2022-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Nash Muhandes, Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "actor.h"

EXTERN_CVAR(Bool, gl_coronas)

class AActor;

#if 0
class ACorona : public AActor
{
	DECLARE_CLASS(ACorona, AActor)

public:
	void Tick();

	float CoronaFade = 0.0f;
};
#endif
