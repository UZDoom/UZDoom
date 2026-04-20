/*
** parsecontext.cpp
**
** Base class for Lemon-based parsers
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

#include <string.h>
#include <ctype.h>
#include "filesystem.h"
#include "parsecontext.h"
#include "p_lnspec.h"


//==========================================================================
//
//
//
//==========================================================================
void FParseContext::AddSym (char *sym, int val)
{
	FParseSymbol syme;
	syme.Value = val;
	strncpy (syme.Sym, sym, 79);
	syme.Sym[79]=0;
	symbols.Push(syme);
}

//==========================================================================
//
//
//
//==========================================================================
bool FParseContext::FindSym (char *sym, FParseSymbol **val)
{
	for(unsigned i=0;i<symbols.Size(); i++)
	{
		if (strcmp (symbols[i].Sym, sym) == 0)
		{
			*val = &symbols[i];
			return true;
		}
	}
	return false;
}


//==========================================================================
//
//
//
//==========================================================================
int FParseContext::GetToken (const char *&sourcep, FParseToken *yylval)
{
	char token[80];
	int toksize;
	int c;

loop:
	while (isspace (c = *sourcep++) && c != 0)
	{
		if (c == '\n')
			SourceLine++;
	}

	if (c == 0)
	{
		return 0;
	}
	if (isdigit (c))
	{
		int buildup = c - '0';
		if (c == '0')
		{
			c = *sourcep++;
			if (c == 'x' || c == 'X')
			{
				yylval->val = (int)strtoll(sourcep, (char**)&sourcep, 16);
				return TokenTrans[NUM];
			}
			else
			{
				sourcep--;
			}
		}
		char *endp;

		sourcep--;
		yylval->val = (int)strtoll(sourcep, &endp, 10);
		if (*endp == '.')
		{
			// It's a float
			yylval->fval = strtod(sourcep, (char**)& sourcep);
			return TokenTrans[FLOATVAL];
		}
		else
		{
			sourcep = endp;
			return TokenTrans[NUM];
		}
	}
	if (isalpha (c))
	{
		int buildup = 0;
		
		token[0] = c;
		toksize = 1;
		while (toksize < 79 && (isalnum (c = *sourcep++) || c == '_'))
		{
			token[toksize++] = c;
		}
		token[toksize] = 0;
		if (toksize == 79 && isalnum (c))
		{
			while (isalnum (c = *sourcep++))
				;
		}
		sourcep--;
		if (FindToken (token, &buildup))
		{
			return buildup;
		}
		if (FindSym (token, &yylval->symval))
		{
			yylval->val = yylval->symval->Value;
			return TokenTrans[NUM];
		}
		if ((yylval->val = P_FindLineSpecial(token)) != 0)
		{
			return TokenTrans[NUM];
		}
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
		// Work around GCC 4.8 bug 54570 causing release build crashes.
		asm("" : "+g" (yylval));
#endif
		strcpy (yylval->sym, token);
		return TokenTrans[SYM];
	}
	if (c == '/')
	{
		c = *sourcep++;
		if (c == '*')
		{
			for (;;)
			{
				while ((c = *sourcep++) != '*' && c != 0)
				{
					if (c == '\n')
						SourceLine++;
				}
				if (c == 0)
					return 0;
				if ((c = *sourcep++) == '/')
					goto loop;
				if (c == 0)
					return 0;
				sourcep--;
			}
		}
		else if (c == '/')
		{
			while ((c = *sourcep++) != '\n' && c != 0)
				;
			if (c == '\n')
				SourceLine++;
			else if (c == EOF)
				return 0;
			goto loop;
		}
		else
		{
			sourcep--;
			return TokenTrans[DIVIDE];
		}
	}
	if (c == '"')
	{
		int tokensize = 0;
		while ((c = *sourcep++) != '"' && c != 0)
		{
			yylval->string[tokensize++] = c;
		}
		yylval->string[tokensize] = 0;
		return TokenTrans[STRING];
	}
	if (c == '|')
	{
		c = *sourcep++;
		if (c == '=')
			return TokenTrans[OR_EQUAL];
		sourcep--;
		return TokenTrans[OR];
	}
	if (c == '<')
	{
		c = *sourcep++;
		if (c == '<')
		{
			c = *sourcep++;
			if (c == '=')
			{
				return TokenTrans[LSHASSIGN];
			}
			sourcep--;
			return 0;
		}
		c--;
		return 0;
	}
	if (c == '>')
	{
		c = *sourcep++;
		if (c == '>')
		{
			c = *sourcep++;
			if (c == '=')
			{
				return TokenTrans[RSHASSIGN];
			}
			sourcep--;
			return 0;
		}
		c--;
		return 0;
	}
	if (c == '#')
	{
		if (!strnicmp(sourcep, "include", 7))
		{
			sourcep+=7;
			return TokenTrans[INCLUDE];
		}
		if (!strnicmp(sourcep, "define", 6))
		{
			sourcep+=6;
			return TokenTrans[DEFINE];
		}
	}
	switch (c)
	{
	case '^': return TokenTrans[XOR];
	case '&': return TokenTrans[AND];
	case '-': return TokenTrans[MINUS];
	case '+': return TokenTrans[PLUS];
	case '*': return TokenTrans[MULTIPLY];
	case '%': return TokenTrans[MODULUS];
	case '(': return TokenTrans[LPAREN];
	case ')': return TokenTrans[RPAREN];
	case ',': return TokenTrans[COMMA];
	case '{': return TokenTrans[LBRACE];
	case '}': return TokenTrans[RBRACE];
	case '=': return TokenTrans[EQUALS];
	case ';': return TokenTrans[SEMICOLON];
	case ':': return TokenTrans[COLON];
	case '[': return TokenTrans[LBRACKET];
	case ']': return TokenTrans[RBRACKET];
	default:  return 0;
	}
}

//==========================================================================
//
//
//
//==========================================================================
int FParseContext::PrintError (const char *s)
{
	if (SourceFile != NULL)
		Printf ("%s, line %d: %s\n", SourceFile, SourceLine, s);
	else
		Printf ("%s\n", s);
	return 0;
}


//==========================================================================
//
//
//
//==========================================================================

void FParseContext::ParseLump(const char *lumpname)
{
	int tokentype;
	int SavedSourceLine = SourceLine;
	const char *SavedSourceFile = SourceFile;
	FParseToken token;

	int lumpno = fileSystem.CheckNumForFullName(lumpname, true);

	if (lumpno == -1) 
	{
		Printf ("%s, line %d: Lump '%s' not found\n", SourceFile, SourceLine, lumpname);
		return;
	}

	// Read the lump into a buffer and add a 0-terminator

	SourceLine = 0;
	SourceFile = lumpname;

	FString source = GetStringFromLump(lumpno);
	const char *sourcep = source.GetChars();
	while ( (tokentype = GetToken(sourcep, &token)) )
	{
		// It is much easier to handle include statements outside the main parser.
		if (tokentype == TokenTrans[INCLUDE])
		{
			if (GetToken(sourcep, &token) != TokenTrans[STRING])
			{
				Printf("%s, line %d: Include: String parameter expected\n", SourceFile, SourceLine);
				return;
			}
			ParseLump(token.string);
		}
		else
		{
			Parse(pParser, tokentype, token, this);
		}
	}
	SourceLine = SavedSourceLine;
	SourceFile = SavedSourceFile;
}

