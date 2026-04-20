/*
** doom_aabbtree.h
**
** AABB-tree used for ray testing
**
**---------------------------------------------------------------------------
**
** Copyright 2017 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "hw_aabbtree.h"

struct FLevelLocals;

// Axis aligned bounding box tree used for ray testing treelines.
class DoomLevelAABBTree : public hwrenderer::LevelAABBTree
{
public:
	// Constructs a tree for the current level
	DoomLevelAABBTree(FLevelLocals *lev);
	bool Update() override;

private:
	bool GenerateTree(const FVector2 *centroids, bool dynamicsubtree);

	// Generate a tree node and its children recursively
	int GenerateTreeNode(int *treelines, int num_lines, const FVector2 *centroids, int *work_buffer);

	TArray<int> mapLines;
	FLevelLocals *Level;
};

