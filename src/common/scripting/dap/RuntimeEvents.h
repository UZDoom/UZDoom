/*
** RuntimeEvents.h
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
#include <functional>
#include "vm.h"
#include "GameEventEmit.h"

namespace dap
{
struct Breakpoint;
}

#define EVENT_DECLARATION(NAME, HANDLER_SIGNATURE)                 \
    typedef std::function<HANDLER_SIGNATURE> NAME## EventHandle;    \
    NAME##EventHandle SubscribeTo##NAME(std::function<HANDLER_SIGNATURE> handler); \
    bool UnsubscribeFrom##NAME(NAME##EventHandle handle);


namespace dap
{
struct Breakpoint;
}

namespace DebugServer
{
namespace RuntimeEvents
{
	EVENT_DECLARATION(InstructionExecution, void(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc))
	EVENT_DECLARATION(CreateStack, void(VMFrameStack *))
	EVENT_DECLARATION(CleanupStack, void(uint32_t))
	EVENT_DECLARATION(Log, void(int level, const char *message))
	EVENT_DECLARATION(BreakpointChanged, void(const dap::Breakpoint &bpoint, const std::string &))
	EVENT_DECLARATION(ExceptionThrown, void(EVMAbortException reason, const std::string &message, const std::string &stackTrace))
	EVENT_DECLARATION(DebuggerEnabled, bool(void))

	void EmitBreakpointChangedEvent(const dap::Breakpoint &bpoint, const std::string &what);
}
}

#undef EVENT_DECLARATION
