/*
** engineerrors.cpp
**
** Contains error classes that can be thrown around
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2020 Christoph Oelckers
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

bool gameisdead;

#include <cstdarg>

#ifdef _WIN32
#include <windows.h>
#include "zstring.h"
void I_DebugPrint(const char *cp)
{
	if (IsDebuggerPresent())
	{
		auto wstr = WideString(cp);
		OutputDebugStringW(wstr.c_str());
	}
}

void I_DebugPrintf(const char *fmt,...)
{
	if (IsDebuggerPresent())
	{
		va_list args;
		va_start(args, fmt);

		FString s;
		s.VFormat(fmt, args);

		va_end(args);

		auto wstr = WideString(s.GetChars());
		OutputDebugStringW(wstr.c_str());
	}
}
#else
void I_DebugPrint(const char *cp)
{
}

void I_DebugPrintf(const char *fmt,...)
{
}
#endif

#include "engineerrors.h"

//==========================================================================
//
// I_Error
//
// Throw an error that will send us to the console if we are far enough
// along in the startup process.
//
//==========================================================================

[[noreturn]] void I_Error(const char *error, ...)
{
	va_list argptr;
	char errortext[MAX_ERRORTEXT];

	va_start(argptr, error);
	vsnprintf(errortext, MAX_ERRORTEXT, error, argptr);
	va_end(argptr);
	I_DebugPrint(errortext);

	throw CRecoverableError(errortext);
}

//==========================================================================
//
// I_FatalError
//
// Throw an error that will end the game.
//
//==========================================================================
extern FILE *Logfile;

[[noreturn]] void I_FatalError(const char *error, ...)
{
	static bool alreadyThrown = false;
	gameisdead = true;

	if (!alreadyThrown)		// ignore all but the first message -- killough
	{
		alreadyThrown = true;
		char errortext[MAX_ERRORTEXT];
		va_list argptr;
		va_start(argptr, error);
		vsnprintf(errortext, MAX_ERRORTEXT, error, argptr);
		va_end(argptr);
		I_DebugPrint(errortext);

		// Record error to log (if logging)
		if (Logfile)
		{
			fprintf(Logfile, "\n**** DIED WITH FATAL ERROR:\n%s\n", errortext);
			fflush(Logfile);
		}

		throw CFatalError(errortext);
	}
	std::terminate(); // recursive I_FatalErrors must immediately terminate.
}

