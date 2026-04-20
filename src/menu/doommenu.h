/*
** doommenu.h
**
** Menu base class and global interface
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2016 Christoph Oelckers
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

