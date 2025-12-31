/*
** st_console.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2015-2018 Alexey Lysiuk
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
*/

#ifndef COCOA_ST_CONSOLE_INCLUDED
#define COCOA_ST_CONSOLE_INCLUDED

@class NSButton;
@class NSProgressIndicator;
@class NSScrollView;
@class NSTextField;
@class NSTextView;
@class NSView;
@class NSWindow;

struct PalEntry;


class FConsoleWindow
{
public:
	static FConsoleWindow& GetInstance();

	static void CreateInstance();
	static void DeleteInstance();

	void Show(bool visible);
	void ShowFatalError(const char* message);

	void AddText(const char* message);

	void SetTitleText();
	void SetProgressBar(bool visible);

	// FStartupScreen functionality
	void Progress(int current, int maximum);

	void NetInit(const char* const message, const bool host);
	void NetMessage(const char* const message);
	void NetConnect(const int client, const char* const name, const unsigned flags, const int status);
	void NetUpdate(const int client, const int status);
	void NetDisconnect(const int client);
	void NetProgress(const int cur, const int limit);
	void NetDone();
	void NetClose();
	bool ShouldStartNet();
	int GetNetKickClient();
	int GetNetBanClient();

private:
	NSWindow*            m_window;
	NSTextView*          m_textView;
	NSScrollView*        m_scrollView;
	NSProgressIndicator* m_progressBar;

	NSView*              m_netView;
	NSTextField*         m_netMessageText;
	NSTextField*         m_netCountText;
	NSProgressIndicator* m_netProgressBar;
	NSButton*            m_netAbortButton;

	unsigned int         m_characterCount;

	int                  m_netCurPos;
	int                  m_netMaxPos;

	FConsoleWindow();

	void ExpandTextView(float height);

	void AddText(const PalEntry& color, const char* message);

	void ScrollTextToBottom();
};

#endif // COCOA_ST_CONSOLE_INCLUDED
