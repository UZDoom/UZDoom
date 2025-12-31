/*
** utf8.h
**
** UTF-8 utilities
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

#include <string>

int utf8_encode(int32_t codepoint, uint8_t *buffer, int *size);
int utf8_decode(const uint8_t *src, int *size);
int GetCharFromString(const uint8_t *&string);
inline int GetCharFromString(const char32_t *&string)
{
	return *string++;
}
const char *MakeUTF8(const char *outline, int *numchars = nullptr);	// returns a pointer to a static buffer, assuming that its caller will immediately process the result. 
const char *MakeUTF8(int codepoint, int *psize = nullptr);

bool myislower(int code);
bool myisupper(int code);
int stripaccent(int code);
int getAlternative(int code);

extern uint16_t win1252map[];
extern uint16_t lowerforupper[65536];
extern uint16_t upperforlower[65536];

// make this only visible on Windows, on other platforms this should not be called.
#ifdef _WIN32
std::wstring WideString(const char*);
#endif
