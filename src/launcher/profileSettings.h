/*
** profileSettings.h
**
** Contains header for profileSettings.cpp
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "const.h"
#include "profile.h"
#include <nfd.h>
#include <string>
#include <vector>

class ProfileSettings
{
  public:
	// is called from outside
	void Draw(bool *p_open, Profile *currEdit, const std::string &profilePath);
	
	// Helper for file picking (uses library: nativefiledialog-extended)
	static std::string OpenPathPicker(std::filesystem::path defaultPath, bool isFolder,
	                                  const std::vector<nfdfilteritem_t> &filters);

  private:
	// Tab rendering functions
	void DrawGeneralTab(Profile *currEdit);
	void DrawLaunchTab(Profile *currEdit);
	void DrawFilesTab(Profile *currEdit);
	void DrawOutputTab(Profile *currEdit);
	void DrawAdvancedTab(Profile *currEdit);

	// Modals
	void DrawFlagEditorModal(Profile *currEdit);

	// Internal state for the flag editor modals
	int         tempFlags[3]    = {0, 0, 0};
	int         tempFlagsCount  = 0;
	bool        isGameplayFlags = true;  // true for Gameplay (DMFlags), false for CompatFlags
	bool        forceDmFlags    = false; // specific to Gameplay flags
	std::string currentModalTitle;
	bool        openFlagEditor = false;
};
