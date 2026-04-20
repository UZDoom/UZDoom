/*
** skyboxtexture.cpp
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

#include "filesystem.h"
#include "textures.h"
#include "skyboxtexture.h"
#include "bitmap.h"
#include "texturemanager.h"



//-----------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------

FSkyBox::FSkyBox(const char *name)
	: FImageTexture(nullptr)
{
	FTextureID texid = TexMan.CheckForTexture(name, ETextureType::Wall);
	if (texid.isValid())
	{
		previous = TexMan.GetGameTexture(texid);
	}
	else previous = nullptr;
	faces[0]=faces[1]=faces[2]=faces[3]=faces[4]=faces[5] = nullptr;
	fliptop = false;
}

//-----------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------

void FSkyBox::SetSize()
{
	if (!previous && faces[0]) previous = faces[0];
	if (previous && previous->GetTexture()->GetImage())
	{
		SetImage(previous->GetTexture()->GetImage());
	}
}
