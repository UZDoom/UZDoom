/*
** c_tabcomplete.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2013-2016 Christoph Oelckers
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

#pragma once

void C_AddTabCommand (const char *name);
void C_RemoveTabCommand (const char *name);
void C_ClearTabCommands();		// Removes all tab commands
void C_TabComplete(bool goForward);
bool C_TabCompleteList();
extern bool TabbedLast;		// True if last key pressed was tab
extern bool TabbedList;		// True if tab list was shown
