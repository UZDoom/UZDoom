/*
** r_canvastexture.h
**
** Maintenance data for camera textures
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
** Copyright 2006-2018 Christoph Oelckers
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

class FCanvas;
class FCanvasTexture;

// This list keeps track of the cameras that draw into canvas textures.
struct FCanvasTextureEntry
{
	TObjPtr<AActor*> Viewpoint;
	FCanvasTexture *Texture;
	FTextureID PicNum;
	double FOV;
};


struct FCanvasTextureInfo
{
	TArray<FCanvasTextureEntry> List;
	
	void Add (AActor *viewpoint, FTextureID picnum, double fov);
	void UpdateAll (std::function<void(AActor *, FCanvasTexture *, double fov)> callback);
	void EmptyList ();
	void Serialize(FSerializer &arc);
	void Mark();
	
};
