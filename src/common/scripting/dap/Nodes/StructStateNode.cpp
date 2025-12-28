/*
** StructStateNode.cpp
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

#include "StructStateNode.h"
#include "types.h"
#include <common/scripting/dap/Utilities.h>
#include <common/scripting/dap/RuntimeState.h>
#include <common/objects/dobject.h>
#include <common/scripting/core/symbols.h>

namespace DebugServer
{ // std::string name, VMValue* value, PType* knownType
StructStateNode::StructStateNode(std::string name, VMValue value, PType *knownType, const VMFrame * currentFrame) : StateNodeNamedVariable(name), m_value(value), m_type(knownType), m_currentFrame(currentFrame) {}

bool StructStateNode::SerializeToProtocol(dap::Variable &variable)
{
	if (m_children.empty())
	{
		CacheState();
	}
	// If this is a valid heap/stack allocated struct or one that is optimized to be stored in the stackframe registers
	bool inRegisters = !m_value.a && m_currentFrame && m_structInfo.StructFields.size();
	bool valid = m_structInfo.IsValid() && (inRegisters || IsVMValueValid(&m_value));
	variable.variablesReference = valid ? GetId() : 0;
	variable.namedVariables = m_structInfo.StructFields.size();
	SetVariableName(variable);
	variable.type = m_type->DescriptiveName();
	auto typeval = variable.type.value("");
	// check if name begins with 'Pointer<'; if so, remove it, and the trailing '>'
	if (typeval.size() > 9 && typeval.find("Pointer<") == 0 && typeval[typeval.size() - 1] == '>')
	{
		typeval = typeval.substr(8, typeval.size() - 9);
	}
	if (inRegisters)
	{
		variable.value = StringFormat("%s <REGISTERS>", typeval.c_str());
	} 
	else if (!valid)
	{
		variable.value = StringFormat("%s <NULL>", typeval.c_str());
	}
	else
	{
		variable.value = StringFormat("%s (%p)", typeval.c_str(), m_value.a);
	}
	return true;
}


bool StructStateNode::GetChildNames(std::vector<std::string> &names)
{
	if (m_children.empty())
	{
		CacheState();
	}
	for (auto &field : m_structInfo.StructFields)
	{
		names.push_back(field.Name);
	}
	return true;
}

void StructStateNode::CacheState()
{
	m_structInfo = GetStructState(m_name, m_value, m_type, m_currentFrame);
	for (auto &field : m_structInfo.StructFields)
	{
		m_children[field.Name] = RuntimeState::CreateNodeForVariable(field.Name, field.Value, field.Type, nullptr);
	}
}

bool StructStateNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (m_children.empty())
	{
		CacheState();
	}
	if (m_children.find(name) != m_children.end())
	{
		node = m_children[name];
		return true;
	}
	return false;
}
}
