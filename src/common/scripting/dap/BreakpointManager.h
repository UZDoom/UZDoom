/*
** BreakpointManager.h
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
#include <map>
#include <set>
#include <dap/protocol.h>
#include <dap/session.h>

#include "GameInterfaces.h"
#include "IdProvider.h"

#include "PexCache.h"

namespace DebugServer
{
class BreakpointManager
{
	public:
	struct BreakpointInfo
	{
		enum class Type
		{
			NONE = -1,
			Line,
			Function,
			Instruction
		};
		Type type;
		int ref;
		std::string funcBreakpointText;
		const char *nativeFuncName;
		dap::Breakpoint bpoint;
	};

	struct ScriptBreakpoints
	{
		int ref {-1};
		dap::Source source;
		std::time_t modificationTime {0};
		std::map<void *, BreakpointInfo> breakpoints;
	};


	explicit BreakpointManager(PexCache *pexCache) : m_pexCache(pexCache) { }
	int64_t GetBreakpointID();
	int AddInvalidBreakpoint(
		std::vector<dap::Breakpoint> &breakpoints, int line, void *address, const std::string &reason, const dap::optional<dap::Source> &source);
	bool AddBreakpointInfo(
		const std::shared_ptr<Binary> &binary,
		VMScriptFunction *function,
		int line,
		void *p_instrRef,
		int offset,
		BreakpointInfo::Type type,
		std::vector<dap::Breakpoint> &r_bpoint,
		const std::string &funcText = {});
	void GetBpointsForResponse(BreakpointInfo::Type type, std::vector<dap::Breakpoint> &responseBpoints);
	dap::ResponseOrError<dap::SetBreakpointsResponse> SetBreakpoints(const dap::Source &src, const dap::SetBreakpointsRequest &request);
	dap::ResponseOrError<dap::SetFunctionBreakpointsResponse> SetFunctionBreakpoints(const dap::SetFunctionBreakpointsRequest &request);
	dap::ResponseOrError<dap::SetInstructionBreakpointsResponse> SetInstructionBreakpoints(const dap::SetInstructionBreakpointsRequest &request);
	void ClearBreakpoints(bool emitChanged = false);
	void ClearBreakpointsForScript(int ref, BreakpointInfo::Type type, bool emitChanged = false);
	bool GetExecutionIsAtValidBreakpoint(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc);
	inline bool IsAtNativeBreakpoint(VMFrameStack *stack);
	void SetBPStoppedEventInfo(VMFrameStack *stack, dap::StoppedEvent &event);
	private:

	PexCache *m_pexCache;
	std::map<void *, std::vector<BreakpointInfo>> m_breakpoints;
	// set of case-insensitive strings
	std::map<std::string_view, BreakpointInfo, ci_less> m_nativeFunctionBreakpoints;
	IdProvider m_idProvider;
	int64_t m_CurrentID = 1;
	size_t times_seen = 0;

	void ClearBreakpointsType(BreakpointInfo::Type type);
};
}
