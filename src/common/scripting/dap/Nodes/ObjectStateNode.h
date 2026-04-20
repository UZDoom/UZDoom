/*
** ObjectStateNode.h
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
#include <map>

namespace DebugServer
{
class ObjectStateNode : public StateNodeNamedVariable, public IStructuredState
{
	bool m_subView;

	const VMValue m_value;
	PType *m_ClassType;
	std::vector<std::string> m_cachedNames; // to ensure proper order of children
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_children;
	caseless_path_map<std::shared_ptr<StateNodeBase>> m_virtualChildren;
	PType *m_VMType = nullptr;
	public:
	ObjectStateNode(const std::string &name, VMValue value, PType *asClass, bool subView = false);

	bool SerializeToProtocol(dap::Variable &variable) override;

	bool GetChildNames(std::vector<std::string> &names) override;
	bool GetChildNode(std::string name, std::shared_ptr<StateNodeBase> &node) override;
	void Reset();
	bool IsVirtualStructure() override { return m_subView; }
	caseless_path_map<std::shared_ptr<StateNodeBase>> GetVirtualContainerChildren() override;
};
}
