/*
** st_start.h
**
** Interface for the startup screen.
**
**---------------------------------------------------------------------------
**
** Copyright 2006-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
** The startup screen interface is based on a mix of Heretic and Hexen.
** Actual implementation is system-specific.
*/

#pragma once

#include <stdint.h>

class FStartupScreen
{
public:
	static FStartupScreen *CreateInstance(int max_progress);

	FStartupScreen(int max_progress)
	{
		MaxPos = max_progress;
		CurPos = 0;
		NotchPos = 0;
	}

	virtual ~FStartupScreen() = default;

	virtual void Progress(int advance = 1) {}
	virtual void AppendStatusLine(const char* status) {}
	virtual void LoadingStatus(const char* message, int colors) {}

	virtual void NetInit(const char* message, bool host) {}
	virtual void NetMessage(const char* message) {}
	virtual void NetConnect(int client, const char* name, unsigned flags, int status) {}
	virtual void NetUpdate(int client, int status) {}
	virtual void NetDisconnect(int client) {}
	virtual void NetProgress(int cur, int limit) {}
	virtual void NetDone() {}
	virtual void NetClose() {}
	virtual bool ShouldStartNet() { return false; }
	virtual int GetNetKickClient() { return -1; }
	virtual int GetNetBanClient() { return -1; }
	virtual bool NetLoop(bool (*loopCallback)(void *), void *data) { return false; }

protected:
	int MaxPos, CurPos, NotchPos;
};

class FBasicStartupScreen : public FStartupScreen
{
public:
	FBasicStartupScreen(int max_progress);
	~FBasicStartupScreen();

	void Progress(int advance = 1) override;

	void NetInit(const char* message, bool host) override;
	void NetMessage(const char* message) override;
	void NetConnect(int client, const char* name, unsigned flags, int status) override;
	void NetUpdate(int client, int status) override;
	void NetDisconnect(int client) override;
	void NetProgress(int cur, int limit) override;
	void NetDone() override;
	void NetClose() override;
	bool ShouldStartNet() override;
	int GetNetKickClient() override;
	int GetNetBanClient() override;
	bool NetLoop(bool (*loopCallback)(void*), void* data) override;

protected:
	int NetMaxPos, NetCurPos;
};



extern FStartupScreen *StartWindow;

//===========================================================================
//
// DeleteStartupScreen
//
// Makes sure the startup screen has been deleted before quitting.
//
//===========================================================================

inline void DeleteStartupScreen()
{
	if (StartWindow != nullptr)
	{
		delete StartWindow;
		StartWindow = nullptr;
	}
}


