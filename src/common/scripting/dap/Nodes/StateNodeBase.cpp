/*
** StateNodeBase.cpp
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

#include "StateNodeBase.h"
namespace DebugServer
{
int StateNodeBase::GetId() const
{
	return m_id;
}

void StateNodeBase::SetId(const uint32_t id)
{
	m_id = id;
}

StateNodeNamedVariable::StateNodeNamedVariable(const std::string &name, const std::string &evalName)
	: m_name(name), m_evalName(evalName.empty() ? name : evalName)
{
}
std::string StateNodeNamedVariable::GetName() const
{
	return m_name;
}
std::string StateNodeNamedVariable::GetEvalName() const
{
	return m_evalName;
}
void StateNodeNamedVariable::SetName(const std::string &name)
{
	m_name = name;
}
void StateNodeNamedVariable::SetEvalName(const std::string &evalName)
{
	m_evalName = evalName;
}
void StateNodeNamedVariable::SetVariableName(dap::Variable &variable)
{
	variable.name = m_name;
	if (m_evalName != m_name)
	{
		variable.evaluateName = m_evalName;
	}
}

} // namespace DebugServer
