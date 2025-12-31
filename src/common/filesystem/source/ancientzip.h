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
