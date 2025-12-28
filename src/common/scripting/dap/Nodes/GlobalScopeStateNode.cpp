/*
** GlobalScopeStateNode.cpp
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

#include "GlobalScopeStateNode.h"
#include <common/scripting/dap/Utilities.h>
#include <common/scripting/dap/RuntimeState.h>

namespace DebugServer
{
// TODO: Do this dynamically?
static const char *const GlobalNames[] = {
	"NotifyFontScale",
	"ConsoleState",
	"menuactive",
	"BackbuttonTime",
	"BackbuttonAlpha",
	"GameTicRate",
	"menuDelegate",
	"WP_NOCHANGE",
	"SmallFont",
	"SmallFont2",
	"BigFont",
	"ConFont",
	"NewConsoleFont",
	"NewSmallFont",
	"AlternativeSmallFont",
	"AlternativeBigFont",
	"OriginalSmallFont",
	"OriginalBigFont",
	"IntermissionFont",
	"CleanXfac",
	"CleanYfac",
	"CleanWidth",
	"CleanHeight",
	"CleanXfac_1",
	"CleanYfac_1",
	"CleanWidth_1",
	"CleanHeight_1",
	"AllServices",
	"Bindings",
	"AutomapBindings",
	"generic_ui",
	"deh",
	"gameinfo",
	"Teams",
	"LocalViewPitch",
	"StatusBar",
	"players",
	"playeringame",
	"PlayerClasses",
	"consoleplayer",
	"validcount",
	"multiplayer",
	"gameaction",
	"gamestate",
	"skyflatnum",
	"globalfreeze",
	"gametic",
	"demoplayback",
	"automapactive",
	"viewactive",
	"Net_Arbitrator",
	"netgame",
	"paused",
	"Terrains",
	"OptionMenuSettings",
	"musplaying",
	"AllClasses",
	"Level",
	// "level", technically its own global, but only the VM one is accessible by the VM
	"AllActorClasses",

	nullptr};

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
	for (int i = 0; GlobalNames[i] != nullptr; i++)
	{
		names.push_back(GlobalNames[i]);
	}
	return true;
}

bool GlobalScopeStateNode::GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node)
{
	if (m_children.empty())
	{
		std::vector<std::string> childNames;
		GetChildNames(childNames);
		caseless_path_set childSet {childNames.begin(), childNames.end()};
		for (auto ns : Namespaces.AllNamespaces)
		{
			auto symbolIter = ns->Symbols.GetIterator();
			PSymbolTable::MapType::Pair *pair;
			while (symbolIter.NextPair(pair))
			{
				if (childSet.find(pair->Key.GetChars()) != childSet.end())
				{
					std::string symname = pair->Key.GetChars();
					PSymbol *val = pair->Value;
					if (val->SymbolName == NAME_None)
					{
						continue;
					}
					PField *field = dyn_cast<PField>(val);
					if (field)
					{
						field->Type;
						// the offset is the address of the field
						void *addr = (void *)(field->Offset);
						VMValue val = GetVMValue(addr, field->Type, field->BitValue);
						m_children[symname] = RuntimeState::CreateNodeForVariable(symname, val, field->Type);
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
