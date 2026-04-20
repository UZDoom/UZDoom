/*
** parsecontext.h
**
** Base class for Lemon-based parsers
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

#ifndef __PARSECONTEST__H_
#define __PARSECONTEST__H_

#include "tarray.h"

// The basic tokens the parser requires the grammar to understand
enum
{
	 OR       =1,
	 XOR      ,
	 AND      ,
	 MINUS    ,
	 PLUS     ,
	 MULTIPLY ,
	 DIVIDE   ,
	 MODULUS  ,
	 NUM      ,
	 FLOATVAL ,
	 LPAREN   ,
	 RPAREN   ,
	 SYM      ,
	 RBRACE   ,
	 LBRACE   ,
	 COMMA    ,
	 EQUALS   ,
	 LBRACKET ,
	 RBRACKET ,
	 OR_EQUAL ,
	 COLON    ,
	 SEMICOLON,
	 LSHASSIGN,
	 RSHASSIGN,
	 STRING   ,
	 INCLUDE  ,
	 DEFINE   ,
};

#define DEFINE_TOKEN_TRANS(prefix) \
	static int TokenTrans[] = { \
	0, \
	prefix##OR, \
	prefix##XOR, \
	prefix##AND, \
	prefix##MINUS, \
	prefix##PLUS, \
	prefix##MULTIPLY, \
	prefix##DIVIDE, \
	prefix##MODULUS, \
	prefix##NUM, \
	prefix##FLOATVAL, \
	prefix##LPAREN, \
	prefix##RPAREN, \
	prefix##SYM, \
	prefix##RBRACE, \
	prefix##LBRACE, \
	prefix##COMMA, \
	prefix##EQUALS, \
	prefix##LBRACKET, \
	prefix##RBRACKET, \
	prefix##OR_EQUAL, \
	prefix##COLON, \
	prefix##SEMICOLON, \
	prefix##LSHASSIGN, \
	prefix##RSHASSIGN, \
	prefix##STRING, \
	prefix##INCLUDE, \
	prefix##DEFINE, \
	 };


struct FParseSymbol
{
	int Value;
	char Sym[80];
};

union FParseToken
{
	int val;
	double fval;
	char sym[80];
	char string[80];
	FParseSymbol *symval;
};


struct FParseContext;

typedef void (*ParseFunc)(void *pParser, int tokentype, FParseToken token, FParseContext *context);

struct FParseContext
{
	TArray<FParseSymbol> symbols;
	int SourceLine;
	const char *SourceFile;
	int EnumVal;
	int *TokenTrans;
	void *pParser;
	ParseFunc Parse;

	FParseContext(void *parser, ParseFunc parse, int *tt)
	{
		SourceLine = 0;
		SourceFile = NULL;
		pParser = parser;
		Parse = parse;
		TokenTrans = tt;
	}

	virtual ~FParseContext() {}

	void AddSym (char *sym, int val);
	bool FindSym (char *sym, FParseSymbol **val);
	virtual bool FindToken (char *tok, int *type) = 0;
	int GetToken (const char *&sourcep, FParseToken *yylval);
	int PrintError (const char *s);
	void ParseLump(const char *lumpname);
};


#endif
