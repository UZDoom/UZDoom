/*
** starter.h
**
** The Qt6 Laucher STARTS HERE
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

struct FStartupSelectionInfo;

class Starter
{
public:
	static bool ExecModal(FStartupSelectionInfo& info);
};
