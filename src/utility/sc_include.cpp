/*
** sc_include.cpp
**
** Helper for resolving include paths for script files
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// HEADER FILES ------------------------------------------------------------

#include "filesystem.h"
#include "sc_include.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// ResolveIncludePath
//
// MH 20251123
//    Adapted from corresponding file in zcc_parser.cpp
//    Resolves SNDINFO include paths.
//    Note that including across archive boundaries is not supported.
//
// Sal
//    Moved to its own file, otherwise I was going to just
//    copy+paste it exactly for TERRAIN includes!
//
//==========================================================================

FString SC_ResolveIncludePath(int includingLump, const char* includedFile)
{
	// Get full path of including file and convert included file to FString
	FString includer = FString(fileSystem.GetFileFullName(includingLump, true));
	FString included = FString(includedFile);

	// Strip any redundant "./" from included
	// Includes shall be relative to parent directory of the including file
	if (included.IndexOf("./") == 0)
	{
		included = included.Mid(2);
	}

	// Remove file name portion from includer
	FString incDir = FString("");
	auto includer_slash_index = includer.LastIndexOf("/");
	if (includer_slash_index != -1)
	{
		incDir = includer.Mid(0, includer_slash_index);
	}

	// Handle .. references
	if (included.IndexOf("../") == 0)
	{
		bool pathOk = true;

		while (included.IndexOf("../") == 0) // go back one folder for each '..'
		{
			included = included.Mid(3);
			auto slash_index = incDir.LastIndexOf("/");
			if (slash_index != -1)
			{
				incDir = incDir.Mid(0, slash_index);
			}
			else if (incDir.IsNotEmpty())
			{
				incDir = "";
			}
			else
			{
				pathOk = false;
				break;
			}
		}

		if (pathOk)
		{
			if (incDir.IsNotEmpty())
			{
				included = incDir + "/" + included;
			}
			return included;
		}

		// Return unmodified if failed
		// S_AddSNDINFO will report a "not found" error when trying to use it
		return FString(includedFile);
	}

	// Handle include file relative
	if (incDir.IsNotEmpty())
	{
	   included = incDir + "/" + included;
	}

	// Completed
	return included;
}
