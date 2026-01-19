#pragma once

#include "zwidget/window/window.h"

#include <memory>
#include <string>
#include <vector>

struct CocoaDisplayWindowImpl;

class CocoaDisplayWindow : public DisplayWindow
{
public:
    CocoaDisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI);
    ~CocoaDisplayWindow();

    void SetWindowTitle(const std::string& text) override;
    void SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images) override;
    void SetWindowFrame(const Rect& box) override;
    void SetClientFrame(const Rect& box) override;

    void Show() override;
    void ShowFullscreen() override;
    void ShowMaximized() override;
    void ShowMinimized() override;
    void ShowNormal() override;
    bool IsWindowFullscreen() override;
    void Hide() override;
    void Activate() override;

    void ShowCursor(bool enable) override;
    void LockKeyboard() override;
    void UnlockKeyboard() override;
    void LockCursor() override;
    void UnlockCursor() override;
    void CaptureMouse() override;
    void ReleaseMouseCapture() override;

    void Update() override;

    bool GetKeyState(InputKey key) override;

    void SetCursor(StandardCursor cursor, std::shared_ptr<CustomCursor> custom) override;

    Rect GetWindowFrame() const override;
    Size GetClientSize() const override;
    int GetPixelWidth() const override;
    int GetPixelHeight() const override;
    double GetDpiScale() const override;

    Point MapFromGlobal(const Point& pos) const override;
    Point MapToGlobal(const Point& pos) const override;

    void SetBorderColor(uint32_t bgra8) override;
    void SetCaptionColor(uint32_t bgra8) override;
    void SetCaptionTextColor(uint32_t bgra8) override;

    void PresentBitmap(int width, int height, const uint32_t* pixels) override;

    std::string GetClipboardText() override;
    void SetClipboardText(const std::string& text) override;

    void* GetNativeHandle() override;

    std::vector<std::string> GetVulkanInstanceExtensions() override;
    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;

    // Metal API access for 3D rendering integration
    void* GetMetalDevice();
    void* GetMetalLayer();

#ifdef HAVE_OPENGL
    // Placeholder for OpenGL-specific methods
    // virtual void* GetOpenGLContext() = 0;
#endif

private:
    std::unique_ptr<CocoaDisplayWindowImpl> impl;
};