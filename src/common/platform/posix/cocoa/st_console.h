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
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
