/*
** m_misc.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __M_MISC__
#define __M_MISC__

#include "basics.h"
#include "zstring.h"

class FConfigFile;
class FGameConfigFile;
class FIWadManager;

extern FGameConfigFile *GameConfig;

void M_FindResponseFile (void);

// [RH] M_ScreenShot now accepts a filename parameter.
//		Pass a NULL to get the original behavior.
void M_ScreenShot (const char *filename);

void M_LoadDefaults ();

bool M_SaveDefaults (const char *filename);
void M_SaveCustomKeys (FConfigFile *config, char *section, char *subsection, size_t sublen);


#include "i_specialpaths.h"
#endif
