/*
** multipatchtexture.h
**
** Texture class for standard Doom multipatch textures
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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

#pragma once
#include "sc_man.h"
#include "palettecontainer.h"
#include "textureid.h"
#include "vectors.h"
#include "bitmap.h"
#include "image.h"
#include "textures.h"

class FImageTexture;
class FTextureManager;

//==========================================================================
//
// TexPart is the data that will get passed to the final texture.
//
//==========================================================================

struct TexPart
{
	FRemapTable *Translation = nullptr;
	FImageSource *Image = nullptr;
	PalEntry Blend = 0;
	blend_t Alpha = FRACUNIT;
	int16_t OriginX = 0;
	int16_t OriginY = 0;
	uint8_t Rotate = 0;
	uint8_t op = OP_COPY;
};

struct TexPartBuild
{
	FRemapTable* Translation = nullptr;
	FImageTexture *TexImage = nullptr;
	PalEntry Blend = 0;
	blend_t Alpha = FRACUNIT;
	int16_t OriginX = 0;
	int16_t OriginY = 0;
	uint8_t Rotate = 0;
	uint8_t op = OP_COPY;
};



//==========================================================================
//
// A texture defined in a TEXTURE1 or TEXTURE2 lump
//
//==========================================================================

class FMultiPatchTexture : public FImageSource
{
	friend class FTexture;
	friend class FGameTexture;
public:
	FMultiPatchTexture(int w, int h, const TArray<TexPartBuild> &parts, bool complex, bool textual);
	int GetNumParts() const { return NumParts; }
	// Query some needed info for texture hack support.
	bool SupportRemap0() override;
	bool IsRawCompatible() override 
	{
		return NumParts != 1 || Parts[0].OriginY == 0 || bTextual;
	}
	FImageSource* GetImageForPart(int num)
	{
		if (num >= 0 && num < NumParts) return Parts[num].Image;
		return nullptr;
	}

protected:
	int NumParts;
	bool bComplex;
	bool bTextual;
	TexPart *Parts;

	// The getters must optionally redirect if it's a simple one-patch texture.
	int CopyPixels(FBitmap *bmp, int conversion, int frame = 0) override;
	PalettedPixels CreatePalettedPixels(int conversion, int frame = 0) override;
	void CopyToBlock(uint8_t *dest, int dwidth, int dheight, FImageSource *source, int xpos, int ypos, int rotate, const uint8_t *translation, int style);
	void CollectForPrecache(PrecacheInfo &info, bool requiretruecolor) override;

};


//==========================================================================
//
// Additional data per patch which is needed for initialization
//
//==========================================================================

struct TexInit
{
	FString TexName;
	ETextureType UseType = ETextureType::Null;
	FGameTexture *GameTexture = nullptr;
	bool Silent = false;
	bool HasLine = false;
	bool UseOffsets = false;
	FScriptPosition sc;
};

//==========================================================================
//
// All build info only needed to construct the multipatch textures
//
//==========================================================================

struct FPatchLookup;

struct BuildInfo
{
	FString Name;
	TArray<TexPartBuild> Parts;
	TArray<TexInit> Inits;
	int Width = 0;
	int Height = 0;
	DVector2 Scale = { 1, 1 };
	bool bWorldPanning = false;	// This sucks!
	int DefinitionLump = 0;
	bool bComplex = false;
	bool textual = false;
	bool bNoDecals = false;
	bool bNoTrim = false;
	int LeftOffset[2] = {};
	int TopOffset[2] = {};
	FGameTexture *texture = nullptr;

	void swap(BuildInfo &other)
	{
		Name.Swap(other.Name);
		Parts.Swap(other.Parts);
		Inits.Swap(other.Inits);
		std::swap(Width, other.Width);
		std::swap(Height, other.Height);
		std::swap(Scale, other.Scale);
		std::swap(bWorldPanning, other.bWorldPanning);
		std::swap(DefinitionLump, other.DefinitionLump);
		std::swap(bComplex, other.bComplex);
		std::swap(textual, other.textual);
		std::swap(bNoDecals, other.bNoDecals);
		std::swap(LeftOffset[0], other.LeftOffset[0]);
		std::swap(LeftOffset[1], other.LeftOffset[1]);
		std::swap(TopOffset[0], other.TopOffset[0]);
		std::swap(TopOffset[1], other.TopOffset[1]);
		std::swap(texture, other.texture);
	}
};



class FMultipatchTextureBuilder
{
	FTextureManager &TexMan;
	TArray<BuildInfo> BuiltTextures;
	TMap<FGameTexture*, bool> complex;
	void(*progressFunc)();
	void(*checkForHacks)(BuildInfo&);

	void MakeTexture(BuildInfo &buildinfo, ETextureType usetype);
	void AddImageToTexture(FImageTexture* tex, BuildInfo& buildinfo);

	void BuildTexture(const void *texdef, FPatchLookup *patchlookup, int maxpatchnum, bool strife, int deflumpnum, ETextureType usetyoe);
	void AddTexturesLump(const void *lumpdata, int lumpsize, int deflumpnum, int patcheslump, int firstdup, bool texture1);

	void ParsePatch(FScanner &sc, BuildInfo &info, TexPartBuild &part, TexInit &init);
	void ResolvePatches(BuildInfo &buildinfo);

public:
	FMultipatchTextureBuilder(FTextureManager &texMan, void(*progressFunc_)(), void(*checkForHacks_)(BuildInfo &)) : TexMan(texMan), progressFunc(progressFunc_), checkForHacks(checkForHacks_)
	{
	}

	void AddTexturesLumps(int lump1, int lump2, int patcheslump);
	void ParseTexture(FScanner &sc, ETextureType usetype, int deflump);
	void ResolveAllPatches();
};
