/*
** curl_loader.h
**
**
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

#ifdef RUNTIME_CURL

bool IsCurlLoaded();
bool LoadCurl();

#else

#define IsCurlLoaded() true
#define LoadCurl() true

#endif
