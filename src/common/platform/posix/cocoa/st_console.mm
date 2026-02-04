/*
** st_console.mm
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

#include "i_common.h"
#include "startupinfo.h"
#include "st_console.h"
#include "v_text.h"
#include "version.h"
#include "palentry.h"
#include "v_video.h"
#include "v_font.h"

static NSColor* RGB(const uint8_t red, const uint8_t green, const uint8_t blue)
{
	return [NSColor colorWithCalibratedRed:red   / 255.0f
									 green:green / 255.0f
									  blue:blue  / 255.0f
									 alpha:1.0f];
}

static NSColor* RGB(const PalEntry& color)
{
	return RGB(color.r, color.g, color.b);
}

static NSColor* RGB(const uint32_t color)
{
	return RGB(PalEntry(color));
}


static const CGFloat PROGRESS_BAR_HEIGHT = 18.0f;
static const CGFloat NET_VIEW_HEIGHT     = 88.0f;


FConsoleWindow::FConsoleWindow()
: m_window([NSWindow alloc])
, m_textView([NSTextView alloc])
, m_scrollView([NSScrollView alloc])
, m_progressBar(nil)
, m_netView(nil)
, m_netMessageText(nil)
, m_netCountText(nil)
, m_netProgressBar(nil)
, m_netAbortButton(nil)
, m_characterCount(0)
, m_netCurPos(0)
, m_netMaxPos(0)
{
	const CGFloat initialWidth  = 512.0f;
	const CGFloat initialHeight = 384.0f;
	const NSRect initialRect = NSMakeRect(0.0f, 0.0f, initialWidth, initialHeight);

	[m_textView initWithFrame:initialRect];
	[m_textView setEditable:NO];
	[m_textView setBackgroundColor:RGB(70, 70, 70)];
	[m_textView setMinSize:NSMakeSize(0.0f, initialHeight)];
	[m_textView setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
	[m_textView setVerticallyResizable:YES];
	[m_textView setHorizontallyResizable:NO];
	[m_textView setAutoresizingMask:NSViewWidthSizable];

	NSTextContainer* const textContainer = [m_textView textContainer];
	[textContainer setContainerSize:NSMakeSize(initialWidth, FLT_MAX)];
	[textContainer setWidthTracksTextView:YES];

	[m_scrollView initWithFrame:initialRect];
	[m_scrollView setBorderType:NSNoBorder];
	[m_scrollView setHasVerticalScroller:YES];
	[m_scrollView setHasHorizontalScroller:NO];
	[m_scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[m_scrollView setDocumentView:m_textView];

	NSString* const title = [NSString stringWithFormat:@"%s %s - Console", GAMENAME, GetVersionString()];

	[m_window initWithContentRect:initialRect
						styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
						  backing:NSBackingStoreBuffered
							defer:NO];
	[m_window setMinSize:[m_window frame].size];
	[m_window setShowsResizeIndicator:NO];
	[m_window setTitle:title];
	[m_window center];
	[m_window exitAppOnClose];

	// Do not allow fullscreen mode for this window
	[m_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenAuxiliary];

	[[m_window contentView] addSubview:m_scrollView];

	[m_window makeKeyAndOrderFront:nil];
}


static FConsoleWindow* s_instance;


void FConsoleWindow::CreateInstance()
{
	assert(NULL == s_instance);
	s_instance = new FConsoleWindow;
}

void FConsoleWindow::DeleteInstance()
{
	assert(NULL != s_instance);
	delete s_instance;
	s_instance = NULL;
}

FConsoleWindow& FConsoleWindow::GetInstance()
{
	assert(NULL != s_instance);
	return *s_instance;
}


void FConsoleWindow::Show(const bool visible)
{
	if (visible)
	{
		[m_window orderFront:nil];
	}
	else
	{
		[m_window orderOut:nil];
	}
}

void FConsoleWindow::ShowFatalError(const char* const message)
{
	SetProgressBar(false);
	//NetDone();

	const CGFloat textViewWidth = [m_scrollView frame].size.width;

	ExpandTextView(-32.0f);

	NSButton* quitButton = [[NSButton alloc] initWithFrame:NSMakeRect(textViewWidth - 76.0f, 0.0f, 72.0f, 30.0f)];
	[quitButton setAutoresizingMask:NSViewMinXMargin];
	[quitButton setBezelStyle:NSRoundedBezelStyle];
	[quitButton setTitle:@"Quit"];
	[quitButton setKeyEquivalent:@"\r"];
	[quitButton setTarget:NSApp];
	[quitButton setAction:@selector(stopModal)];

	NSView* quitPanel = [[NSView alloc] initWithFrame:NSMakeRect(0.0f, 0.0f, textViewWidth, 32.0f)];
	[quitPanel setAutoresizingMask:NSViewWidthSizable];
	[quitPanel addSubview:quitButton];

	[[m_window contentView] addSubview:quitPanel];
	[m_window orderFront:nil];

	AddText(PalEntry(255,   0,   0), "\nExecution could not continue.\n");
	AddText(PalEntry(255, 255, 170), message);
	AddText("\n");

	ScrollTextToBottom();

	[NSApp runModalForWindow:m_window];
}


static const unsigned int THIRTY_FPS = 33; // milliseconds per update


template <typename Function, unsigned int interval = THIRTY_FPS>
struct TimedUpdater
{
	explicit TimedUpdater(const Function& function)
	{
		extern uint64_t I_msTime();
		const unsigned int currentTime = I_msTime();

		if (currentTime - m_previousTime > interval)
		{
			m_previousTime = currentTime;

			function();

			[[NSRunLoop currentRunLoop] limitDateForMode:NSDefaultRunLoopMode];
		}
	}

	static unsigned int m_previousTime;
};

template <typename Function, unsigned int interval>
unsigned int TimedUpdater<Function, interval>::m_previousTime;

template <typename Function, unsigned int interval = THIRTY_FPS>
static void UpdateTimed(const Function& function)
{
	TimedUpdater<Function, interval> dummy(function);
}


void FConsoleWindow::AddText(const char* message)
{
	PalEntry color(223, 223, 223);

	char buffer[1024] = {};
	size_t pos = 0;
	bool reset = false;

	while (*message != '\0')
	{
		if ((TEXTCOLOR_ESCAPE == *message && 0 != pos)
			|| (pos == sizeof buffer - 1)
			|| reset)
		{
			buffer[pos] = '\0';
			pos = 0;
			reset = false;

			AddText(color, buffer);
		}

		if (TEXTCOLOR_ESCAPE == *message)
		{
			const uint8_t* colorID = reinterpret_cast<const uint8_t*>(message) + 1;
			if ('\0' == *colorID)
			{
				break;
			}

			const EColorRange range = V_ParseFontColor(colorID, CR_UNTRANSLATED, CR_YELLOW);

			if (range != CR_UNDEFINED)
			{
				color = V_LogColorFromColorRange(range);
			}

			message += 2;
		}
		else if (0x1d == *message || 0x1f == *message) // Opening and closing bar characters
		{
			buffer[pos++] = '-';
			++message;
		}
		else if (0x1e == *message) // Middle bar character
		{
			buffer[pos++] = '=';
			++message;
		}
		else
		{
			buffer[pos++] = *message++;
		}
	}

	if (0 != pos)
	{
		buffer[pos] = '\0';

		AddText(color, buffer);
	}

	if ([m_window isVisible])
	{
		UpdateTimed([&]()
		{
			[m_textView scrollRangeToVisible:NSMakeRange(m_characterCount, 0)];
		});
	}
}

void FConsoleWindow::AddText(const PalEntry& color, const char* const message)
{
	NSString* const text = [NSString stringWithCString:message
											  encoding:NSISOLatin1StringEncoding];

	NSDictionary* const attributes = [NSDictionary dictionaryWithObjectsAndKeys:
									  [NSFont systemFontOfSize:14.0f], NSFontAttributeName,
									  RGB(color), NSForegroundColorAttributeName,
									  nil];

	NSAttributedString* const formattedText =
	[[NSAttributedString alloc] initWithString:text
									attributes:attributes];
	[[m_textView textStorage] appendAttributedString:formattedText];

	m_characterCount += [text length];
}


void FConsoleWindow::ScrollTextToBottom()
{
	[m_textView scrollRangeToVisible:NSMakeRange(m_characterCount, 0)];

	[[NSRunLoop currentRunLoop] limitDateForMode:NSDefaultRunLoopMode];
}


void FConsoleWindow::SetTitleText()
{
	static const CGFloat TITLE_TEXT_HEIGHT = 32.0f;

	NSRect textViewFrame = [m_scrollView frame];
	textViewFrame.size.height -= TITLE_TEXT_HEIGHT;
	[m_scrollView setFrame:textViewFrame];

	const NSRect titleTextRect = NSMakeRect(
		0.0f,
		textViewFrame.origin.y + textViewFrame.size.height,
		textViewFrame.size.width,
		TITLE_TEXT_HEIGHT);

	// Temporary solution for the same foreground and background colors
	// It's used in graphical startup screen, with Hexen style in particular
	// Native OS X backend doesn't implement this yet

	if (GameStartupInfo.FgColor == GameStartupInfo.BkColor)
	{
		GameStartupInfo.FgColor = ~GameStartupInfo.FgColor;
	}

	NSTextField* titleText = [[NSTextField alloc] initWithFrame:titleTextRect];
	[titleText setStringValue:[NSString stringWithCString:GameStartupInfo.Name.GetChars()
												 encoding:NSISOLatin1StringEncoding]];
	[titleText setAlignment:NSTextAlignmentCenter];
	[titleText setTextColor:RGB(GameStartupInfo.FgColor)];
	[titleText setBackgroundColor:RGB(GameStartupInfo.BkColor)];
	[titleText setFont:[NSFont fontWithName:@"Trebuchet MS Bold" size:18.0f]];
	[titleText setAutoresizingMask:NSViewWidthSizable | NSViewMinYMargin];
	[titleText setSelectable:NO];
	[titleText setBordered:NO];

	[[m_window contentView] addSubview:titleText];
}

void FConsoleWindow::SetProgressBar(const bool visible)
{
	if (  (!visible && nil == m_progressBar)
		|| (visible && nil != m_progressBar))
	{
		return;
	}

	if (visible)
	{
		ExpandTextView(-PROGRESS_BAR_HEIGHT);

		static const CGFloat PROGRESS_BAR_X = 2.0f;
		const NSRect PROGRESS_BAR_RECT = NSMakeRect(
			PROGRESS_BAR_X, 0.0f,
			[m_window frame].size.width - PROGRESS_BAR_X * 2, 16.0f);

		m_progressBar = [[NSProgressIndicator alloc] initWithFrame:PROGRESS_BAR_RECT];
		[m_progressBar setIndeterminate:NO];
		[m_progressBar setAutoresizingMask:NSViewWidthSizable];

		[[m_window contentView] addSubview:m_progressBar];
	}
	else
	{
		ExpandTextView(PROGRESS_BAR_HEIGHT);

		[m_progressBar removeFromSuperview];
		[m_progressBar release];
		m_progressBar = nil;
	}
}


void FConsoleWindow::ExpandTextView(const float height)
{
	NSRect textFrame = [m_scrollView frame];
	textFrame.origin.y    -= height;
	textFrame.size.height += height;
	[m_scrollView setFrame:textFrame];
}


void FConsoleWindow::Progress(const int current, const int maximum)
{
	if (nil == m_progressBar)
	{
		return;
	}

	UpdateTimed([&]()
	{
		[m_progressBar setMaxValue:maximum];
		[m_progressBar setDoubleValue:current];
	});
}
