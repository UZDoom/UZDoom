#pragma once

#include "zwidget/window/window.h"

class CocoaDisplayBackend : public DisplayBackend
{
public:
    CocoaDisplayBackend();
    ~CocoaDisplayBackend();

    std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI) override;

    void ProcessEvents() override;
    void RunLoop() override;
    void ExitLoop() override;
    
    bool IsCocoa() override { return true; }

    void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
    void StopTimer(void* timerID) override;

    Size GetScreenSize() override;

    std::unique_ptr<OpenFileDialog> CreateOpenFileDialog(DisplayWindow* owner) override;
    std::unique_ptr<SaveFileDialog> CreateSaveFileDialog(DisplayWindow* owner) override;
    std::unique_ptr<OpenFolderDialog> CreateOpenFolderDialog(DisplayWindow* owner) override;

    static std::unique_ptr<DisplayBackend> TryCreateCocoa();
};