/*
** netstartwindow.h
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <exception>
#include <string>
#include <vector>

class NetStartWindow
{
  public:
	static void NetInit(const char *message, bool host);
	static void NetMessage(const char *message);
	static void NetConnect(int client, const char *name, unsigned flags, int status);
	static void NetUpdate(int client, int status);
	static void NetDisconnect(int client);
	static void NetProgress(int cur, int limit);
	static void NetDone();
	static void NetClose();
	static bool ShouldStartNet();
	static int  GetNetKickClient();
	static int  GetNetBanClient();
	static bool NetLoop(bool (*timer_callback)(void *), void *userdata);
};
