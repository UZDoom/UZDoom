/*
** p_blockmap.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Christoph Oelckers
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

#ifndef __P_BLOCKMAP_H
#define __P_BLOCKMAP_H

#include "doomtype.h"

class AActor;

// [RH] Like msecnode_t, but for the blockmap
struct FBlockNode
{
	AActor *Me;						// actor this node references
	int BlockIndex;					// index into blocklinks for the block this node is in
	int Group;						// portal group this link belongs to (can be different than the actor's own group
	FBlockNode **PrevActor;			// previous actor in this block
	FBlockNode *NextActor;			// next actor in this block
	FBlockNode **PrevBlock;			// previous block this actor is in
	FBlockNode *NextBlock;			// next block this actor is in

	static FBlockNode *Create (AActor *who, int x, int y, int group = -1);
	void Release ();

	static FBlockNode *FreeBlocks;
};

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size 128x128.
// Used to speed up collision detection
// by spatial subdivision in 2D.
//

struct FBlockmap
{
	int*				blockmaplump;	// offsets in blockmap are from here

	int*				blockmap;
	int					bmapwidth;
	int					bmapheight; 	// in mapblocks
	double				bmaporgx;
	double				bmaporgy;		// origin of block map
	FBlockNode**		blocklinks; 	// for thing chains

	// mapblocks are used to check movement
	// against lines and things
	static constexpr int MAPBLOCKUNITS = 128;

	inline int GetBlockX(double xpos)
	{
		return int((xpos - bmaporgx) / MAPBLOCKUNITS);
	}

	inline int GetBlockY(double ypos)
	{
		return int((ypos - bmaporgy) / MAPBLOCKUNITS);
	}

	inline bool isValidBlock(int x, int y) const
	{
		return ((unsigned int)x < (unsigned int)bmapwidth &&
			(unsigned int)y < (unsigned int)bmapheight);
	}

	inline int *GetLines(int x, int y) const
	{
		// There is an extra entry at the beginning of every block.
		// Apparently, id had originally intended for it to be used
		// to keep track of things, but the final code does not do that.
		int offset = y*bmapwidth + x;
		return blockmaplump + *(blockmap + offset) + 1;
	}

	bool VerifyBlockMap(int count, unsigned numlines);

	void Clear()
	{
		if (blockmaplump != nullptr)
		{
			delete[] blockmaplump;
			blockmaplump = nullptr;
		}
		if (blocklinks != nullptr)
		{
			delete[] blocklinks;
			blocklinks = nullptr;
		}
	}

	~FBlockmap()
	{
		Clear();
	}

};

#endif
