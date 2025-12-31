/*
** actorptrselect.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2011 FDARI
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

#pragma once

//==========================================================================
//
// Standard pointer acquisition functions
//
// Possible effective results at run-time
//   assigntovariable = NULL (or a RETURN statement is issued)
//   P_BulletSlope(pointer_owner, &temporary), assigntovariable = temporary
//   assigntovariable = pointer_owner->target or ...->master or ...->tracer
//
//==========================================================================

class AActor;

// Pointer selectors (enum)

enum AAPTR
{
	AAPTR_DEFAULT = 0,
	AAPTR_NULL = 0x1,
	AAPTR_TARGET = 0x2,
	AAPTR_MASTER = 0x4,
	AAPTR_TRACER = 0x8,

	AAPTR_PLAYER_GETTARGET = 0x10,
	AAPTR_PLAYER_GETCONVERSATION = 0x20,

	AAPTR_PLAYER1 = 0x40,
	AAPTR_PLAYER2 = 0x80,
	AAPTR_PLAYER3 = 0x100,
	AAPTR_PLAYER4 = 0x200,
	AAPTR_PLAYER5 = 0x400,
	AAPTR_PLAYER6 = 0x800,
	AAPTR_PLAYER7 = 0x1000,
	AAPTR_PLAYER8 = 0x2000,

	AAPTR_FRIENDPLAYER = 0x4000,
	AAPTR_GET_LINETARGET = 0x8000,

	AAPTR_PLAYER_SELECTORS =
		AAPTR_PLAYER_GETTARGET|AAPTR_PLAYER_GETCONVERSATION,

	AAPTR_GENERAL_SELECTORS =
		AAPTR_TARGET|AAPTR_MASTER|AAPTR_TRACER|AAPTR_FRIENDPLAYER|AAPTR_GET_LINETARGET,

	AAPTR_STATIC_SELECTORS =
		AAPTR_PLAYER1|AAPTR_PLAYER2|AAPTR_PLAYER3|AAPTR_PLAYER4|
		AAPTR_PLAYER5|AAPTR_PLAYER6|AAPTR_PLAYER7|AAPTR_PLAYER8|
		AAPTR_NULL

};

enum EPTRClientSideState
{
	CSPTR_IGNORE = -1,
	CSPTR_SERVER,
	CSPTR_CLIENTSIDE,
};

/*
	COPY_AAPTR

	Result overview in order of priority:

	1. Caller is player and a player specific selector is specified: Player specific selector is used.
	2. Caller is non-null and a general actor selector is specified: General actor selector is used.
	3. A static actor selector is specified: Static actor selector is used.
	4. The origin actor is used.

	Only one selector of each type can be used.
*/

struct FLevelLocals;
AActor *COPY_AAPTR(AActor *origin, int selector);
AActor *COPY_AAPTREX(FLevelLocals *Level, AActor *origin, int selector, EPTRClientSideState clientSide);
enum PTROP
{
	PTROP_UNSAFETARGET = 1,
	PTROP_UNSAFEMASTER = 2,
	PTROP_NOSAFEGUARDS = PTROP_UNSAFETARGET|PTROP_UNSAFEMASTER
};


// [FDARI] Exported logic for guarding against loops in Target (for missiles) and Master (for all) chains.
// It is called from multiple locations.
// The code may be in need of optimisation.


//==========================================================================
//
// Checks whether this actor is a missile
// Unfortunately this was buggy in older versions of the code and many
// released DECORATE monsters rely on this bug so it can only be fixed
// with an optional flag
//
//==========================================================================

void VerifyTargetChain(AActor *self, bool preciseMissileCheck=true);
void VerifyMasterChain(AActor *self);
void ASSIGN_AAPTR(AActor *toActor, int toSlot, AActor *ptr, int flags) ;

