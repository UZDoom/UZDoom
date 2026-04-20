/*
** skyboxtexture.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2019 Christoph Oelckers
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

#include "textures.h"

//-----------------------------------------------------------------------------
//
// Todo: Get rid of this
// The faces can easily be stored in the material layer array
//
//-----------------------------------------------------------------------------

class FSkyBox : public FImageTexture
{
public:

	FGameTexture* previous;
	FGameTexture* faces[6];	// the faces need to be full materials as they can have all supported effects.
	bool fliptop;

	FSkyBox(const char* name);
	void SetSize();

	bool Is3Face() const
	{
		return faces[5] == nullptr;
	}

	bool IsFlipped() const
	{
		return fliptop;
	}

	FGameTexture* GetSkyFace(int num) const 
	{
		return faces[num];
	}

	bool GetSkyFlip() const
	{
		return fliptop;
	}
};
