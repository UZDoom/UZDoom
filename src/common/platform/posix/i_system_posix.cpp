/*
** i_system_posix.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#include <fnmatch.h>

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif // __APPLE__

#include "cmdlib.h"
#include "i_system.h"


bool I_WriteIniFailed(const char * filename)
{
	printf("The config file %s could not be saved:\n%s\n", filename, strerror(errno));
	return false; // return true to retry
}

TArray<FString> I_GetBethesdaPath()
{
	// Bethesda.net Launcher is Windows only at the moment
	return TArray<FString>();
}
