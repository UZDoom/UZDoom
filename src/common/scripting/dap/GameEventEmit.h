/*
** GameEventEmit.h
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
#include "vm.h"

class VMFrameStack;

namespace DebugServer
{
namespace RuntimeEvents
{
	void EmitInstructionExecutionEvent(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc);
	void EmitLogEvent(int level, const char *message);
	void EmitExceptionEvent(EVMAbortException reason, const std::string &message, const std::string &stackTrace);
	bool IsDebugServerRunning();
}
}
