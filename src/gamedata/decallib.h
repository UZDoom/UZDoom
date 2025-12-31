/*
** decallib.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __DECALLIB_H__
#define __DECALLIB_H__

#include <string.h>

#include "doomtype.h"
#include "renderstyle.h"
#include "palettecontainer.h"

class FScanner;
class FDecalTemplate;
struct FDecalAnimator;
class PClass;
class DBaseDecal;
struct side_t;

class FDecalBase
{
	friend class FDecalLib;
public:
	virtual const FDecalTemplate *GetDecal () const;
	virtual void ReplaceDecalRef (FDecalBase *from, FDecalBase *to) = 0;
	FName GetDecalName() const;
	
protected:
	FDecalBase ();
	virtual ~FDecalBase ();

	FDecalBase *Left, *Right;
	FName Name;
	uint16_t SpawnID;
	TArray<const PClass *> Users;	// Which actors generate this decal
};

class FDecalTemplate : public FDecalBase
{
	friend class FDecalLib;
public:
	FDecalTemplate () : Translation (NO_TRANSLATION) {}

	void ApplyToDecal (DBaseDecal *actor, side_t *wall) const;
	const FDecalTemplate *GetDecal () const;
	void ReplaceDecalRef (FDecalBase *from, FDecalBase *to);

	double ScaleX, ScaleY;
	uint32_t ShadeColor;
	FTranslationID Translation;
	FRenderStyle RenderStyle;
	FTextureID PicNum;
	uint16_t RenderFlags;
	bool translatable;
	double Alpha;				// same as actor->alpha
	const FDecalAnimator *Animator;
	const FDecalBase *LowerDecal;

	enum { DECAL_RandomFlipX = 0x100, DECAL_RandomFlipY = 0x200 };
};

class FDecalLib
{
public:
	FDecalLib ();
	~FDecalLib ();

	void Clear ();
	void ReadDecals (FScanner &sc);
	void ReadAllDecals ();

	FDecalBase* GetDecalBaseByName(const char* name) const;
	const FDecalTemplate *GetDecalByNum (uint16_t num) const;
	const FDecalTemplate *GetDecalByName (const char *name) const;

private:
	struct FTranslation;

	static void DelTree (FDecalBase *root);
	static FDecalBase *ScanTreeForNum (const uint16_t num, FDecalBase *root);
	static FDecalBase *ScanTreeForName (const char *name, FDecalBase *root);
	static void ReplaceDecalRef (FDecalBase *from, FDecalBase *to, FDecalBase *root);
	FTranslation *GenerateTranslation (uint32_t start, uint32_t end);
	void AddDecal (const char *name, uint16_t num, const FDecalTemplate &decal);
	void AddDecal (FDecalBase *decal);
	FDecalAnimator *FindAnimator (const char *name);

	uint16_t GetDecalID (FScanner &sc);
	void ParseDecal (FScanner &sc);
	void ParseDecalGroup (FScanner &sc);
	void ParseGenerator (FScanner &sc);
	void ParseFader (FScanner &sc);
	void ParseStretcher (FScanner &sc);
	void ParseSlider (FScanner &sc);
	void ParseCombiner (FScanner &sc);
	void ParseColorchanger (FScanner &sc);

	FDecalBase *Root;
	FTranslation *Translations;
};

extern FDecalLib DecalLibrary;

#endif //__DECALLIB_H__
