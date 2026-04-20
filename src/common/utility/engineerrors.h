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
