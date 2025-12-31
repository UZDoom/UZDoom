/*
** c_dispatch.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
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

#ifndef __C_DISPATCH_H__
#define __C_DISPATCH_H__

#include <string>
#include <vector>
#include <stdint.h>
#include <functional>
#include "c_console.h"
#include "tarray.h"
#include "c_commandline.h"
#include "zstring.h"

class FConfigFile;


// Contains the contents of an exec'ed file
struct FExecList
{
	TArray<FString> Commands;
	TArray<FString> Pullins;

	void AddCommand(const char *cmd, const char *file = nullptr);
	void ExecCommands() const;
	void AddPullins(std::vector<std::string> &wads, FConfigFile *config) const;
};

extern bool ParsingKeyConf, UnsafeExecutionContext;
extern	FString			StoredWarp;			// [RH] +warp at the command line


FExecList *C_ParseCmdLineParams(FExecList *exec);

// Add commands to the console as if they were typed in. Can handle wait
// and semicolon-separated commands. This function may modify the source
// string, but the string will be restored to its original state before
// returning. Therefore, commands passed must not be in read-only memory.
void AddCommandString (const char *text, int keynum=0);

void C_RunDelayedCommands();
void C_ClearDelayedCommands();

// Process a single console command. Does not handle wait.
void C_DoCommand (const char *cmd, int keynum=0);
bool C_IsValidInt(const char* arg, int& value, int base = 10);
bool C_IsValidFloat(const char* arg, double& value);

FExecList *C_ParseExecFile(const char *file, FExecList *source);
void C_SearchForPullins(FExecList *exec, const char *file, class FCommandLine &args);
bool C_ExecFile(const char *file);
void C_ClearDynCCmds();

// Write out alias commands to a file for all current aliases.
void C_ArchiveAliases (FConfigFile *f);

void C_SetAlias (const char *name, const char *cmd);
void C_ClearAliases ();

// build a single string out of multiple strings
FString BuildString (int argc, FString *argv);

typedef std::function<void(FCommandLine & argv, int key)> CCmdRun;;

class FConsoleCommand
{
public:
	FConsoleCommand (const char *name, CCmdRun RunFunc);
	virtual ~FConsoleCommand ();
	virtual bool IsAlias ();
	void PrintCommand();

	virtual void Run (FCommandLine &args, int key);
	static FConsoleCommand* FindByName (const char* name);

	FConsoleCommand *m_Next, **m_Prev;
	FString m_Name;

	enum { HASH_SIZE = 251 };	// Is this prime?

protected:
	FConsoleCommand ();
	bool AddToHash (FConsoleCommand **table);

	CCmdRun m_RunFunc;

};

#define CCMD(n) \
	void Cmd_##n (FCommandLine &, int key); \
	FConsoleCommand Cmd_##n##_Ref (#n, Cmd_##n); \
	void Cmd_##n (FCommandLine &argv, int key)

class FUnsafeConsoleCommand : public FConsoleCommand
{
public:
	FUnsafeConsoleCommand (const char *name, CCmdRun RunFunc)
	: FConsoleCommand (name, RunFunc)
	{
	}

	virtual void Run (FCommandLine &args, int key) override;
};

#define UNSAFE_CCMD(n) \
	static void Cmd_##n (FCommandLine &, int key); \
	static FUnsafeConsoleCommand Cmd_##n##_Ref (#n, Cmd_##n); \
	void Cmd_##n (FCommandLine &argv, int key)

const int KEY_DBLCLICKED = 0x8000;

class FConsoleAlias : public FConsoleCommand
{
public:
	FConsoleAlias (const char *name, const char *command, bool noSave);
	~FConsoleAlias ();
	void Run (FCommandLine &args, int key);
	bool IsAlias ();
	void PrintAlias ();
	void Archive (FConfigFile *f);
	void Realias (const char *command, bool noSave);
	void SafeDelete ();
protected:
	FString m_Command[2];	// Slot 0 is saved to the ini, slot 1 is not.
	bool bDoSubstitution;
	bool bRunning;
	bool bKill;
};

class FUnsafeConsoleAlias : public FConsoleAlias
{
public:
	FUnsafeConsoleAlias (const char *name, const char *command)
	: FConsoleAlias (name, command, true)
	{
	}

	virtual void Run (FCommandLine &args, int key) override;
};

class UnsafeExecutionScope
{
	const bool wasEnabled;

public:
	explicit UnsafeExecutionScope(const bool enable = true)
		: wasEnabled(UnsafeExecutionContext)
	{
		UnsafeExecutionContext = enable;
	}

	~UnsafeExecutionScope()
	{
		UnsafeExecutionContext = wasEnabled;
	}
};



void execLogfile(const char *fn, bool append = false);

enum
{
	CCMD_OK = 0,
	CCMD_SHOWHELP = 1
};

struct CCmdFuncParm
{
	int32_t numparms;
	const char* name;
	const char** parms;
	const char* raw;
};

using CCmdFuncPtr = CCmdFuncParm const* const;

// registers a function
//   name = name of the function
//   help = a short help string
//   func = the entry point to the function
int C_RegisterFunction(const char* name, const char* help, int (*func)(CCmdFuncPtr));


#endif //__C_DISPATCH_H__
