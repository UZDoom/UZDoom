/*
** ancientzip.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2010-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
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
#include "fs_files.h"

namespace FileSys {
	
class FZipExploder
{
	unsigned int Hold, Bits;
	FileReader *In;
	unsigned int InLeft;

	/****************************************************************
	   Shannon-Fano tree structures, variables and related routines
	 ****************************************************************/

	struct HuffNode
	{
		unsigned char Value;
		unsigned char Length;
		unsigned short ChildTable;
	};

	struct TableBuilder
	{
		unsigned char Value;
		unsigned char Length;
		unsigned short Code;
	};

	std::vector<HuffNode> LiteralDecoder;
	std::vector<HuffNode> DistanceDecoder;
	std::vector<HuffNode> LengthDecoder;
	unsigned char ReadBuf[256];
	unsigned int bs, be;

	static int buildercmp(const void *a, const void *b);
	void InsertCode(std::vector<HuffNode> &decoder, unsigned int pos, int bits, unsigned short code, int len, unsigned char value);
	unsigned int InitTable(std::vector<HuffNode> &decoder, int numspots);
	int BuildDecoder(std::vector<HuffNode> &decoder, TableBuilder *values, int numvals);
	int DecodeSFValue(const std::vector<HuffNode> &currentTree);
	int DecodeSF(std::vector<HuffNode> &decoder, int numvals);
public:
	int Explode(unsigned char *out, unsigned int outsize, FileReader &in, unsigned int insize, int flags);
};

int ShrinkLoop(unsigned char *out, unsigned int outsize, FileReader &in, unsigned int insize);
}
