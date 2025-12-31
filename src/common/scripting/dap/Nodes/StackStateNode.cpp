/*
** StackStateNode.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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

#include <string>

#include "StackFrameStateNode.h"
#include "StackStateNode.h"
#include "common/scripting/dap/RuntimeState.h"
#include "common/scripting/dap/Utilities.h"

namespace DebugServer
{
StackStateNode::StackStateNode(const uint32_t stackId) : m_stackId(stackId) { }

bool StackStateNode::SerializeToProtocol(dap::Thread &thread) const
{
	thread.id = m_stackId;

	std::vector<VMFrame *> frames;
	RuntimeState::GetStackFrames(m_stackId, frames);

	if (frames.empty())
	{
		thread.name = StringFormat("(%ld)",  static_cast<int64_t>(thread.id));
	}
	else
	{
		const auto frame = frames.back();
		const auto name = frame->Func ? frame->Func->PrintableName : "<unknown>";
		thread.name = StringFormat("%s (%ld)", name, static_cast<int64_t>(thread.id));
	}

	return true;
}

bool StackStateNode::GetChildNames(std::vector<std::string> &names)
{
	if (!m_children.empty())
	{
		for (const auto &child : m_children)
		{
			names.push_back(std::to_string(child.first));
		}
		return true;
	}
	std::vector<VMFrame *> frames;
	RuntimeState::GetStackFrames(m_stackId, frames);

	size_t frameNum = 0;
	for (size_t i = 0; i < frames.size(); i++)
	{
		if (PCIsAtNativeCall(frames.at(i)))
		{
			names.push_back(std::to_string(frameNum));
			m_children[frameNum] = std::make_shared<StackFrameStateNode>(GetCalledFunction(frames.at(i)), frames.at(i));
			frameNum++;
		}
		names.push_back(std::to_string(frameNum));
		m_children[frameNum] = std::make_shared<StackFrameStateNode>(frames.at(i));
		frameNum++;
	}

	return true;
}

bool StackStateNode::GetChildNode(const std::string name, std::shared_ptr<StateNodeBase> &node)
{
	int level;
	if (!ParseInt(name, &level))
	{
		return false;
	}
	if (m_children.empty())
	{
		std::vector<std::string> names;
		GetChildNames(names);
	}
	if (m_children.find(level) != m_children.end())
	{
		node = m_children[level];
		return true;
	}
	return false;
}
}
