/*
** critsec.cpp
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

#ifdef _WIN32

#ifndef _WINNT_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace FileSys {

class FInternalCriticalSection
{
public:
	void Enter()
	{
		AcquireSRWLockExclusive(&CritSec);
	}
	void Leave()
	{
		ReleaseSRWLockExclusive(&CritSec);
	}
private:
	SRWLOCK CritSec = SRWLOCK_INIT;
};


FInternalCriticalSection *CreateCriticalSection()
{
	return new FInternalCriticalSection();
}

void DeleteCriticalSection(FInternalCriticalSection *c)
{
	delete c;
}

void EnterCriticalSection(FInternalCriticalSection *c)
{
	c->Enter();
}

void LeaveCriticalSection(FInternalCriticalSection *c)
{
	c->Leave();
}

#else

#include "critsec.h"

#include <pthread.h>

namespace FileSys {

class FInternalCriticalSection
{
public:
	FInternalCriticalSection();
	~FInternalCriticalSection();

	void Enter();
	void Leave();

private:
	pthread_mutex_t m_mutex;

};

// TODO: add error handling

FInternalCriticalSection::FInternalCriticalSection()
{
	pthread_mutexattr_t attributes;
	pthread_mutexattr_init(&attributes);
	pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_mutex, &attributes);

	pthread_mutexattr_destroy(&attributes);
}

FInternalCriticalSection::~FInternalCriticalSection()
{
	pthread_mutex_destroy(&m_mutex);
}

void FInternalCriticalSection::Enter()
{
	pthread_mutex_lock(&m_mutex);
}

void FInternalCriticalSection::Leave()
{
	pthread_mutex_unlock(&m_mutex);
}


FInternalCriticalSection *CreateCriticalSection()
{
	return new FInternalCriticalSection();
}

void DeleteCriticalSection(FInternalCriticalSection *c)
{
	delete c;
}

void EnterCriticalSection(FInternalCriticalSection *c)
{
	c->Enter();
}

void LeaveCriticalSection(FInternalCriticalSection *c)
{
	c->Leave();
} 

#endif

}
