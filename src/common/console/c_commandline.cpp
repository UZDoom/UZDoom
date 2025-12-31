/*
** c_commandline.cpp
**
** Functions for executing console commands and aliases
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2009-2016 Christoph Oelckers
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

// HEADER FILES ------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "c_commandline.h"
#include "c_cvars.h"
#include "v_text.h"

// ParseCommandLine
//
// Parse a command line (passed in args). If argc is non-NULL, it will
// be set to the number of arguments. If argv is non-NULL, it will be
// filled with pointers to each argument; argv[0] should be initialized
// to point to a buffer large enough to hold all the arguments. The
// return value is the necessary size of this buffer.
//
// Special processing:
//   Inside quoted strings, \" becomes just "
//                          \\ becomes just a single backslash          
//							\c becomes just TEXTCOLOR_ESCAPE
//   $<cvar> is replaced by the contents of <cvar>

static size_t ParseCommandLine(const char* args, int* argc, char** argv, bool no_escapes)
{
	int count;
	char* buffstart;
	char* buffplace;

	count = 0;
	buffstart = NULL;
	if (argv != NULL)
	{
		buffstart = argv[0];
	}
	buffplace = buffstart;

	for (;;)
	{
		while (*args <= ' ' && *args)
		{ // skip white space
			args++;
		}
		if (*args == 0)
		{
			break;
		}
		else if (*args == '\"')
		{ // read quoted string
			char stuff;
			if (argv != NULL)
			{
				argv[count] = buffplace;
			}
			count++;
			args++;
			do
			{
				stuff = *args++;
				if (!no_escapes && stuff == '\\' && *args == '\"')
				{
					stuff = '\"', args++;
				}
				else if (!no_escapes && stuff == '\\' && *args == '\\')
				{
					args++;
				}
				else if (!no_escapes && stuff == '\\' && *args == 'c')
				{
					stuff = TEXTCOLOR_ESCAPE, args++;
				}
				else if (stuff == '\"')
				{
					stuff = 0;
				}
				else if (stuff == 0)
				{
					args--;
				}
				if (argv != NULL)
				{
					*buffplace = stuff;
				}
				buffplace++;
			} while (stuff);
		}
		else
		{ // read unquoted string
			const char* start = args++, * end;
			FBaseCVar* var;
			UCVarValue val;

			while (*args && *args > ' ' && *args != '\"')
				args++;
			if (*start == '$' && (var = FindCVarSub(start + 1, int(args - start - 1))))
			{
				val = var->GetGenericRep(CVAR_String);
				start = val.String;
				end = start + strlen(start);
			}
			else
			{
				end = args;
			}
			if (argv != NULL)
			{
				argv[count] = buffplace;
				while (start < end)
					*buffplace++ = *start++;
				*buffplace++ = 0;
			}
			else
			{
				buffplace += end - start + 1;
			}
			count++;
		}
	}
	if (argc != NULL)
	{
		*argc = count;
	}
	return (buffplace - buffstart);
}

FCommandLine::FCommandLine (const char *commandline, bool no_escapes)
{
	cmd = commandline;
	_argc = -1;
	_argv = NULL;
	noescapes = no_escapes;
}

FCommandLine::~FCommandLine ()
{
	if (_argv != NULL)
	{
		delete[] _argv;
	}
}

void FCommandLine::Shift()
{
	// Only valid after _argv has been filled.
	for (int i = 1; i < _argc; ++i)
	{
		_argv[i - 1] = _argv[i];
	}
}

int FCommandLine::argc ()
{
	if (_argc == -1)
	{
		argsize = ParseCommandLine (cmd, &_argc, NULL, noescapes);
	}
	return _argc;
}

const char *FCommandLine::operator[] (int i)
{
	if (_argv == NULL)
	{
		int count = argc();
		_argv = new char *[count + (argsize+sizeof(char*)-1)/sizeof(char*)];
		_argv[0] = (char *)_argv + count*sizeof(char *);
		ParseCommandLine (cmd, NULL, _argv, noescapes);
	}
	return _argv[i];
}
