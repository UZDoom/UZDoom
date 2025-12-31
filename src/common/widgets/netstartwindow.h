/*
** netstartwindow.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
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
*/

#pragma once

#include <zwidget/core/widget.h>
#include <stdexcept>

class TextLabel;
class ListView;
class PushButton;
class Timer;

class NetStartWindow : public Widget
{
public:
	static void NetInit(const char* message, bool host);
	static void NetMessage(const char* message);
	static void NetConnect(int client, const char* name, unsigned flags, int status);
	static void NetUpdate(int client, int status);
	static void NetDisconnect(int client);
	static void NetProgress(int cur, int limit);
	static void NetDone();
	static void NetClose();
	static bool ShouldStartNet();
	static int GetNetKickClient();
	static int GetNetBanClient();
	static bool NetLoop(bool (*timer_callback)(void*), void* userdata);

private:
	NetStartWindow(bool host);

	void SetMessage(const std::string& message);
	void SetProgress(int pos);

protected:
	void OnClose() override;
	void OnGeometryChanged() override;
	virtual void ForceStart();
	virtual void OnKick();
	virtual void OnBan();

private:
	void OnCallbackTimerExpired();

	TextLabel* MessageLabel = nullptr;
	TextLabel* ProgressLabel = nullptr;
	ListView* LobbyWindow = nullptr;
	PushButton* AbortButton = nullptr;
	PushButton* ForceStartButton = nullptr;
	PushButton* KickButton = nullptr;
	PushButton* BanButton = nullptr;

	Timer* CallbackTimer = nullptr;

	int pos = 0;
	int maxpos = 1;
	bool (*timer_callback)(void*) = nullptr;
	void* userdata = nullptr;

	bool exitreason = false;
	bool shouldstart = false;
	bool hosting = false;
	std::vector<int> kickclients;
	std::vector<int> banclients;

	std::exception_ptr CallbackException;

	static NetStartWindow* Instance;
};
