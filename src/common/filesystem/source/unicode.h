/*
** unicode.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

#include <stdint.h>
#include <vector>
namespace FileSys {
	
void utf16_to_utf8(const unsigned short* in, std::vector<char>& buffer);
void ibm437_to_utf8(const char* in, std::vector<char>& buffer);
char *tolower_normalize(const char *str);
bool unicode_validate(const char* str);

}
