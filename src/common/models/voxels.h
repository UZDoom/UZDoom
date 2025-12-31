/*
** voxels.h
**
** Voxels from Build
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#ifndef __RES_VOXEL_H
#define __RES_VOXEL_H

#include <stdint.h>

enum
{
	MAXVOXMIPS = 5,
};

struct kvxslab_t
{
	uint8_t		ztop;			// starting z coordinate of top of slab
	uint8_t		zleng;			// # of bytes in the color array - slab height
	uint8_t		backfacecull;	// low 6 bits tell which of 6 faces are exposed
	uint8_t		col[1/*zleng*/];// color data from top to bottom
};

struct kvxslab_bgra_t
{
	uint32_t	ztop;			// starting z coordinate of top of slab
	uint32_t	zleng;			// # of bytes in the color array - slab height
	uint32_t	backfacecull;	// low 6 bits tell which of 6 faces are exposed
	uint32_t	col[1/*zleng*/];// color data from top to bottom
};

struct FVoxel;

struct FVoxelMipLevel
{
	FVoxelMipLevel();
	~FVoxelMipLevel();

	int			SizeX;
	int			SizeY;
	int			SizeZ;
	DVector3	Pivot;
	int			*OffsetX;
	short		*OffsetXY;
private:
	uint8_t	*SlabData;
	TArray<uint8_t> SlabDataRemapped;
public:
	TArray<uint32_t> SlabDataBgra;

	uint8_t *GetSlabData(bool wantpaletted) const;

	friend FVoxel *R_LoadKVX(int lumpnum);
	friend struct FVoxel;
};

struct FVoxel
{
	TArray<uint8_t> Palette;
	int LumpNum;
	int NumMips;
	int VoxelIndex;
	FVoxelMipLevel Mips[MAXVOXMIPS];
	bool Remapped = false;
	bool Bgramade = false;

	void CreateBgraSlabData();
	void Remap();
	void RemovePalette();
};

struct FVoxelDef
{
	FVoxel *Voxel;
	int PlacedSpin;			// degrees/sec to spin actors without MF_DROPPED set
	int DroppedSpin;		// degrees/sec to spin actors with MF_DROPPED set
	int VoxeldefIndex;		// Needed by GZDoom
	double		Scale;
	DAngle		AngleOffset;// added to actor's angle to compensate for wrong-facing voxels
	double		xoffset;
	double		yoffset;
	double		zoffset;
	bool		PitchFromMomentum;
	bool		UseActorPitch;
	bool		UseActorRoll;
};

extern TDeletingArray<FVoxel *> Voxels;	// used only to auto-delete voxels on exit.
extern TDeletingArray<FVoxelDef *> VoxelDefs;

FVoxel* VOX_GetVoxel(int lumpnum);

FVoxel *R_LoadKVX(int lumpnum);
FVoxelDef *R_LoadVoxelDef(int lumpnum, int spin);

#endif
