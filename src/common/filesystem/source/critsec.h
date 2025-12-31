/*
** critsec.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
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

namespace FileSys {
// System independent critical sections without polluting the namespace with the operating system headers.
class FInternalCriticalSection;
FInternalCriticalSection *CreateCriticalSection();
void DeleteCriticalSection(FInternalCriticalSection *c);
void EnterCriticalSection(FInternalCriticalSection *c);
void LeaveCriticalSection(FInternalCriticalSection *c);

// This is just a convenience wrapper around the function interface adjusted to use std::lock_guard
class FCriticalSection
{
public:
	FCriticalSection()
	{
		c = CreateCriticalSection();
	}
	
	~FCriticalSection()
	{
		DeleteCriticalSection(c);
	}

	void lock()
	{
		EnterCriticalSection(c);
	}
	
	void unlock()
	{
		LeaveCriticalSection(c);
	}

private:
	FInternalCriticalSection *c;

};
}
