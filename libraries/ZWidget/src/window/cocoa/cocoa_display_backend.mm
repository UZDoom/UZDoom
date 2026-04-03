#include "cocoa_display_backend.h"
#include "cocoa_display_window.h"
#include "cocoa_open_file_dialog.h"
#include "cocoa_save_file_dialog.h"
#include "cocoa_open_folder_dialog.h"

#include <zwidget/core/timer.h>

#include "AppKitWrapper.h"

// Helper struct to hold timer callbacks without requiring a Widget owner
struct CocoaTimerData
{
    std::function<void()> callback;
    NSTimer* nstimer;
};

@interface ZWidgetTimerTarget : NSObject
{
    CocoaTimerData* timerData;
}
- (id)initWithTimerData:(CocoaTimerData*)data;
- (void)timerFired:(NSTimer*)nstimer;
@end

@implementation ZWidgetTimerTarget
- (id)initWithTimerData:(CocoaTimerData*)data
{
    self = [super init];
    if (self)
    {
        timerData = data;
    }
    return self;
}

- (void)timerFired:(NSTimer*)nstimer
{
    if (timerData && timerData->callback)
        timerData->callback();
}
@end

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateCocoa()
{
    return std::make_unique<CocoaDisplayBackend>();
}

CocoaDisplayBackend::CocoaDisplayBackend()
{
    // Initialize NSApp if not already done
    [NSApplication sharedApplication];

    // Only configure NSApp if we're the owner (no delegate set yet)
    if ([NSApp delegate] == nil)
    {
        // We're the primary app - set activation policy for keyboard events
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
    }
    // else: Parent app is managing NSApp lifecycle, don't interfere
}

CocoaDisplayBackend::~CocoaDisplayBackend()
{
}

std::unique_ptr<DisplayWindow> CocoaDisplayBackend::Create(DisplayWindowHost* windowHost, WidgetType type, DisplayWindow* owner, RenderAPI renderAPI)
{
    return std::make_unique<CocoaDisplayWindow>(windowHost, type, owner, renderAPI);
}

void CocoaDisplayBackend::ProcessEvents()
{
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
    if (event)
    {
        [NSApp sendEvent:event];
    }
}

void CocoaDisplayBackend::RunLoop()
{
    [NSApp run];
}

void CocoaDisplayBackend::ExitLoop()
{
    [NSApp stop:nil];

    // Post a dummy event to wake up the event loop so stop: takes effect
    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:YES];
}

void* CocoaDisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
    CocoaTimerData* timerData = new CocoaTimerData();
    timerData->callback = onTimer;

    ZWidgetTimerTarget* target = [[ZWidgetTimerTarget alloc] initWithTimerData:timerData];
    NSTimer* nstimer = [NSTimer scheduledTimerWithTimeInterval:timeoutMilliseconds / 1000.0
                                                        target:target
                                                      selector:@selector(timerFired:)
                                                      userInfo:nil
                                                       repeats:YES];
    timerData->nstimer = nstimer;

    return timerData;
}

void CocoaDisplayBackend::StopTimer(void* timerID)
{
    CocoaTimerData* timerData = static_cast<CocoaTimerData*>(timerID);
    if (timerData)
    {
        if (timerData->nstimer)
        {
            [timerData->nstimer invalidate];
        }
        delete timerData;
    }
}

Size CocoaDisplayBackend::GetScreenSize()
{
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    return Size(screenFrame.size.width, screenFrame.size.height);
}

std::unique_ptr<OpenFileDialog> CocoaDisplayBackend::CreateOpenFileDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaOpenFileDialog>(owner);
}

std::unique_ptr<SaveFileDialog> CocoaDisplayBackend::CreateSaveFileDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaSaveFileDialog>(owner);
}

std::unique_ptr<OpenFolderDialog> CocoaDisplayBackend::CreateOpenFolderDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaOpenFolderDialog>(owner);
}
