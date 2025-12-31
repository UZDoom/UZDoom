/*
** g_input.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
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

// These were in i_input.h, which differed between platforms and on Windows caused problems with its 
// inclusion of system specific data, so it has been separated into this platform independent file.
void I_PutInClipboard (const char *str);
FString I_GetFromClipboard (bool use_primary_selection);
void I_SetMouseCapture();
void I_ReleaseMouseCapture();
void I_GetEvent();
