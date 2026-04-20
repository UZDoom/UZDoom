/*
** c_commandline.h
**
**
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

#pragma once

class FConfigFile;
struct CCmdFuncParm;


// Class that can parse command lines
class FCommandLine
{
	friend int C_RegisterFunction(const char* name, const char* help, int (*func)(CCmdFuncParm const* const));
public:
	FCommandLine (const char *commandline, bool no_escapes = false);
	~FCommandLine ();
	int argc ();
	const char *operator[] (int i);
	const char *args () { return cmd; }
	void Shift();

private:
	const char *cmd;
	bool noescapes;
	int _argc;
	char **_argv;
	size_t argsize;
};

