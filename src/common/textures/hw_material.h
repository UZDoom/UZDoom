/*
** hw_material.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __GL_MATERIAL_H
#define __GL_MATERIAL_H

#include "m_fixed.h"
#include "textures.h"

struct FRemapTable;
class IHardwareTexture;

struct MaterialLayerInfo
{
	FTexture* layerTexture;
	int scaleFlags;
	int clampflags;
};

//===========================================================================
// 
// this is the material class for OpenGL. 
//
//===========================================================================

class FMaterial
{
	private:
	TArray<MaterialLayerInfo> mTextureLayers; // the only layers allowed to scale are the brightmap and the glowmap.
	int mShaderIndex;
	int mLayerFlags = 0;
	int mScaleFlags;

public:
	static void SetLayerCallback(IHardwareTexture* (*layercallback)(int layer, int translation));

	FGameTexture *sourcetex;	// the owning texture. 

	FMaterial(FGameTexture *tex, int scaleflags);
	virtual ~FMaterial();
	int GetLayerFlags() const { return mLayerFlags; }
	int GetShaderIndex() const { return mShaderIndex; }
	int GetScaleFlags() const { return mScaleFlags; }
	virtual void DeleteDescriptors() { }
	FVector2 GetDetailScale() const
	{
		return sourcetex->GetDetailScale();
	}

	FGameTexture* Source() const
	{
		return sourcetex;
	}

	void ClearLayers()
	{
		mTextureLayers.Resize(1);
	}

	void AddTextureLayer(FTexture *tex, bool allowscale)
	{
		mTextureLayers.Push({ tex, allowscale });
	}

	int NumLayers() const
	{
		return mTextureLayers.Size();
	}

	IHardwareTexture *GetLayer(int i, int translation, MaterialLayerInfo **pLayer = nullptr) const;


	static FMaterial *ValidateTexture(FGameTexture * tex, int scaleflags, bool create = true);
	const TArray<MaterialLayerInfo> &GetLayerArray() const
	{
		return mTextureLayers;
	}
};

#endif


