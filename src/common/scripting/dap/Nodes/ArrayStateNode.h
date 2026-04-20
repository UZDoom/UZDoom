/*
** ArrayStateNode.h
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

#pragma once
#include <common/scripting/dap/GameInterfaces.h>

#include <dap/protocol.h>
#include <map>
#include "StateNodeBase.h"

namespace DebugServer
{
class ArrayStateNode : public StateNodeNamedVariable, public IStructuredState
{

	const VMValue m_value;
	PType *m_type;
	PType *m_elementType;
	std::map<std::string, std::shared_ptr<StateNodeBase>> m_children;
	public:
	ArrayStateNode(std::string name, VMValue value, PType *type);

	virtual ~ArrayStateNode() override = default;

	bool SerializeToProtocol(dap::Variable &variable) override;

	bool GetChildNames(std::vector<std::string> &names) override;
	bool CacheChildren();
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};
}
