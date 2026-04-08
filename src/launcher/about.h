/*
** about.h
**
** Header for about.cpp
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

class About
{
  public:
	// draw respective dialogs
	static void DrawReleaseNotesDialog(bool *p_open, const std::string &lang);
	static void DrawCreditsDialog(bool *p_open, const std::string &lang);
};
