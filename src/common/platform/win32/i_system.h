/*
** i_system.h
**
** Timers, pre-console output, IWAD selection, and misc system routines.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2007-2012 Skulltag Development Team
** Copyright 2007-2016 Zandronum Development Team
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
** 4. Redistributions in any form must be accompanied by information on how to
**    obtain complete source code for the software and any accompanying software
**    that uses the software. The source code must either be included in the
**    distribution or be available for no more than the cost of distribution plus
**    a nominal fee, and must be freely redistributable under reasonable
**    conditions. For an executable file, complete source code means the source
**    code for all modules it contains. It does not include source code for
**    modules or files that typically accompany the major components of the
**    operating system on which the executable file runs.
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

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "basics.h"
#include <thread>
#include "tarray.h"
#include "zstring.h"
#include "utf8.h"

struct WadStuff;
struct FStartupSelectionInfo;

// [RH] Detects the OS the game is running under.
FString I_DetectOS (void);

// Called by DoomMain.
void CalculateCPUSpeed (void);

// Return a seed value for the RNG.
unsigned int I_MakeRNGSeed();


void I_StartFrame (void);
void I_StartTic (void);

// Set the mouse cursor. The texture must be 32x32.
class FGameTexture;
bool I_SetCursor(FGameTexture *cursor);

// Repaint the pre-game console
void I_PaintConsole (void);

// Print a console string
void I_PrintStr (const char *cp);

// Set the title string of the startup window
void I_SetIWADInfo ();

// Pick from multiple IWADs to use
bool HoldingQueryKey(const char* key);
bool I_PickIWad(bool showwin, FStartupSelectionInfo& info);

// The ini could not be saved at exit
bool I_WriteIniFailed (const char* filename);

// [RH] Checks the registry for Steam's install path, so we can scan its
// directories for IWADs if the user purchased any through Steam.
FString I_GetSteamPath();

// [GZ] Same deal for GOG paths
TArray<FString> I_GetGogPaths();

// Again for the Bethesda.net Launcher path
TArray<FString> I_GetBethesdaPath();

// Damn Microsoft for doing Get/SetWindowLongPtr half-assed. Instead of
// giving them proper prototypes under Win32, they are just macros for
// Get/SetWindowLong, meaning they take LONGs and not LONG_PTRs.
#ifdef _WIN64
typedef long long WLONG_PTR;
#elif _MSC_VER
typedef _W64 long WLONG_PTR;
#else
typedef long WLONG_PTR;
#endif

// Wrapper for GetLongPathName
FString I_GetLongPathName(const FString &shortpath);

// Mirror WIN32_FIND_DATAA in <winbase.h>
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

int I_GetNumaNodeCount();
int I_GetNumaNodeThreadCount(int numaNode);
void I_SetThreadNumaNode(std::thread &thread, int numaNode);

void I_OpenShellFolder(const char*);
FString I_GetCWD();
bool I_ChDir(const char* path);

#endif
