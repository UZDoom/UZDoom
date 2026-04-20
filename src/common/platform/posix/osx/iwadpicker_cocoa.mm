/*
** iwadpicker_cocoa.mm
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2010 Braden Obrzut
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

#include "widgets/launcherwindow.h"

// TODO: get rid of this
int I_PickIWad_Cocoa(FStartupSelectionInfo &info)
{
	return LauncherWindow::ExecModal(info);
}
