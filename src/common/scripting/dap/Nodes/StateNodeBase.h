/*
** StateNodeBase.h
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

#include <cstring>
#include <dap/protocol.h>

namespace DebugServer
{
class StateNodeBase
{
	uint32_t m_id = 0;
	public:
	virtual ~StateNodeBase() = default;

	int GetId() const;

	void SetId(uint32_t id);
};

class RuntimeState;

class IProtocolVariableSerializable
{
	public:
	virtual bool SerializeToProtocol(dap::Variable &variable) = 0;
};

class IProtocolVariableSerializableWithName : public IProtocolVariableSerializable
{
	public:
	virtual std::string GetName() const = 0;
	virtual std::string GetEvalName() const = 0;
	virtual void SetName(const std::string &name) = 0;
	virtual void SetEvalName(const std::string &evalName) = 0;
};

class StateNodeNamedVariable : public StateNodeBase, public IProtocolVariableSerializableWithName
{
	protected:
	std::string m_name;
	std::string m_evalName;
	public:
	StateNodeNamedVariable(const std::string &name, const std::string &evalName = {});
	std::string GetName() const override;
	std::string GetEvalName() const override;
	void SetName(const std::string &name) override;
	void SetEvalName(const std::string &evalName) override;
	void SetVariableName(dap::Variable &variable);
	
};

class IProtocolScopeSerializable
{
	public:
	virtual bool SerializeToProtocol(dap::Scope &scope) = 0;
};

class IStructuredState
{
	public:
	virtual bool GetChildNames(std::vector<std::string> &names) = 0;
	virtual bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) = 0;
	virtual bool IsVirtualStructure() { return false; }
	virtual caseless_path_map<std::shared_ptr<StateNodeBase>> GetVirtualContainerChildren() { return {}; }
};
} // namespace DebugServer
