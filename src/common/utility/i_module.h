/*
** i_module.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Braden Obrzut
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

#include <assert.h>
#include <string>
#include <initializer_list>

/* FModule Run Time Library Loader
 *
 * This provides an interface for loading optional dependencies or detecting
 * version specific symbols at run time.  These classes largely provide an
 * interface for statically declaring the symbols that are going to be used
 * ahead of time, thus should not be used on the stack or as part of a
 * dynamically allocated object.  The procedure templates take the FModule
 * as a template argument largely to make such use of FModule awkward.
 *
 * Declared procedures register themselves with FModule and the module will not
 * be considered loaded unless all required procedures can be resolved.  In
 * order to remove the need for boilerplate code, optional procedures do not
 * enforce the requirement that the value is null checked before use.  As a
 * debugging aid debug builds will check that operator bool was called at some
 * point, but this is just a first order sanity check.
 */

class FModule;
class FStaticModule;

template<FModule &Module, typename Proto>
class TOptProc;

template<FModule &Module, typename Proto>
class TReqProc;

template<FStaticModule &Module, typename Proto, Proto Sym>
class TStaticProc;

class FModule
{
	template<FModule &Module, typename Proto>
	friend class TOptProc;
	template<FModule &Module, typename Proto>
	friend class TReqProc;

	struct StaticProc
	{
		void *Call;
		const char* Name;
		StaticProc *Next;
		bool Optional;
	};

	void RegisterStatic(StaticProc &proc)
	{
		proc.Next = reqSymbols;
		reqSymbols = &proc;
	}

	void *handle = nullptr;

	// Debugging aid
	const char *name;

	// Since FModule is supposed to be statically allocated it is assumed that
	// reqSymbols will be initialized to nullptr avoiding initialization order
	// problems with declaring procedures.
	StaticProc *reqSymbols;

	bool Open(const char* lib);
	void *GetSym(const char* name);

public:
	template<FModule &Module, typename Proto, Proto Sym>
	using Opt = TOptProc<Module, Proto>;
	template<FModule &Module, typename Proto, Proto Sym>
	using Req = TReqProc<Module, Proto>;

	FModule(const char* name) : name(name) {};
	~FModule() { Unload(); }

	// Load a shared library using the first library name which satisfies all
	// of the required symbols.
	bool Load(std::initializer_list<const char*> libnames);
	void Unload();

	bool IsLoaded() const { return handle != nullptr; }
};

// Null version of FModule which satisfies the API so the same code can be used
// for run time and compile time linking.
class FStaticModule
{
	template<FStaticModule &Module, typename Proto, Proto Sym>
	friend class TStaticProc;

	const char *name;
public:
	template<FStaticModule &Module, typename Proto, Proto Sym>
	using Opt = TStaticProc<Module, Proto, Sym>;
	template<FStaticModule &Module, typename Proto, Proto Sym>
	using Req = TStaticProc<Module, Proto, Sym>;

	FStaticModule(const char* name) : name(name) {};

	bool Load(std::initializer_list<const char*> libnames) { return true; }
	void Unload() {}

	bool IsLoaded() const { return true; }
};

// Allow FModuleMaybe<DYN_XYZ> to switch based on preprocessor flag.
// Use FModuleMaybe<DYN_XYZ>::Opt and FModuleMaybe<DYN_XYZ>::Req for procs.
template<bool Dynamic>
struct TModuleType { using Type = FModule; };
template<>
struct TModuleType<false> { using Type = FStaticModule; };

template<bool Dynamic>
using FModuleMaybe = typename TModuleType<Dynamic>::Type;

// ------------------------------------------------------------------------

template<FModule &Module, typename Proto>
class TOptProc
{
	FModule::StaticProc proc;
#ifndef NDEBUG
	mutable bool checked = false;
#endif

	// I am not a pointer
	bool operator==(void*) const;
	bool operator!=(void*) const;

public:
	TOptProc(const char* function)
	{
		proc.Name = function;
		proc.Optional = true;
		Module.RegisterStatic(proc);
	}

	operator Proto() const
	{
#ifndef NDEBUG
		assert(checked);
#endif
		return (Proto)proc.Call;
	}
	explicit operator bool() const
	{
#ifndef NDEBUG
		assert(Module.IsLoaded());
		checked = true;
#endif
		return proc.Call != nullptr;
	}
};

template<FModule &Module, typename Proto>
class TReqProc
{
	FModule::StaticProc proc;

	// I am not a pointer
	bool operator==(void*) const;
	bool operator!=(void*) const;

public:
	TReqProc(const char* function)
	{
		proc.Name = function;
		proc.Optional = false;
		Module.RegisterStatic(proc);
	}

	operator Proto() const
	{
#ifndef NDEBUG
		assert(Module.IsLoaded());
#endif
		return (Proto)proc.Call;
	}
	explicit operator bool() const { return true; }
};

template<FStaticModule &Module, typename Proto, Proto Sym>
class TStaticProc
{
	// I am not a pointer
	bool operator==(void*) const;
	bool operator!=(void*) const;

public:
	TStaticProc(const char* function) {}

	operator Proto() const { return Sym; }
	explicit operator bool() const { return Sym != nullptr; }
};

void FModule_SetProgDir(const char* progdir);
extern std::string module_progdir;
