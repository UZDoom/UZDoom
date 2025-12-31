/*
** DebugExecutionManager.h
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

#pragma once

#include "GameInterfaces.h"

#include "BreakpointManager.h"
#include "RuntimeState.h"
#include <dap/session.h>
#include <mutex>
#include <atomic>

namespace DebugServer
{
enum StepType
{
	STEP_IN = 0,
	STEP_OVER,
	STEP_OUT
};

enum StepGranularity
{
	kInstruction = 0,
	kLine,
	kStatement
};

class DebugExecutionManager
{

public:
	enum class DebuggerState
	{
		kRunning = 0,
		kPaused,
		kStepping
	};
	enum class pauseReason
	{
		CONTINUING = -1,
		NONE = 0,
		step,
		breakpoint,
		paused,
		exception
	};
	enum class ExceptionFilter
	{
		kScript,
		kMAX
	};
	private:
	std::mutex m_instructionMutex;
	bool m_closed;

	std::shared_ptr<dap::Session> m_session;
	RuntimeState *m_runtimeState;
	BreakpointManager *m_breakpointManager;

	std::atomic<DebuggerState> m_state = DebuggerState::kRunning;
	std::atomic<uint32_t> m_currentStepStackId = 0;
	StepType m_currentStepType = StepType::STEP_IN;
	StepGranularity m_granularity;
	int m_lastLine = -1;
	const VMOP *m_lastInstruction = nullptr;
	VMFrame *m_currentStepStackFrame;
	VMFunction *m_currentVMFunction;
	std::set<ExceptionFilter> m_exceptionFilters = {ExceptionFilter::kScript};
	public:
	explicit DebugExecutionManager(RuntimeState *runtimeState, BreakpointManager *breakpointManager) : m_closed(true), m_runtimeState(runtimeState), m_breakpointManager(breakpointManager), m_currentStepStackFrame(nullptr)
	{
	}
	static dap::array<dap::ExceptionBreakpointsFilter> GetAllExceptionFilters();


	void Close();
	void HandleInstruction(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc);
	void HandleException(EVMAbortException reason, const std::string &message, const std::string &stackTrace);
	void Open(std::shared_ptr<dap::Session> ses);
	bool Continue();
	bool Pause();
	bool Step(uint32_t stackId, StepType stepType, StepGranularity stepGranularity);
	dap::array<dap::Breakpoint> SetExceptionBreakpointFilters(const std::vector<std::string> &filterIds);
	static ExceptionFilter GetFilterID(const std::string &filter_string);
	bool IsPaused() const { return m_state == DebuggerState::kPaused; }
	private:
	inline pauseReason CheckState(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc);
			void ResetStepState(DebuggerState state, VMFrameStack *stack);
			void WaitWhilePaused(pauseReason pauseReason, VMFrameStack *stack);
};
}
