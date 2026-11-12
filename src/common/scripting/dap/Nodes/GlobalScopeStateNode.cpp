/*
** GlobalScopeStateNode.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: MIT
**
**---------------------------------------------------------------------------
**
*/

#include "GlobalScopeStateNode.h"
#include <common/scripting/dap/Utilities.h>
#include <common/scripting/dap/RuntimeState.h>

namespace DebugServer
{

GlobalScopeStateNode::GlobalScopeStateNode() { }

bool GlobalScopeStateNode::SerializeToProtocol(dap::Scope &scope)
{
	scope.name = "Global";
	scope.expensive = false;
	scope.presentationHint = "globals";
	scope.variablesReference = GetId();

	std::vector<std::string> childNames;
	GetChildNames(childNames);

	scope.namedVariables = childNames.size();
	scope.indexedVariables = 0;

	return true;
}

bool GlobalScopeStateNode::GetChildNames(std::vector<std::string> &names)
{
	for (auto field : AutoSegs::ClassFields.fields)
	{
		if (strlen(field->ClassName) == 0){
			names.push_back(field->FieldName);
		}
	}
	std::sort(names.begin(), names.end(), ci_less());
	return true;
}

bool GlobalScopeStateNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (m_children.empty())
	{
		for (auto f : AutoSegs::ClassFields.fields)
		{
			if (strlen(f->ClassName) == 0) {
				for (auto ns : Namespaces.AllNamespaces)
				{
					if (auto it = ns->Symbols.FindSymbol(f->FieldName, true); it != nullptr)
					{
						if (PField *field = dyn_cast<PField>(it); field)
						{
							if (field->Offset != f->FieldOffset)
							{
								continue;
							}
							// the offset is the address of the field
							void *addr = (void *)(field->Offset);
							VMValue val = GetVMValue(addr, field->Type, field->BitValue);
							m_children[f->FieldName] = RuntimeState::CreateNodeForVariable(f->FieldName, val, field->Type);
							break;
						}
					}
				}

			}
		}
	}
	if (m_children.find(name) != m_children.end())
	{
		node = m_children[name];
		return true;
	}
	return false;
}

}
