/*
** engineerrors.h
**
** Contains error classes that can be thrown around
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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
** Code written prior to 2026 is also licensed under:
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

#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <string.h>
#include <stdio.h>
#include <exception>
#include <stdexcept>
#include "basics.h"

#define MAX_ERRORTEXT	1024

class CEngineError : public std::exception
{
public:
	CEngineError ()
	{
		m_Message[0] = '\0';
	}
	CEngineError (const char *message)
	{
		SetMessage (message);
	}
	void SetMessage (const char *message)
	{
		strncpy (m_Message, message, MAX_ERRORTEXT-1);
		m_Message[MAX_ERRORTEXT-1] = '\0';
	}
	void AppendMessage(const char *message) noexcept
	{
		size_t len = strlen(m_Message);
		strncpy(m_Message + len, message, MAX_ERRORTEXT - 1 - len);
		m_Message[MAX_ERRORTEXT - 1] = '\0';
	}
	const char *GetMessage (void) const noexcept
	{
		if (m_Message[0] != '\0')
			return (const char *)m_Message;
		else
			return NULL;
	}
	char const *what() const noexcept override
	{
		return m_Message;
	}


protected:
	char m_Message[MAX_ERRORTEXT];
};


class CRecoverableError : public CEngineError
{
public:
	CRecoverableError() : CEngineError() {}
	CRecoverableError(const char *message) : CEngineError(message) {}
};

class CFatalError : public CEngineError
{
public:
	CFatalError() : CEngineError() {}
	CFatalError(const char *message) : CEngineError(message) {}
};

class CExitEvent : public std::exception
{
	int m_reason;
public:
	CExitEvent(int reason) { m_reason = reason; }
	char const *what() const noexcept override
	{
		return "The game wants to exit";
	}
	int Reason() const { return m_reason; }
};

void I_ShowFatalError(const char *message);
[[noreturn]] void I_Error (const char *error, ...) GCCPRINTF(1,2);
[[noreturn]] void I_FatalError (const char *error, ...) GCCPRINTF(1,2);

#endif //__ERRORS_H__
