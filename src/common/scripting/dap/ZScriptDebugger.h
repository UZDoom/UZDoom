/*
** ZScriptDebugger.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025 UZDoom Maintainers and Contributors
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

#include <dap/protocol.h>
#include <dap/session.h>
#include <dap/traits.h>
#include "RuntimeEvents.h"
#include "PexCache.h"
#include "BreakpointManager.h"
#include "DebugExecutionManager.h"
#include "IdMap.h"
#include "Protocol/struct_extensions.h"

#include <thread>

namespace DebugServer
{
enum DisconnectAction
{
	DisconnectDefault, // Attach -> Detach, Launch -> Terminate
	DisconnectTerminate,
	DisconnectDetach
};

enum VariablesFilter
{
	VariablesNamed,
	VariablesIndexed,
	VariablesBoth
};
class ZScriptDebugger
{
	template <typename T> using IsEvent = dap::traits::EnableIfIsType<dap::Event, T>;
	public:
	ZScriptDebugger();
	~ZScriptDebugger();

	void StartSession(std::shared_ptr<dap::Session> session);
	bool EndSession(bool closed = false);
	bool IsJustMyCode() const { return false; }
	void SetJustMyCode(bool enable) { }
	dap::ResponseOrError<dap::EvaluateResponse> Evaluate(const dap::EvaluateRequest &request);
	;
	template <typename T, typename = IsEvent<T>> void SendEvent(const T &event);
	bool IsEndingSession();
	int GetLastStoppedThreadId() { return 0; }

	dap::ResponseOrError<dap::InitializeResponse> Initialize(const dap::InitializeRequest &request);
	dap::ResponseOrError<dap::LaunchResponse> Launch(const dap::PDSLaunchRequest &request);
	dap::ResponseOrError<dap::AttachResponse> Attach(const dap::PDSAttachRequest &request);
	dap::ResponseOrError<dap::ContinueResponse> Continue(const dap::ContinueRequest &request);
	dap::ResponseOrError<dap::PauseResponse> Pause(const dap::PauseRequest &request);
	dap::ResponseOrError<dap::ThreadsResponse> GetThreads(const dap::ThreadsRequest &request);
	dap::ResponseOrError<dap::SetBreakpointsResponse> SetBreakpoints(const dap::SetBreakpointsRequest &request);
	dap::ResponseOrError<dap::SetFunctionBreakpointsResponse> SetFunctionBreakpoints(const dap::SetFunctionBreakpointsRequest &request);
	dap::ResponseOrError<dap::StackTraceResponse> GetStackTrace(const dap::StackTraceRequest &request);
	dap::ResponseOrError<dap::StepInResponse> StepIn(const dap::StepInRequest &request);
	dap::ResponseOrError<dap::StepOutResponse> StepOut(const dap::StepOutRequest &request);
	dap::ResponseOrError<dap::NextResponse> Next(const dap::NextRequest &request);
	dap::ResponseOrError<dap::ScopesResponse> GetScopes(const dap::ScopesRequest &request);
	dap::ResponseOrError<dap::VariablesResponse> GetVariables(const dap::VariablesRequest &request);
	dap::ResponseOrError<dap::SourceResponse> GetSource(const dap::SourceRequest &request);
	dap::ResponseOrError<dap::LoadedSourcesResponse> GetLoadedSources(const dap::LoadedSourcesRequest &request);
	dap::ResponseOrError<dap::DisassembleResponse> Disassemble(const dap::DisassembleRequest &request);
	dap::ResponseOrError<dap::SetExceptionBreakpointsResponse> SetExceptionBreakpoints(const dap::SetExceptionBreakpointsRequest &request);
	dap::ResponseOrError<dap::SetInstructionBreakpointsResponse> SetInstructionBreakpoints(const dap::SetInstructionBreakpointsRequest &request);
	dap::ResponseOrError<dap::ModulesResponse> Modules(const dap::ModulesRequest &request);
	private:
	std::shared_ptr<IdProvider> m_idProvider;

	std::shared_ptr<dap::Session> m_session = nullptr;
	std::shared_ptr<PexCache> m_pexCache;
	std::shared_ptr<BreakpointManager> m_breakpointManager;
	std::shared_ptr<RuntimeState> m_runtimeState;
	std::shared_ptr<DebugExecutionManager> m_executionManager;
	std::map<int, dap::Source> m_projectSources;
	std::string m_projectPath;
	std::string m_projectArchive;
	dap::InitializeRequest m_clientCaps;
	bool m_printLog = false;

	RuntimeEvents::CreateStackEventHandle m_createStackEventHandle;
	RuntimeEvents::CleanupStackEventHandle m_cleanupStackEventHandle;
	RuntimeEvents::InstructionExecutionEventHandle m_instructionExecutionEventHandle;
	RuntimeEvents::LogEventHandle m_logEventHandle;
	RuntimeEvents::BreakpointChangedEventHandle m_breakpointChangedEventHandle;


	std::atomic<bool> m_endingSession = false;
	bool m_quitting = false; // Received a disconnect request with a terminateDebuggee flag; if this is true, we exit the program
	bool m_initialized
		= false; // Received initialize request; If this isn't true, we don't send events, prevents sending events before the client is ready (or if socket has been closed before initialization)
	RuntimeEvents::ExceptionThrownEventHandle m_exceptionThrownEventHandle;

	void RegisterSessionHandlers();
	dap::Error Error(const std::string &msg);
	void EventLogged(int severity, const char *msg);
	void StackCreated(VMFrameStack *stack);
	void StackCleanedUp(uint32_t stackId);
	void InstructionExecution(VMFrameStack *stack, VMReturn *ret, int numret, const VMOP *pc);
	void CheckSourceLoaded(const std::string &scriptName);
	void BreakpointChanged(const dap::Breakpoint &bpoint, const std::string &reason);
	void ExceptionThrown(EVMAbortException reason, const std::string &message, const std::string &stackTrace);
};
}
