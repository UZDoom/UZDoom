/*
** StackStateNode.h
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
#include "StateNodeBase.h"

namespace DebugServer
{
class StackStateNode : public StateNodeBase, public IStructuredState
{
	uint32_t m_stackId;
	std::map<size_t, std::shared_ptr<StateNodeBase>> m_children;
	public:
	StackStateNode(uint32_t stackId);

	bool SerializeToProtocol(dap::Thread &thread) const;

	bool GetChildNames(std::vector<std::string> &names) override;
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};
}
