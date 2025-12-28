/*
** DebugServer.h
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
