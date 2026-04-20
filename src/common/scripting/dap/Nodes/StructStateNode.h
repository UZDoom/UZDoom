/*
** StructStateNode.h
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
class StructStateNode : public StateNodeNamedVariable, public IStructuredState
{
	StructInfo m_structInfo;
	const VMValue m_value;
	PType *m_type;
	const VMFrame * m_currentFrame = nullptr;
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	void CacheState();
	public:
	StructStateNode(std::string name, VMValue value, PType *knownType, const VMFrame *currentFrame = nullptr);

	bool SerializeToProtocol(dap::Variable &variable) override;

	bool GetChildNames(std::vector<std::string> &names) override;
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
};
}
