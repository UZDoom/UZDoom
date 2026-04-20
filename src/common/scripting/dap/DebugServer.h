/*
** DebugServer.h
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

#include <condition_variable>
#include <thread>
#include <mutex>

namespace dap
{
namespace net
{
	class Server;
}
class ReaderWriter;
}
namespace DebugServer
{
class ZScriptDebugger;
class DebugServer
{
public:
	DebugServer();
	~DebugServer();

	void RunRestartThread();
	bool Listen(int port);
	void Stop();

private:
	void onClientConnected(const std::shared_ptr<dap::ReaderWriter> &connection);
	void onError(const char * msg);
	bool StartServer();

	using ResetThreadLock = std::unique_lock<std::mutex>;
	std::unique_ptr<ZScriptDebugger> debugger;
	std::unique_ptr<dap::net::Server> m_server;
	std::condition_variable cv;
	std::mutex mutex; // guards 'terminate'
	int port;
	bool stopped = false;
	bool terminate = false;
	bool restart_server = false;
	bool closed = false;
	bool quitting = false; // On receiving a disconnect request with a terminateDebuggee flag
	std::thread restart_thread;
};
}
