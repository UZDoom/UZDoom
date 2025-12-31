/*
** screenjob.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2020 Christoph Oelckers
** Copyright 2020-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
#include <functional>
#include "dobject.h"
#include "v_2ddrawer.h"
#include "d_eventbase.h"
#include "s_soundinternal.h"
#include "gamestate.h"
#include "zstring.h"
#include "c_cvars.h"

EXTERN_CVAR(Bool, inter_subtitles)

using CompletionFunc = std::function<void(bool)>;

void Job_Init();

enum
{
	SJ_BLOCKUI = 1,
};

enum
{
	ST_VOTE,
	ST_MUST_BE_SKIPPABLE,
	ST_UNSKIPPABLE,
};

struct CutsceneDef
{
	FString video;
	FString function;
	FString soundName;
	int soundID = -1;	// ResID not SoundID!
	int framespersec = 0; // only relevant for ANM.
	bool transitiononly = false; // only play when transitioning between maps, but not when starting on a map or ending a game.

	void Create(DObject* runner);
	bool isdefined() { return video.IsNotEmpty() || function.IsNotEmpty(); }
	int GetSound();
};

void EndScreenJob();
void DeleteScreenJob();
bool ScreenJobResponder(event_t* ev);
bool ScreenJobTick();
void ScreenJobDraw();
bool ScreenJobValidate();

struct CutsceneDef;
bool StartCutscene(const char* s, int flags, const CompletionFunc& completion);
bool StartCutscene(CutsceneDef& cs, int flags, const CompletionFunc& completion_);
bool CanWipe();

VMFunction* LookupFunction(const char* qname, bool validate = true);
void CallCreateFunction(const char* qname, DObject* runner);
DObject* CreateRunner(bool clearbefore = true, int skipType = ST_VOTE);
void AddGenericVideo(DObject* runner, const FString& fn, int soundid, int fps);

struct CutsceneState
{
	DObject* runner;
	PClass* runnerclass;
	PType* runnerclasstype;
	CompletionFunc completion;
};

extern CutsceneState cutscene;
