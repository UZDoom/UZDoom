/*
** cmdlib.h
**
** Misc utilities (mostly file handling stuff)
**
**---------------------------------------------------------------------------
**
** Copyright 1999-2016 Marisa Heit
** Copyright 2019 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
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

#ifndef __CMDLIB__
#define __CMDLIB__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include "zstring.h"
#include "files.h"

#if !defined(GUID_DEFINED)
#define GUID_DEFINED
typedef struct _GUID
{
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t	Data4[8];
} GUID;
#endif

template <typename T, size_t N>
char(&_ArraySizeHelper(T(&array)[N]))[N];

#define countof( array ) (sizeof( _ArraySizeHelper( array ) )) 

// the dec offsetof macro doesnt work very well...
#define myoffsetof(type,identifier) ((size_t)&((type *)alignof(type))->identifier - alignof(type))

bool FileExists (const char *filename);
FString RecursiveFileExists(const FString& path, const FString& file);
inline bool FileExists(const FString& filename)
{
	return FileExists(filename.GetChars());
}
bool FileReadable (const char *filename);
bool DirExists(const char *filename);
bool DirEntryExists (const char *pathname, bool *isdir = nullptr);
bool GetFileInfo(const char* pathname, size_t* size, time_t* time);

extern	FString progdir;

void	FixPathSeperator (char *path);
static void	inline FixPathSeperator (FString &path) { path.ReplaceChars('\\', '/'); }

void 	DefaultExtension (FString &path, const char *extension, bool forcebackslash = false);
void NormalizeFileName(FString &str);

FString	ExtractFilePath (const char *path, bool forcebackslash = false);
FString	ExtractFileBase (const char *path, bool keep_extension=false, bool forcebackslash = false);
FString StripExtension(const char* path);
void SplitPath(const char* path, FString& directory, FString& base, FString& ext, bool forcebackslash = false);

struct FScriptPosition;
bool	IsNum (const char *str);		// [RH] added

char	*copystring(const char *s);

bool CheckWildcards (const char *pattern, const char *text);

void FormatGUID (char *buffer, size_t buffsize, const GUID &guid);

const char *myasctime ();

int strbin (char *str);
FString strbin1 (const char *start);

void CreatePath(const char * fn);
void RemoveFile(const char* file);
int RemoveDir(const char* file);

FString ExpandEnvVars(const char *searchpathstring);
FString NicePath(const char *path);

bool IsAbsPath(const char*);
FString M_ZLibError(int zerrnum);

inline constexpr int32_t Scale(int32_t a, int32_t b, int32_t c)
{
	return (int32_t)(((int64_t)a * b) / c);
}

inline constexpr double Scale(double a, double b, double c)
{
	return (a * b) / c;
}

struct MD5Context;

void md5Update(FileReader& file, MD5Context& md5, unsigned len);
void uppercopy(char* to, const char* from);
FString GetStringFromLump(int lump, bool zerotruncate = true);

inline void fillshort(void* buff, size_t count, uint16_t clear)
{
	int16_t* b2 = (int16_t*)buff;
	for (size_t i = 0; i < count; ++i)
	{
		b2[i] = clear;
	}
}

template<typename T> inline constexpr int Sgn(const T& val) { return (val > 0) - (val < 0); }


inline int sizeToBits(int w)
{
	int j = 15;

	while ((j > 1) && ((1 << j) > w))
		j--;
	return j;
}


#endif
