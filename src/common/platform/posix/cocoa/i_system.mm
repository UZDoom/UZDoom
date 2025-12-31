/*
** i_system.mm
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2012-2018 Alexey Lysiuk
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

#include "i_common.h"
#include "c_cvars.h"
#include "i_interface.h"

#include <fnmatch.h>
#include <sys/sysctl.h>

#include "i_system.h"
#include "st_console.h"
#include "v_text.h"

CVAR(String, queryiwad_key, "", CVAR_GLOBALCONFIG | CVAR_ARCHIVE); // Currently unimplemented.

EXTERN_CVAR(Bool, longsavemessages)
double PerfToSec, PerfToMillisec;

void CalculateCPUSpeed()
{
	long long frequency;
	size_t size = sizeof frequency;

	if (0 == sysctlbyname("machdep.tsc.frequency", &frequency, &size, nullptr, 0) && 0 != frequency)
	{
		PerfToSec = 1.0 / frequency;
		PerfToMillisec = 1000.0 / frequency;

		if (!batchrun)
		{
			Printf("CPU speed: %.0f MHz\n", 0.001 / PerfToMillisec);
		}
	}
}


void I_SetIWADInfo()
{
	FConsoleWindow::GetInstance().SetTitleText();
}


void I_PrintStr(const char* const message)
{
	FConsoleWindow::GetInstance().AddText(message);

	// Strip out any color escape sequences before writing to output
	char* const copy = new char[strlen(message) + 1];
	const char* srcp = message;
	char* dstp = copy;

	while ('\0' != *srcp)
	{
		if (TEXTCOLOR_ESCAPE == *srcp)
		{
			if ('\0' != srcp[1])
			{
				srcp += 2;
			}
			else
			{
				break;
			}
		}
		else if (0x1d == *srcp || 0x1f == *srcp) // Opening and closing bar character
		{
			*dstp++ = '-';
			++srcp;
		}
		else if (0x1e == *srcp) // Middle bar character
		{
			*dstp++ = '=';
			++srcp;
		}
		else
		{
			*dstp++ = *srcp++;
		}
	}

	*dstp = '\0';

	fputs(copy, stdout);
	delete[] copy;
	fflush(stdout);
}


void Mac_I_FatalError(const char* const message);

void I_ShowFatalError(const char *message)
{
	Mac_I_FatalError(message);
}

bool HoldingQueryKey(const char* key)
{
	// TODO: Implement
	return false;
}

bool I_PickIWad(bool showwin, FStartupSelectionInfo& info)
{
	if (!showwin)
	{
		return true;
	}

	I_SetMainWindowVisible(false);

	extern int I_PickIWad_Cocoa(WadStuff*, int, bool, int);
	const int result = I_PickIWad_Cocoa(&(*info.Wads)[0], (int)info.Wads->Size(), showwin, info.DefaultIWAD);

	I_SetMainWindowVisible(true);

	if (result >= 0)
	{
		info.DefaultIWAD = result;
		return true;
	}

	return false;
}


void I_PutInClipboard(const char* const string)
{
	NSPasteboard* const pasteBoard = [NSPasteboard generalPasteboard];
	NSString* const stringType = NSStringPboardType;
	NSArray* const types = [NSArray arrayWithObjects:stringType, nil];
	NSString* const content = [NSString stringWithUTF8String:string];

	[pasteBoard declareTypes:types
					   owner:nil];
	[pasteBoard setString:content
				  forType:stringType];
}

FString I_GetFromClipboard(bool returnNothing)
{
	if (returnNothing)
	{
		return FString();
	}

	NSPasteboard* const pasteBoard = [NSPasteboard generalPasteboard];
	NSString* const value = [pasteBoard stringForType:NSStringPboardType];

	return FString([value UTF8String]);
}


unsigned int I_MakeRNGSeed()
{
	return static_cast<unsigned int>(arc4random());
}

FString I_GetCWD()
{
	NSString *currentpath = [[NSFileManager defaultManager] currentDirectoryPath];
	return currentpath.UTF8String;
}

bool I_ChDir(const char* path)
{
	return [[NSFileManager defaultManager] changeCurrentDirectoryPath:[NSString stringWithUTF8String:path]];
}

void I_OpenShellFolder(const char* folder)
{
	NSFileManager *filemgr = [NSFileManager defaultManager];
	NSString *currentpath = [filemgr currentDirectoryPath];

	[filemgr changeCurrentDirectoryPath:[NSString stringWithUTF8String:folder]];
	if (longsavemessages)
		Printf("Opening folder: %s\n", folder);
	std::system("open .");
	[filemgr changeCurrentDirectoryPath:currentpath];
}

