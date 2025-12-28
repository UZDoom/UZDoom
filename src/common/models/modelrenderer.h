/*
** modelrenderer.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Christoph Oelckers
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
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "renderstyle.h"
#include "matrix.h"
#include "model.h"

class FModelRenderer
{
public:
	virtual ~FModelRenderer() = default;

	virtual ModelRendererType GetType() const = 0;

	virtual void BeginDrawModel(FRenderStyle style, int smf_flags, const VSMatrix &objectToWorldMatrix, bool mirrored) = 0;
	virtual void EndDrawModel(FRenderStyle style, int smf_flags) = 0;

	virtual IModelVertexBuffer *CreateVertexBuffer(bool needindex, bool singleframe) = 0;

	virtual VSMatrix GetViewToWorldMatrix() = 0;

	virtual void BeginDrawHUDModel(FRenderStyle style, const VSMatrix &objectToWorldMatrix, bool mirrored, int smf_flags) = 0;
	virtual void EndDrawHUDModel(FRenderStyle style, int smf_flags) = 0;

	virtual void SetInterpolation(double interpolation) = 0;
	virtual void SetMaterial(FGameTexture *skin, bool clampNoFilter, FTranslationID translation) = 0;
	virtual void DrawArrays(int start, int count) = 0;
	virtual void DrawElements(int numIndices, size_t offset) = 0;
	virtual void SetupFrame(FModel* model, unsigned int frame1, unsigned int frame2, unsigned int size, int boneStartIndex) {};
};

