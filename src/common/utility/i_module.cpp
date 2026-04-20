/*
** i_module.cpp
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

#include "i_module.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif


#ifndef _WIN32
#define LoadLibraryA(x) dlopen((x), RTLD_LAZY)
#define GetProcAddress(a,b) dlsym((a),(b))
#define FreeLibrary(x) dlclose((x))
using HMODULE = void*;
#endif

bool FModule::Load(std::initializer_list<const char*> libnames)
{
	for(auto lib : libnames)
	{
		if(!Open(lib))
			continue;

		StaticProc *proc;
		for(proc = reqSymbols;proc;proc = proc->Next)
		{
			if(!(proc->Call = GetSym(proc->Name)) && !proc->Optional)
			{
				Unload();
				break;
			}
		}

		if(IsLoaded())
			return true;
	}

	return false;
}

void FModule::Unload()
{
	if(handle)
	{
		FreeLibrary((HMODULE)handle);
		handle = nullptr;
	}
}

bool FModule::Open(const char* lib)
{
#ifdef _WIN32
	if((handle = GetModuleHandleA(lib)) != nullptr)
		return true;
#else
	// Loading an empty string in Linux doesn't do what we expect it to.
	if(*lib == '\0')
		return false;
#endif
	handle = LoadLibraryA(lib);
	return handle != nullptr;
}

void *FModule::GetSym(const char* name)
{
	return (void *)GetProcAddress((HMODULE)handle, name);
}

std::string module_progdir(".");	// current program directory used to look up dynamic libraries. Default to something harmless in case the user didn't set it.

void FModule_SetProgDir(const char* progdir)
{
	module_progdir = progdir;
}
