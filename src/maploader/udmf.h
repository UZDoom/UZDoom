/*
** udmf.h
**
** UDMF text map parser
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Christoph Oelckers
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

#ifndef __P_UDMF_H
#define __P_UDMF_H

#include "sc_man.h"
#include "m_fixed.h"

class UDMFParserBase
{
protected:
	FScanner sc;
	FName namespc = NAME_None;
	int namespace_bits;
	FString parsedString;
	bool BadCoordinates = false;

	void Skip();
	FName ParseKey(bool checkblock = false, bool *isblock = NULL);
	int CheckInt(FName key);
	double CheckFloat(FName key);
	double CheckCoordinate(FName key);
	DAngle CheckAngle(FName key);
	bool CheckBool(FName key);
	const char *CheckString(FName key);
	int MatchString(FName key, const char* const* strings, int defval);

	template<typename T>
	bool Flag(T &value, int mask, FName key)
	{
		if (CheckBool(key))
		{
			value |= mask;
			return true;
		}
		else
		{
			value &= ~mask;
			return false;
		}
	}

};

#define BLOCK_ID (ENamedName)-1

#endif
