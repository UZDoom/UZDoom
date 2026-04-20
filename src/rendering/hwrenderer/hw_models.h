/*
** hw_models.h
**
** hardware renderer model handling code
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "tarray.h"
#include "p_pspr.h"
#include "voxels.h"
#include "models.h"
#include "hwrenderer/data/buffers.h"
#include "hw_modelvertexbuffer.h"
#include "modelrenderer.h"

class HWSprite;
struct HWDrawInfo;
class FRenderState;


class FHWModelRenderer : public FModelRenderer
{
	friend class FModelVertexBuffer;
	int modellightindex = -1;
	int boneIndexBase = -1;
	HWDrawInfo *di;
	FRenderState &state;
public:
	FHWModelRenderer(HWDrawInfo *d, FRenderState &st, int mli) : modellightindex(mli), di(d), state(st)
	{}
	ModelRendererType GetType() const override { return GLModelRendererType; }
	void BeginDrawModel(FRenderStyle style, int smf_flags, const VSMatrix &objectToWorldMatrix, bool mirrored) override;
	void EndDrawModel(FRenderStyle style, int smf_flags) override;
	IModelVertexBuffer *CreateVertexBuffer(bool needindex, bool singleframe) override;
	VSMatrix GetViewToWorldMatrix() override;
	void BeginDrawHUDModel(FRenderStyle style, const VSMatrix &objectToWorldMatrix, bool mirrored, int smf_flags) override;
	void EndDrawHUDModel(FRenderStyle style, int smf_flags) override;
	void SetInterpolation(double interpolation) override;
	void SetMaterial(FGameTexture *skin, bool clampNoFilter, FTranslationID translation) override;
	void DrawArrays(int start, int count) override;
	void DrawElements(int numIndices, size_t offset) override;
	void SetupFrame(FModel *model, unsigned int frame1, unsigned int frame2, unsigned int size, int boneStartIndex) override;

};

