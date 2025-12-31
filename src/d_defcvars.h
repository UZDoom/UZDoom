/*
** d_defcvars.h
**
** defcvars loader split from d_main.cpp
**
**---------------------------------------------------------------------------
**
** Copyright 2021 Rachael Alexanderson
** Copyright 2021-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#define SHOULD_BLACKLIST(name) \
	if (#name[0]==CurrentFindCVar[0]) \
		if (CurrentFindCVar.Compare(#name) == 0) \
			blacklisted = true;

