/*
** ObjectStateNode.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025 UZDoom Maintainers and Contributors
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
#include <common/scripting/dap/GameInterfaces.h>
#include <dap/protocol.h>

#include "StateNodeBase.h"
#include <map>

namespace DebugServer
{
class ObjectStateNode : public StateNodeNamedVariable, public IStructuredState
{
	bool m_subView;

	const VMValue m_value;
	PType *m_ClassType;
	std::vector<std::string> m_cachedNames; // to ensure proper order of children
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_virtualChildren;
	PType *m_VMType = nullptr;
	public:
	ObjectStateNode(const std::string &name, VMValue value, PType *asClass, bool subView = false);

	bool SerializeToProtocol(dap::Variable &variable) override;

	bool GetChildNames(std::vector<std::string> &names) override;
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
	void Reset();
	bool IsVirtualStructure() override { return m_subView; }
	caseless_path_map<std::shared_ptr<StateNodeBase>> GetVirtualContainerChildren() override;
};
}
