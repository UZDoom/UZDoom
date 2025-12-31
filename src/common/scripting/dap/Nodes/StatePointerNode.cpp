/*
** StatePointerNode.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
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
#include "StatePointerNode.h"
#include "ValueStateNode.h"
#include "common/scripting/dap/Utilities.h"

#include "dobject.h"
#include "info.h"
#include "symbols.h"
#include "types.h"

namespace DebugServer
{
StatePointerNode::StatePointerNode(std::string name, VMValue value, PClass *owningType) : StateNodeNamedVariable(name), m_value(value), m_OwningType(owningType) { }
void DumpStateHelper(FStateLabels *StateList, const FString &prefix)
{
	for (int i = 0; i < StateList->NumLabels; i++)
	{
		auto state = StateList->Labels[i].State;
		if (state != NULL)
		{
			const PClassActor *owner = FState::StaticFindStateOwner(state);
			if (owner == NULL)
			{
				if (state->DehIndex >= 0) Printf(PRINT_NONOTIFY, "%s%s: DehExtra %d\n", prefix.GetChars(), StateList->Labels[i].Label.GetChars(), state->DehIndex);
				else
					Printf(PRINT_NONOTIFY, "%s%s: invalid\n", prefix.GetChars(), StateList->Labels[i].Label.GetChars());
			}
			else
			{
				Printf(PRINT_NONOTIFY, "%s%s: %s\n", prefix.GetChars(), StateList->Labels[i].Label.GetChars(), FState::StaticGetStateName(state).GetChars());
			}
		}
		if (StateList->Labels[i].Children != NULL)
		{
			DumpStateHelper(StateList->Labels[i].Children, prefix + '.' + StateList->Labels[i].Label.GetChars());
		}
	}
}

bool StatePointerNode::SerializeToProtocol(dap::Variable &variable)
{
	variable.variablesReference = IsVMValueValid(&m_value) ? GetId() : 0;
	variable.namedVariables = 0;
	std::vector<std::string> names;
	GetChildNames(names);
	variable.namedVariables = names.size();
	SetVariableName(variable);
	variable.type = "StatePointer";
	if (!IsVMValueValid(&m_value))
	{
		variable.value = "<NULL>";
	}
	else
	{
		auto *state = static_cast<FState *>(m_value.a);
		auto *owner = FState::StaticFindStateOwner(state);
		FName label = NAME_None;
		if (owner)
		{
			for (int i = 0; i < owner->GetStateLabels()->NumLabels; i++)
			{
				if (owner->GetStateLabels()->Labels[i].State == state)
				{
					label = owner->GetStateLabels()->Labels[i].Label;
					break;
				}
			}
		}
		if (!owner || label == NAME_None)
		{
			variable.value = FState::StaticGetStateName(state).GetChars();
		}
		else
		{
			variable.value = StringFormat("%s.%s", owner->TypeName.GetChars(), label.GetChars());
		}
	}
	return true;
}

bool StatePointerNode::GetChildNames(std::vector<std::string> &names)
{

	names.push_back("NextState");
	names.push_back("sprite");
	names.push_back("Tics");
	names.push_back("TicRange");
	names.push_back("Light");
	names.push_back("StateFlags");
	names.push_back("Frame");
	names.push_back("UseFlags");
	names.push_back("DefineFlags");
	names.push_back("Misc1");
	names.push_back("Misc2");
	names.push_back("DehIndex");

	return true;
}

bool StatePointerNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (!IsVMValueValid(&m_value))
	{
		return false;
	}

	FState *state = static_cast<FState *>(m_value.a);
	if (!state)
	{
		return false;
	}
	if (CaseInsensitiveEquals(name, "NextState"))
	{
		node = std::make_shared<StatePointerNode>("NextState", state->NextState, m_OwningType);
		return true;
	}
	if (CaseInsensitiveEquals(name, "sprite"))
	{
		node = std::make_shared<ValueStateNode>("sprite", state->sprite, TypeSpriteID);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "Tics"))
	{
		node = std::make_shared<ValueStateNode>("Tics", state->Tics, TypeSInt16);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "TicRange"))
	{
		node = std::make_shared<ValueStateNode>("TicRange", state->TicRange, TypeUInt16);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "Light"))
	{
		node = std::make_shared<ValueStateNode>("Light", state->Light, TypeSInt16);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "StateFlags"))
	{
		std::vector<std::string> strings;
		if (state->StateFlags & STF_SLOW)
		{
			strings.push_back("STF_SLOW");
		}
		if (state->StateFlags & STF_FAST)
		{
			strings.push_back("STF_FAST");
		}
		if (state->StateFlags & STF_FULLBRIGHT)
		{
			strings.push_back("STF_FULLBRIGHT");
		}
		if (state->StateFlags & STF_NODELAY)
		{
			strings.push_back("STF_NODELAY");
		}
		if (state->StateFlags & STF_SAMEFRAME)
		{
			strings.push_back("STF_SAMEFRAME");
		}
		if (state->StateFlags & STF_CANRAISE)
		{
			strings.push_back("STF_CANRAISE");
		}
		if (state->StateFlags & STF_DEHACKED)
		{
			strings.push_back("STF_DEHACKED");
		}
		if (state->StateFlags & STF_CONSUMEAMMO)
		{
			strings.push_back("STF_CONSUMEAMMO");
		}
		std::string value = StringFormat("%hu (%s)", state->StateFlags, StringJoin(strings, " | ").c_str());
		node = std::make_shared<DummyNode>("StateFlags", value, "StateFlags");
		return true;
	}
	else if (CaseInsensitiveEquals(name, "Frame"))
	{
		node = std::make_shared<ValueStateNode>("Frame", state->Frame, TypeUInt8);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "UseFlags"))
	{
		std::vector<std::string> strings;
		if (state->UseFlags & SUF_ACTOR)
		{
			strings.push_back("SUF_ACTOR");
		}
		if (state->UseFlags & SUF_OVERLAY)
		{
			strings.push_back("SUF_OVERLAY");
		}
		if (state->UseFlags & SUF_WEAPON)
		{
			strings.push_back("SUF_WEAPON");
		}
		if (state->UseFlags & SUF_ITEM)
		{
			strings.push_back("SUF_ITEM");
		}
		// join them together with a ' | ' separator
		const char *const delim = " | ";
		std::ostringstream imploded;
		std::copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(imploded, delim));

		node = std::make_shared<ValueStateNode>("UseFlags", state->DefineFlags, TypeUInt8);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "DefineFlags"))
	{
		std::string value = std::to_string(state->DefineFlags) + " ";
		switch (state->DefineFlags)
		{
		case SDF_NEXT:
			value += "SDF_NEXT";
			break;
		case SDF_STATE:
			value += "SDF_STATE";
			break;
		case SDF_STOP:
			value += "SDF_STOP";
			break;
		case SDF_WAIT:
			value += "SDF_WAIT";
			break;
		case SDF_LABEL:
			value += "SDF_LABEL";
			break;
		case SDF_INDEX:
			value += "SDF_INDEX";
			break;
		case SDF_MASK:
			value += "SDF_MASK";
			break;
		default:
			value += "<INVALID>";
			break;
		}
		node = std::make_shared<DummyNode>("DefineFlags", value, "DefineFlags");
		return true;
	}
	else if (CaseInsensitiveEquals(name, "Misc1"))
	{
		node = std::make_shared<ValueStateNode>("Misc1", state->Misc1, TypeSInt32);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "Misc2"))
	{
		node = std::make_shared<ValueStateNode>("Misc2", state->Misc2, TypeSInt32);
		return true;
	}
	else if (CaseInsensitiveEquals(name, "DehIndex"))
	{
		node = std::make_shared<ValueStateNode>("DehIndex", state->DehIndex, TypeSInt32);
		return true;
	}

	return true;
}
}
