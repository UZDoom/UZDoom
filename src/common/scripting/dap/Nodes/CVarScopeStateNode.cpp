/*
** CVarScopeStateNode.cpp
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

#include "CVarScopeStateNode.h"
#include <common/scripting/dap/Utilities.h>
#include <common/scripting/dap/RuntimeState.h>
#include "common/console/c_cvars.h"
#include "gstrings.h"

namespace DebugServer
{
CVarStateNode::CVarStateNode(FBaseCVar *cvar) : m_cvar(cvar) { }

dap::Variable CVarStateNode::ToVariable(FBaseCVar *m_cvar)
{
	dap::Variable variable;
	if (!m_cvar)
	{
		variable.name = "<INVALID>";
		variable.value = "<INVALID>";
		return variable;
	}


	const char *vmvalstr = nullptr;
	const char *realTypeString = nullptr;
	ECVarType favoriteType;
	UCVarValue val = m_cvar->GetFavoriteRep(&favoriteType);
	// VMValue vmval;
	// PType *vmtype;
	// switch (type)
	// {
	// 	case CVAR_Flag:
	// 	case CVAR_Bool:
	// 		vmval = VMValue(val.Bool);
	// 		vmtype = TypeBool;
	// 		break;
	// 	case CVAR_Int:
	// 	case CVAR_Color:
	// 	case CVAR_Mask:
	// 		vmval = VMValue(val.Int);
	// 		vmtype = TypeSInt32;
	// 		break;
	// 	case CVAR_Float:
	// 		vmval = VMValue(val.Float);
	// 		vmtype = TypeFloat64;
	// 		break;
	// 	case CVAR_String:
	// 		{
	// 			vmvalstr = val.String;
	// 			vmval = VMValue(&vmvalstr);
	// 			vmtype = TypeString;
	// 			auto test = vmval.sp;
	// 			int i = 0;
	// 		}
	// 		break;
	// 	default:
	// 		break;
	// }
	variable.value = m_cvar->GetHumanString();

	switch (m_cvar->GetRealType())
	{
	// actually do these in order of the enum
	case CVAR_Bool:
		realTypeString = "CVar<Bool>";
		break;
	case CVAR_Int:
		realTypeString = "CVar<Int>";
		break;
	case CVAR_Float:
		realTypeString = "CVar<Float>";
		if (variable.value.find('.') == std::string::npos && variable.value.find('e') == std::string::npos)
		{
			variable.value += ".0";
		}
		break;
	case CVAR_String:
	{
		realTypeString = "CVar<String>";
		// If none of the values are non-numbers or non-'.', then we need to wrap it in quotes
		if (favoriteType == CVAR_String)
		{
			variable.value = StringFormat("\"%s\"", variable.value.c_str());
		}
	}
	break;
	case CVAR_Color:
		realTypeString = "CVar<Color>";
		break;
	case CVAR_Flag:
		realTypeString = "CVar<Flag>";
		break;
	case CVAR_Mask:
		realTypeString = "CVar<Mask>";
		break;
	case CVAR_Dummy:
		realTypeString = "CVar<Dummy>";
		break;
	default:
		realTypeString = "<UNKNOWN>";
		break;
	}
	const FString &description = m_cvar->GetDescription();


	if (!description.IsEmpty())
	{
		std::string_view localized = GStrings.localize(description.GetChars());
		if (!localized.empty() && localized.substr(1) != description.GetChars())
		{
			variable.type = StringFormat("%s (%s)", realTypeString, localized.data());
		}
		else
		{
			variable.type = realTypeString;
		}
	}
	else
	{
		variable.type = realTypeString;
	}
	variable.name = m_cvar->GetName();

	return variable;
}

bool CVarStateNode::SerializeToProtocol(dap::Variable &variable)
{
	variable = ToVariable(m_cvar);
	return true;
}

bool CVarScopeStateNode::SerializeToProtocol(dap::Scope &scope)
{
	scope.name = "CVars";
	scope.expensive = true;
	scope.presentationHint = "cvars";
	scope.variablesReference = GetId();

	if (m_CachedNames.empty())
	{
		std::vector<std::string> childNames;
		GetChildNames(childNames);
	}
	scope.namedVariables = m_CachedNames.size();
	scope.indexedVariables = 0;

	return true;
}

bool CVarScopeStateNode::GetChildNames(std::vector<std::string> &names)
{
	if (!m_CachedNames.empty())
	{
		names = m_CachedNames;
		return true;
	}
	decltype(cvarMap)::Iterator it(cvarMap);
	decltype(cvarMap)::Pair *pair;
	m_CachedNames.reserve(cvarMap.CountUsed());
	while (it.NextPair(pair))
	{
		auto var = pair->Value;
		m_CachedNames.emplace_back(var->GetName());
		m_children[var->GetName()] = std::make_shared<CVarStateNode>(var);
	}
	std::sort(m_CachedNames.begin(), m_CachedNames.end(), CVarNameComparer());
	names = m_CachedNames;
	return true;
}

bool CVarScopeStateNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (m_CachedNames.empty())
	{
		std::vector<std::string> childNames;
		GetChildNames(childNames);
	}
	if (m_children.find(name) == m_children.end())
	{
		auto var = FindCVar(name.c_str(), nullptr);
		if (!var)
		{
			return false;
		}
		m_children[var->GetName()] = std::make_shared<CVarStateNode>(var);
		m_CachedNames.push_back(var->GetName());
		std::sort(m_CachedNames.begin(), m_CachedNames.end(), CVarNameComparer());
	}
	if (m_children.find(name) != m_children.end())
	{
		node = m_children[name];
		return true;
	}
	return false;
}
}
