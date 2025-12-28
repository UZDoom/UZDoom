/*
** stb_include.h
**
** parse and process #include directives
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Jay
** Copyright 2025 GZDoom Maintainers and Contributors
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
** stb_include.h - v0.02 - public domain
**
** Credits:
**  Written by Sean Barrett.
**
** Fixes:
**  Michal Klos
**
**---------------------------------------------------------------------------
**
** To build this, in one source file that includes this file do
**      #define STB_INCLUDE_IMPLEMENTATION
**
** This program parses a string and replaces lines of the form
**         #include "foo"
** with the contents of a file named "foo". It also embeds the
** appropriate #line directives. Note that all include files must
** reside in the location specified in the gzdoom filesystem.
**
*/

#ifndef STB_INCLUDE_STB_INCLUDE_H
#define STB_INCLUDE_STB_INCLUDE_H

#include "zstring.h"

// Do include-processing on the string 'str'.
FString stb_include_string(FString str, FString filename, TArray<FString> &filenames, FString &error);

// Load the file 'filename' and do include-processing on the string therein.
FString stb_include_file(FString filename, TArray<FString> &filenames, FString &error);

#endif
