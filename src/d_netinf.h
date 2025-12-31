/*
** d_netinf.h
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __D_NETINFO_H__
#define __D_NETINFO_H__

#include "c_cvars.h"

enum
{
	GENDER_MALE,
	GENDER_FEMALE,
	GENDER_NEUTER,
	GENDER_OBJECT,
	GENDER_MAX
};

int D_GenderToInt (const char *gender);
extern const char *GenderNames[GENDER_MAX];

int D_PlayerClassToInt (const char *classname);

void D_SetupUserInfo (void);

void D_UserInfoChanged (FBaseCVar *info);

bool D_SendServerInfoChange (FBaseCVar *cvar, UCVarValue value, ECVarType type);
bool D_SendServerFlagChange (FBaseCVar *cvar, int bitnum, bool set, bool silent);
void D_DoServerInfoChange (TArrayView<uint8_t>& stream, bool singlebit);

FString D_GetUserInfoStrings(int pnum, bool compact = false);
void D_ReadUserInfoStrings (int player, TArrayView<uint8_t>& stream, bool update);

struct FPlayerColorSet;
void D_GetPlayerColor (int player, float *h, float *s, float *v, FPlayerColorSet **colorset);
void D_PickRandomTeam (int player);
int D_PickRandomTeam ();
class player_t;
int D_GetFragCount (player_t *player);

#endif //__D_CLIENTINFO_H__
