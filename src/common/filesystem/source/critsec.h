/*
** critsec.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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
