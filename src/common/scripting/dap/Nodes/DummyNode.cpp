/*
** DummyNode.cpp
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

#include "DummyNode.h"

#include <utility>
namespace DebugServer
{
DummyNode::DummyNode(std::string name, std::string value, std::string type) : StateNodeNamedVariable(std::move(name)), m_value(std::move(value)), m_type(std::move(type)) { }

bool DummyNode::SerializeToProtocol(dap::Variable &variable)
{
	SetVariableName(variable);
	variable.value = m_value;
	variable.type = m_type;
	return true;
}

DummyWithChildrenNode::DummyWithChildrenNode(std::string name, std::string value, std::string type, caseless_path_map<std::shared_ptr<StateNodeBase>> children)
		: StateNodeNamedVariable(std::move(name)), m_value(std::move(value)), m_type(std::move(type)), m_children(std::move(children))
{
}

bool DummyWithChildrenNode::GetChildNames(std::vector<std::string> &names)
{
	for (const auto &child : m_children)
	{
		names.push_back(child.first);
	}
	return true;
}

bool DummyWithChildrenNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (m_children.find(name) != m_children.end())
	{
		node = m_children[name];
		return true;
	}
	return false;
}

bool DummyWithChildrenNode::SerializeToProtocol(dap::Variable &variable)
{
	SetVariableName(variable);
	variable.value = m_value;
	variable.type = m_type;
	variable.variablesReference = GetId();
	variable.namedVariables = m_children.size();
	return true;
}
}
