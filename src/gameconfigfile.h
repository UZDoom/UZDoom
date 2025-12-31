/*
** gameconfigfile.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2007-2016 Christoph Oelckers
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

#ifndef __GAMECONFIGFILE_H__
#define __GAMECONFIGFILE_H__

#include "doomtype.h"
#include "configfile.h"
#include "files.h"

class FArgs;
class FIWadManager;

class FGameConfigFile : public FConfigFile
{
public:
	FGameConfigFile ();
	~FGameConfigFile ();

	void DoAutoloadSetup (FIWadManager *iwad_man);
	void DoGlobalSetup ();
	void DoGameSetup (const char *gamename);
	void DoKeySetup (const char *gamename);
	void DoModSetup (const char *gamename);
	void ArchiveGlobalData ();
	void ArchiveGameData (const char *gamename);
	void AddAutoexec (FArgs *list, const char *gamename);
	FString GetConfigPath (bool tryProg);
	void ReadNetVars ();

protected:
	void WriteCommentHeader (FileWriter *file) const;
	void CreateStandardAutoExec (const char *section, bool start);

private:
	void SetRavenDefaults (bool isHexen);
	void SetStrifeDefaults ();
	void ReadCVars (unsigned flags);

	bool bModSetup;
	bool b226ResetGamepad;

	char section[64];
	char *subsection;
	size_t sublen;
};

extern FGameConfigFile *GameConfig;

#endif //__GAMECONFIGFILE_H__
