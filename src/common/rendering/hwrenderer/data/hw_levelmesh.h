/*
** hw_levelmesh.h
**
**
**
**---------------------------------------------------------------------------
**
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
#include "vectors.h"

namespace hwrenderer
{

class LevelMesh
{
public:
	virtual ~LevelMesh() = default;

	TArray<FVector3> MeshVertices;
	TArray<int> MeshUVIndex;
	TArray<uint32_t> MeshElements;
	TArray<int> MeshSurfaces;
};

} // namespace
