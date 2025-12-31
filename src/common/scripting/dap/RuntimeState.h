/*
** RuntimeState.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
** Code written prior to 2026 is also licensed under:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "IdMap.h"

#include <memory>
#include <vector>
#include "Nodes/StateNodeBase.h"
#include "vm.h"

namespace DebugServer
{
class RuntimeState
{
	std::unique_ptr<IdMap<std::string>> m_paths;
	std::shared_ptr<StateNodeBase> m_root;
	public:
	static VMFrameStack *m_GlobalVMStack;
	explicit RuntimeState(const std::shared_ptr<IdProvider> &idProvider);
	void Reset();
	bool ResolveStateByPath(std::string requestedPath, std::shared_ptr<StateNodeBase> &node, bool isEvaluate = false);
	bool ResolveStateById(uint32_t id, std::shared_ptr<StateNodeBase> &node, bool isEvaluate = false);
	bool ResolveChildrenByParentPath(std::string requestedPath, std::vector<std::shared_ptr<StateNodeBase>> &nodes, size_t start = 0, size_t count = INT_MAX);
	bool ResolveChildrenByParentId(uint32_t id, std::vector<std::shared_ptr<StateNodeBase>> &nodes, size_t start = 0, size_t count = INT_MAX);
	std::string GetPathById(const uint32_t id) const;

	static std::shared_ptr<StateNodeBase>
	CreateNodeForVariable(std::string name, VMValue variable, PType *p_type, const VMFrame *current_frame = nullptr, PClass *stateOwningClass = nullptr);

	static VMFrameStack *GetStack(uint32_t stackId);
	static VMFrame *GetFrame(uint32_t stackId, uint32_t level);

	static void GetStackFrames(VMFrameStack *stack, std::vector<VMFrame *> &frames);
	static bool GetStackFrames(uint32_t stackId, std::vector<VMFrame *> &frames);
};
}
