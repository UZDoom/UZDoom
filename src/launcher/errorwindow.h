/*
** errorwindow.h
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ErrorWindow
{
  public:
	// Initializes a standalone SDL+ImGui window to display the crash reporter
	static bool ExecModal(const std::string &text, const std::string &log, std::vector<uint8_t> minidump);
};
