/*
** utf8.h
**
** UTF-8 utilities
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Christoph Oelckers
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
