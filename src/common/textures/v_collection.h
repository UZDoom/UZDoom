/*
** v_collection.h
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

#ifndef __V_COLLECTION_H__
#define __V_COLLECTION_H__

#include "tarray.h"
#include "textureid.h"

class FGameTexture;

class FImageCollection
{
public:
	FImageCollection();
	FImageCollection(const char **patchNames, int numPatches);

	void Init(const char **patchnames, int numPatches, ETextureType namespc = ETextureType::Any);
	void Add(const char **patchnames, int numPatches, ETextureType namespc = ETextureType::Any);
	void Uninit();

	FGameTexture *operator[] (int index) const;

protected:
	TArray<FTextureID> ImageMap;
};

#endif //__V_COLLECTION_H__
