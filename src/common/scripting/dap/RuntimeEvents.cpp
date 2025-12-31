/*
** RuntimeEvents.cpp
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

#define XBYAK_NO_OP_NAMES

#include "RuntimeEvents.h"

#include <cassert>
#include <dap/protocol.h>
#include "GameInterfaces.h"
#include "common/scripting/vm/vmintern.h"

namespace DebugServer
{
namespace RuntimeEvents
{
#define EVENT_WRAPPER_IMPL(NAME, HANDLER_SIGNATURE)                               \
	bool g_## NAME## EventActive = false;																    \
	std::function<HANDLER_SIGNATURE> g_## NAME## Event;                     \
                                                                                  \
	NAME##EventHandle SubscribeTo##NAME(std::function<HANDLER_SIGNATURE> handler) \
	{                                                                             \
		g_## NAME## Event = handler;\
		g_## NAME## EventActive = true;                                               \
		return handler;\
	}\
	\
	bool UnsubscribeFrom## NAME(NAME## EventHandle handle)\
	{\
		if (!handle)																															\
			return false;                                                         \
		g_## NAME## EventActive = false;                                               \
		g_## NAME## Event  = nullptr;\
		return true;\
	}

	EVENT_WRAPPER_IMPL(InstructionExecution, void(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc))
	EVENT_WRAPPER_IMPL(CreateStack, void(VMFrameStack *))
	EVENT_WRAPPER_IMPL(CleanupStack, void(uint32_t))
	EVENT_WRAPPER_IMPL(Log, void(int level, const char *msg))
	EVENT_WRAPPER_IMPL(BreakpointChanged, void(const dap::Breakpoint &bpoint, const std::string &))
	EVENT_WRAPPER_IMPL(ExceptionThrown, void(EVMAbortException reason, const std::string &message, const std::string &stackTrace))
	EVENT_WRAPPER_IMPL(DebuggerEnabled, bool(void))

#undef EVENT_WRAPPER_IMPL


	void EmitBreakpointChangedEvent(const dap::Breakpoint &bpoint, const std::string &what)
	{
		if (g_BreakpointChangedEventActive)
		{
			g_BreakpointChangedEvent(bpoint, what);
		}
	}
	void EmitInstructionExecutionEvent(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc)
	{
		if (g_InstructionExecutionEventActive)
		{
			g_InstructionExecutionEvent(stack, ret, numret, pc);
		}
	}
	void EmitLogEvent(int level, const char *msg)
	{
		if (g_LogEventActive)
		{
			g_LogEvent(level, msg);
		}
	}
	void EmitExceptionEvent(EVMAbortException reason, const std::string &message, const std::string &stackTrace)
	{
		if (g_ExceptionThrownEventActive)
		{
			g_ExceptionThrownEvent(reason, message, stackTrace);
		}
	}

	bool IsDebugServerRunning()
	{
		if (g_DebuggerEnabledEventActive){
			return g_DebuggerEnabledEvent();
		}
		return false;
	}

	// TODO: Are CreateStack and CleanupStack events needed? VM execution is single-threaded and there's only one stack.
	// Maybe an event when the last frame gets popped off, but I'm not sure what would even need that.

}
}
