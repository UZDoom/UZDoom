/*
** sc_include.h
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

#pragma once

#include "basics.h"
#include "zstring.h"

FString SC_ResolveIncludePath(int includingLump, const char* includedFile);
