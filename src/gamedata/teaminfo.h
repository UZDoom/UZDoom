/*
** teaminfo.h
**
** Parses TEAMINFO and manages teams.
**
**---------------------------------------------------------------------------
**
** Copyright 2007-2009 Christopher Westley
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

#ifndef __TEAMINFO_H__
#define __TEAMINFO_H__

#include "doomtype.h"
#include "sc_man.h"

const int TEAM_NONE = 255;
const int TEAM_MAXIMUM = 16;

class FTeam
{
public:
	FTeam ();
	static void ParseTeamInfo ();
	static bool IsValid (unsigned int uiTeam);
	static bool ChangeTeam(unsigned int pNum, unsigned int newTeam);

	const char *GetName () const;
	int GetPlayerColor () const;
	int GetTextColor () const;
	const FString& GetLogo () const;
	bool GetAllowCustomPlayerColor () const;

	int			m_iPlayerCount;
	int			m_iScore;
	int			m_iPresent;
	int			m_iTies;

private:
	static void ParseTeamDefinition (FScanner &Scan);
	static void ClearTeams ();

public:	// needed for script access.
	FString		m_Name;
private:
	int			m_iPlayerColor;
	FString		m_TextColor;
	FString		m_Logo;
	bool		m_bAllowCustomPlayerColor;
};

extern TArray<FTeam>	Teams;

#endif
