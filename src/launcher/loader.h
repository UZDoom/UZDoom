/*
** loader.h
**
** Header for loader.cpp
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
#include "gstrings.h"
#include <string>

enum importStatus
{
	IMPORT_IWAD_SUCCESS,
	IMPORT_PWAD_SUCCESS,
	IMPORT_FAIL,
	IMPORT_ARCHIVE_FAIL,
	IMPORT_DUPLICATE,
	IMPORT_CANCELLED
};


class Loader
{
  public:

	// Pass the filepath obtained from File Picker to be processed then return status after profile creation (if)
	static importStatus ProcessArchive(const std::filesystem::path &filePath);
	static importStatus ProcessWad(const std::filesystem::path &filePath);

	// Extracts zip using build-in miniz.
	static bool ExtractArchive(const std::filesystem::path &archivePath, const std::filesystem::path &targetDir);

	// Generates a standard timestamp string for folder names
	static std::string GenerateTimestampString();
};
