#pragma once

#include <unordered_map>
#include <zwidget/window/window.h>
#include <zwidget/window/sdlnativehandle.h>
#include <SDL3/SDL.h>

#include "zwidget/core/image.h"

class SDL3DisplayWindow : public DisplayWindow
{
public:
	SDL3DisplayWindow(DisplayWindowHost* windowHost, WidgetType type, SDL3DisplayWindow* owner, RenderAPI renderAPI, double uiscale);
	~SDL3DisplayWindow();

	void SetWindowTitle(const std::string& text) override;
	void SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images) override;
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

	Rect GetClientFrame() const override;
	Size GetClientSize() const override;
	int GetPixelWidth() const override;
	int GetPixelHeight() const override;
	double GetDpiScale() const override;

	void PresentBitmap(int width, int height, const uint32_t* pixels) override;

	void SetBorderColor(uint32_t bgra8) override;
	void SetCaptionColor(uint32_t bgra8) override;
	void SetCaptionTextColor(uint32_t bgra8) override;

	std::string GetClipboardText() override;
	void SetClipboardText(const std::string& text) override;

	Point MapFromGlobal(const Point& pos) const override;
	Point MapToGlobal(const Point& pos) const override;

	void* GetNativeHandle() override { return &Handle; }

	std::vector<std::string> GetVulkanInstanceExtensions() override;
	VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;

	static void DispatchEvent(const SDL_Event& event);
	static SDL3DisplayWindow* FindEventWindow(const SDL_Event& event);

	void OnWindowEvent(const SDL_WindowEvent& event);
	void OnTextInput(const SDL_TextInputEvent& event);
	void OnKeyUp(const SDL_KeyboardEvent& event);
	void OnKeyDown(const SDL_KeyboardEvent& event);
	void OnMouseButtonUp(const SDL_MouseButtonEvent& event);
	void OnMouseButtonDown(const SDL_MouseButtonEvent& event);
	void OnMouseWheel(const SDL_MouseWheelEvent& event);
	void OnMouseMotion(const SDL_MouseMotionEvent& event);
	void OnJoyButtonUp(const SDL_GamepadButtonEvent& event);
	void OnJoyButtonDown(const SDL_GamepadButtonEvent& event);
	void OnPaintEvent();
	static void OnTimerEvent(const SDL_UserEvent& event);

	InputKey GetMouseButtonKey(const SDL_MouseButtonEvent& event);

	static InputKey ScancodeToInputKey(SDL_Scancode keycode);
	static InputKey GameControllerButtonToInputKey(SDL_GamepadButton button);
	static SDL_Scancode InputKeyToScancode(InputKey inputkey);

	static SDL_PixelFormat ImageFormatToSDLPixelFormat(const ImageFormat& format);

	template<typename T>
	Point GetMousePos(const T& event)
	{
		double uiscale = GetDpiScale();
		return Point(event.x / uiscale, event.y / uiscale);
	}

	static void ProcessEvents();
	static void RunLoop();
	static void ExitLoop();

	static void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer);
	static void StopTimer(void* timerID);
	static Uint32 ExecTimer(void* id, SDL_TimerID timerID, Uint32 interval);

	DisplayWindowHost* WindowHost = nullptr;
	SDLNativeHandle Handle;
	SDL_Renderer* RendererHandle = nullptr;
	SDL_Texture* BackBufferTexture = nullptr;
	int BackBufferWidth = 0;
	int BackBufferHeight = 0;

	SDL_Surface* windowIconSurface = nullptr;

	double UIScale = 1.0;

	bool CursorLocked = false;
	bool isFullscreen = false;

	static bool ExitRunLoop;
	static Uint32 PaintEventNumber;
	static std::unordered_map<unsigned int, SDL3DisplayWindow*> WindowList;

	static std::unordered_map<SDL_TimerID, void *> TimerHandles;
	static std::unordered_map<void *, std::function<void()>> Timers;
	static unsigned long TimerIDs;
	static Uint32 TimerEventNumber;
};
