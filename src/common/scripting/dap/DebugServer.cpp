/*
** DebugServer.cpp
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

#include "DebugServer.h"
#include <thread>
#include <functional>
#include <dap/network.h>
#include "ZScriptDebugger.h"

// Main entry point for the debug server

namespace DebugServer
{
DebugServer::DebugServer()
{
	terminate = false;
	stopped = false;
	quitting = false;
	debugger = std::unique_ptr<ZScriptDebugger>(new ZScriptDebugger());
}

void DebugServer::RunRestartThread()
{
	while (true)
	{
		ResetThreadLock lock(mutex);
		cv.wait(lock, [&] { return terminate; });
		quitting = debugger->EndSession(closed);
		terminate = false;
		if (stopped || quitting)
		{
			break;
		}
		if (restart_server){
			m_server->stop();
			m_server = nullptr;
			m_server = dap::net::Server::create();
			if (!StartServer()) {
				RuntimeEvents::UnsubscribeFromDebuggerEnabled([](){return true;});
				stopped = true;
				break;
			}
			restart_server = false;
		}
	}
	if (quitting)
	{
		throw CExitEvent(0);
	}

}

bool DebugServer::Listen(int p_port)
{
	terminate = false;
	stopped = false;
	quitting = false;
	closed = false;
	if (!p_port)
	{
		return false;
	}
	port = p_port;
	if (!m_server)
	{
		m_server = dap::net::Server::create();
	}
	else
	{
		Stop();
	}
	restart_thread = std::thread(std::bind(&DebugServer::RunRestartThread, this));
	return StartServer();
}

void DebugServer::onClientConnected(const std::shared_ptr<dap::ReaderWriter> &connection)
{
		if (!connection || !connection->isOpen())
		{
			LogInternalError("Client connected but connection is not open");
			return;
		}
		closed = false;
		std::shared_ptr<dap::Session> sess;
		sess = dap::Session::create();
		sess->bind(
			connection,
			[this]()
			{
				LogInternal("DAP connection closed.");
				// try_lock here because this can be called when the reset thread is ending the session
				ResetThreadLock lock(mutex, std::defer_lock);
				if (lock.try_lock() && !terminate && !debugger->IsEndingSession())
				{
					LogInternalError("DAP connection closed without terminating session.");
					closed = true;
					terminate = true;
					cv.notify_all();
				}
			});
		// Registering a handle for when we send the DisconnectResponse;
		// After we send the disconnect response, the restart thread will stop the session and restart the server.
		sess->registerSentHandler(
			[this](const dap::ResponseOrError<dap::DisconnectResponse> &)
			{
				LogInternal("Debugger disconnecting...");
				ResetThreadLock lock(mutex);
				terminate = true;
				cv.notify_all();
			});

		LogInternal("DAP client connected.");
		debugger->StartSession(sess);
};

void DebugServer::onError(const char *msg)
{
	LogInternalError("Server error: %s\n", msg);
	if (restart_server){
		LogInternalError("Restart failed! Stopping server...");
	} else {
		LogInternalError("Restarting server...");
		{
			ResetThreadLock lock(mutex);
			terminate = true;
			closed = true;
			restart_server = true;
			cv.notify_all();
		}
	}
};

bool DebugServer::StartServer()
{
	if (!m_server->start(port, [this](auto conn){this->onClientConnected(conn);}, [this](auto msg){this->onError(msg);}))
	{
		LogInternalError("DAP debugging server failed to start on port %d, debugging will be unavailable!", port);
		return false;
	}
	RuntimeEvents::SubscribeToDebuggerEnabled([](){return true;});
	LogInternal("DAP debugging server started on port %d", port);
	return true;
}

void DebugServer::Stop()
{
	RuntimeEvents::UnsubscribeFromDebuggerEnabled([](){return true;});
	{
		ResetThreadLock lock(mutex);
		terminate = true;
		stopped = true;
		cv.notify_all();
	}
	if (m_server) {
		m_server->stop();
	}
	if (restart_thread.joinable())
	{
		restart_thread.join();
	}
}

DebugServer::~DebugServer() { Stop(); }
}
