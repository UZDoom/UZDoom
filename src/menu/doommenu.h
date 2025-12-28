/*
** doommenu.h
**
** Menu base class and global interface
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
#include "menu.h"
#include "savegamemanager.h"

struct FNewGameStartup
{
	bool hasPlayerClass;
	FString PlayerClass;
	int Episode;
	int Skill;
};

extern FNewGameStartup NewGameStartupInfo;
void M_StartupEpisodeMenu(FNewGameStartup *gs);
void M_StartupSkillMenu(FNewGameStartup *gs);
void M_CreateGameMenus();
void SetDefaultMenuColors();
void OnMenuOpen(bool makeSound);

class FSavegameManager : public FSavegameManagerBase
{
	void PerformSaveGame(const char *fn, const char *sgdesc) override;
	void PerformLoadGame(const char *fn, bool) override;
	FString ExtractSaveComment(FSerializer &arc) override;
	FString BuildSaveName(const char* prefix, int slot) override;
	void ReadSaveStrings() override;
};

extern FSavegameManager savegameManager;

