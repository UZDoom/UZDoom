/*
** CVarScopeStateNode.h
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

class FBaseCVar;
namespace DebugServer
{

class CVarStateNode : public StateNodeBase, public IProtocolVariableSerializable{
	FBaseCVar *m_cvar = nullptr;
	public:
	CVarStateNode(FBaseCVar *cvar);
	bool SerializeToProtocol(dap::Variable &variable) override;
	static dap::Variable ToVariable(FBaseCVar *cvar);
};

class CVarNameComparer {
public:
	bool operator()(const std::string &a, const std::string &b) const {
		if (!a.empty() && !b.empty()) {
			// check if the first character is uppercase and the other is lowercase
			// uppercase goes last
			if (isupper(a[0]) && islower(b[0])) {
				return false;
			}
			if (islower(a[0]) && isupper(b[0])) {
				return true;
			}
		}
		return a < b;
	}
};

class CVarScopeStateNode  : public StateNodeBase, public IProtocolScopeSerializable, public IStructuredState
{
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	std::vector<std::string> m_CachedNames; // to preserve order
public:
	CVarScopeStateNode() = default;

	bool SerializeToProtocol(dap::Scope &scope) override;
	bool GetChildNames(std::vector<std::string> &names) override;
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};
} 
