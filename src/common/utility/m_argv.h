/*
** m_argv.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __M_ARGV_H__
#define __M_ARGV_H__

#include "name.h"
#include "tarray.h"
#include "zstring.h"

class FArg {
friend class FArgs;

public:
	FArg(
		const char * name,
		const char * section,
		const char * summary,
		const char * usage,
		const char * details,
		bool advanced = false
	);

protected:
	FString name;
	FName section;
	FString summary, usage, details;
	bool advanced;

    static TMap<FString, FArg *>& Available() {
		static TMap<FString, FArg *> Map;
		return Map;
	}
};

#define EXTERN_FARG(argument) \
	extern FArg FArg_##argument

#define FARG_CUSTOM(name, argument, section, advanced, summary, usage, details) \
	FArg FArg_##name (argument, section, summary, usage, details, advanced)

#define FARG(argument, section, summary, usage, details) \
	FARG_CUSTOM(argument, "-" #argument, section, false, summary, usage, details)

#define FARG_ADVANCED(argument, section, usage, details) \
	FARG_CUSTOM(argument, "-" #argument, section, true, details, usage, "")

//
// MISC
//
class FArgs
{
public:

	typedef TIterator<FString>                  iterator;
	typedef TIterator<const FString>            const_iterator;
	typedef FString								value_type;

	iterator begin()
	{
		return Argv.begin();
	}
	const_iterator begin() const
	{
		return Argv.begin();
	}
	const_iterator cbegin() const
	{
		return Argv.begin();
	}

	iterator end()
	{
		return Argv.end();
	}
	const_iterator end() const
	{
		return Argv.end();
	}
	const_iterator cend() const
	{
		return Argv.end();
	}

	FArgs();
	FArgs(const FArgs &args);
	FArgs(int argc, char **argv);
	FArgs(int argc, const char** argv);
	FArgs(int argc, FString *argv);

	FArgs &operator=(const FArgs &other);
	const FString& operator[](size_t index) { return Argv[index]; }

	void AppendRawArg(FString arg);
	void AppendRawArgsString(FString argv);

	void AppendArg(const FArg arg);
	void RemoveArg(int argindex);
	void RemoveArgs(const FArg check);
	void CollectFiles(const FArg arg, const char *extension);
	FArgs *GatherFiles(const FArg arg) const;

	int CheckParm(const FArg check, int start=1) const;	// Returns the position of the given parameter in the arg list (0 if not found).
	int CheckParm(const FArg ** check, int start = 1) const;	// Returns the position of the given parameter in the arg list (0 if not found). Allows checking for multiple switches
	int CheckParmList(const FArg check, FString **strings, int start=1) const;
	const char *CheckValue(const FArg check) const;

	const char *GetArg(int arg) const;
	FString *GetArgList(int arg) const;
	FString TakeValue(const FArg check);
	int NumArgs() const;

	static void PrintHelpMessage(bool full);

private:
	void FlushArgs();
	void SetArg(int argnum, const char *arg);
	void SetRawArgs(int argc, char **argv);
	void AppendRawArgs(int argc, const FString *argv);
	void CollectFiles(const char *finalname, const char** arg, const char* extension);

	TArray<FString>& Array() { return Argv; }
	TArray<FString> Argv;

	static int TestArgList(const FArg ** check, const char * str);
};

extern FArgs *Args;

#endif //__M_ARGV_H__
