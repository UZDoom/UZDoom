/*
** v_collection.cpp
**
** Holds a collection of images
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

#include "v_collection.h"
#include "v_font.h"
#include "v_video.h"
#include "filesystem.h"
#include "texturemanager.h"

FImageCollection::FImageCollection ()
{
}

FImageCollection::FImageCollection (const char **patchNames, int numPatches)
{
	Add (patchNames, numPatches);
}

void FImageCollection::Init (const char **patchNames, int numPatches, ETextureType namespc)
{
	ImageMap.Clear();
	Add(patchNames, numPatches, namespc);
}

// [MH] Mainly for mugshots with skins and SBARINFO
void FImageCollection::Add (const char **patchNames, int numPatches, ETextureType namespc)
{
	int OldCount = ImageMap.Size();

	ImageMap.Resize(OldCount + numPatches);

	for (int i = 0; i < numPatches; ++i)
	{
		FTextureID texid = TexMan.CheckForTexture(patchNames[i], namespc);
		ImageMap[OldCount + i] = texid;
	}
}

void FImageCollection::Uninit ()
{
	ImageMap.Clear();
}

FGameTexture *FImageCollection::operator[] (int index) const
{
	if ((unsigned int)index >= ImageMap.Size())
	{
		return NULL;
	}
	return ImageMap[index].Exists()? TexMan.GetGameTexture(ImageMap[index], true) : NULL;
}
