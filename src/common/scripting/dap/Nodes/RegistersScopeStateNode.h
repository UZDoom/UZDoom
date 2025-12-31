/*
** RegistersScopeStateNode.h
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

#pragma once

#include <common/scripting/dap/GameInterfaces.h>
#include <dap/protocol.h>

#include "StateNodeBase.h"

namespace DebugServer
{

class RegistersNode : public StateNodeBase, public IProtocolVariableSerializable, public IStructuredState
{
	protected:
	VMFrame *m_stackFrame;
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	std::string m_name;
	public:
	RegistersNode(std::string name, VMFrame *stackFrame);

			virtual std::string GetPrefix() const = 0;
			virtual int GetNumberOfRegisters() const = 0;

			virtual VMValue GetRegisterValue(int index) const = 0;

			virtual PType *GetRegisterType([[maybe_unused]] int index) const = 0;

			bool SerializeToProtocol(dap::Variable &variable) override;

			bool GetChildNames(std::vector<std::string> &names) override;

			bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};

class PointerRegistersNode : public RegistersNode
{
	public:
	int GetNumberOfRegisters() const override;

			std::string GetPrefix() const override { return "a"; }
			VMValue GetRegisterValue(int index) const override;

			PType *GetRegisterType([[maybe_unused]] int index) const override;

			PointerRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };

			bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node);
};

class StringRegistersNode : public RegistersNode
{
	public:
	std::string GetPrefix() const override { return "s"; }
	int GetNumberOfRegisters() const override;

	VMValue GetRegisterValue(int index) const override;

	PType *GetRegisterType([[maybe_unused]] int index) const override;

	StringRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };
};

class FloatRegistersNode : public RegistersNode
{
	public:
	std::string GetPrefix() const override { return "f"; }

			int GetNumberOfRegisters() const override;

	VMValue GetRegisterValue(int index) const override;

	PType *GetRegisterType([[maybe_unused]] int index) const override;

	FloatRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };
};

class IntRegistersNode : public RegistersNode
{
	public:
	std::string GetPrefix() const override { return "d"; }

			int GetNumberOfRegisters() const override;

	VMValue GetRegisterValue(int index) const override;

	PType *GetRegisterType([[maybe_unused]] int index) const override;

	IntRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };
};

class ParamsRegistersNode : public RegistersNode
{
public:
	std::string GetPrefix() const override { return ""; }

		int GetNumberOfRegisters() const override;

	VMValue GetRegisterValue(int index) const override;

	PType *GetRegisterType([[maybe_unused]] int index) const override;

	ParamsRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };

	bool SerializeToProtocol(dap::Variable &variable) override;
};

class SpecialSetupRegistersNode : public RegistersNode
{
	public:
	std::string GetPrefix() const override { return ""; }

	int GetNumberOfRegisters() const override;

	VMValue GetRegisterValue(int index) const override;

	PType *GetRegisterType([[maybe_unused]] int index) const override;

	SpecialSetupRegistersNode(std::string name, VMFrame *stackFrame) : RegistersNode(name, stackFrame) { };

	bool SerializeToProtocol(dap::Variable &variable) override;
};

class RegistersScopeStateNode : public StateNodeBase, public IProtocolScopeSerializable, public IStructuredState
{
	VMFrame *m_stackFrame;
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	public:
	RegistersScopeStateNode(VMFrame *stackFrame);

	bool SerializeToProtocol(dap::Scope &scope) override;

	bool GetChildNames(std::vector<std::string> &names) override;

	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};
}
