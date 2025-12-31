/*
** d_netinf.h
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
