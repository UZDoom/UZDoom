/*
** doom_aabbtree.h
**
** AABB-tree used for ray testing
**
**---------------------------------------------------------------------------
**
** Copyright 2017 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

