/*
** parse_xlat.cpp
**
** Translation definition compiler
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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


#include "doomtype.h"
#include "parsecontext.h"
#include "xlat_parser.h"
#include "xlat.h"


// Token types not used in the grammar
enum
{
	XLAT_INCLUDE=128,
	XLAT_STRING,
	XLAT_FLOATVAL,	// floats are not used by the grammar
};

DEFINE_TOKEN_TRANS(XLAT_)


TMap<FName, FTranslator > translators;


struct SpecialArgs
{
	int addflags;
	int argcount;
	int args[5];
};

struct SpecialArg
{
	int arg;
	ELineTransArgOp argop;
};

struct ListFilter
{
	uint16_t filter;
	uint8_t value;
};

struct MoreFilters
{
	MoreFilters *next;
	ListFilter filter;
};

struct MoreLines
{
	MoreLines *next;
	FBoomArg arg;
};

struct ParseBoomArg
{
	uint8_t constant;
	uint16_t mask;
	MoreFilters *filters;
};


struct XlatParseContext : public FParseContext
{
	XlatParseContext(void *parser, ParseFunc parse, int *tt, FTranslator *trans)
		: FParseContext(parser, parse, tt)
	{
		Translator = trans;
		DefiningLineType = -1;
	}

	//==========================================================================
	//
	//
	//
	//==========================================================================
	bool FindToken (char *tok, int *type)
	{
		static const char *tokens[] =
		{
			"arg2", "arg3", "arg4", "arg5", "bitmask", "clear",
			"define", "enum", "flags", "include", "lineflag", "lineid", 
			"maxlinespecial", "nobitmask", "sector", "tag"
		};
		static const short types[] =
		{
			XLAT_ARG2, XLAT_ARG3, XLAT_ARG4, XLAT_ARG5, XLAT_BITMASK, XLAT_CLEAR,
			XLAT_DEFINE, XLAT_ENUM, XLAT_FLAGS, XLAT_INCLUDE, XLAT_LINEFLAG, XLAT_TAG,
			XLAT_MAXLINESPECIAL, XLAT_NOBITMASK, XLAT_SECTOR, XLAT_TAG
		};

		int min = 0, max = countof(tokens) - 1;

		while (min <= max)
		{
			int mid = (min + max) / 2;
			int lexval = stricmp (tok, tokens[mid]);
			if (lexval == 0)
			{
				*type = types[mid];
				return true;
			}
			else if (lexval > 0)
			{
				min = mid + 1;
			}
			else
			{
				max = mid - 1;
			}
		}
		return false;
	}

	int DefiningLineType;
	FTranslator *Translator;
};

#include "xlat_parser.c"


//==========================================================================
//
//
//
//==========================================================================

FTranslator *P_LoadTranslator(const char *lumpname)
{
	FName fname = lumpname;
	auto translator = &translators[fname];
	if (!translator->loaded)
	{
		void *pParser = XlatParseAlloc(malloc);

		XlatParseContext context(pParser, XlatParse, TokenTrans, translator);
		context.ParseLump(lumpname);
		FParseToken tok;
		tok.val=0;
		XlatParse(pParser, 0, tok, &context);
		XlatParseFree(pParser, free);
		translator->loaded = true;
	}
	return translator;
}


