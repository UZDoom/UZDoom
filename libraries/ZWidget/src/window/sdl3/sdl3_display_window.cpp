#include "sdl3_display_window.h"
#include <stdexcept>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <cmath>

#include "zwidget/core/image.h"

Uint32 SDL3DisplayWindow::PaintEventNumber = 0xffffffff;
bool SDL3DisplayWindow::ExitRunLoop;
std::unordered_map<unsigned int, SDL3DisplayWindow*> SDL3DisplayWindow::WindowList;

SDL3DisplayWindow::SDL3DisplayWindow(DisplayWindowHost* windowHost, WidgetType type, SDL3DisplayWindow* owner, RenderAPI renderAPI, double uiscale) : WindowHost(windowHost), UIScale(uiscale)
{
	unsigned int flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (renderAPI == RenderAPI::Vulkan)
		flags |= SDL_WINDOW_VULKAN;
	else if (renderAPI == RenderAPI::OpenGL)
		flags |= SDL_WINDOW_OPENGL;
#if defined(__APPLE__)
	else if (renderAPI == RenderAPI::Metal)
		flags |= SDL_WINDOW_METAL;
#endif
	if (type == WidgetType::Popup)
		flags |= SDL_WINDOW_BORDERLESS;

	if (renderAPI == RenderAPI::Vulkan || renderAPI == RenderAPI::OpenGL || renderAPI == RenderAPI::Metal)
	{
		Handle.window = SDL_CreateWindow("", 320, 200, flags);
		if (!Handle.window)
			throw std::runtime_error(std::string("Unable to create SDL window:") + SDL_GetError());
	}
	else
	{
		if (!SDL_CreateWindowAndRenderer("", 320, 200, flags, &Handle.window, &RendererHandle))
			throw std::runtime_error(std::string("Unable to create SDL window:") + SDL_GetError());
	}

	SDL_StartTextInput(Handle.window);

	WindowList[SDL_GetWindowID(Handle.window)] = this;
}

SDL3DisplayWindow::~SDL3DisplayWindow()
{
	SDL3DisplayWindow::UnlockCursor();

	SDL_StopTextInput(Handle.window);

	WindowList.erase(WindowList.find(SDL_GetWindowID(Handle.window)));

	if (BackBufferTexture)
	{
		SDL_DestroyTexture(BackBufferTexture);
		BackBufferTexture = nullptr;
	}

	if (RendererHandle)
		SDL_DestroyRenderer(RendererHandle);
	SDL_DestroyWindow(Handle.window);
	RendererHandle = nullptr;
	Handle.window = nullptr;
}

std::vector<std::string> SDL3DisplayWindow::GetVulkanInstanceExtensions()
{
	Uint32 extCount = 0;
	char const* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);

	std::vector<std::string> result;

	for (Uint32 i = 0 ; i < extCount ; i++)
		result.emplace_back(std::string(extNames[i]));

	return result;
}

VkSurfaceKHR SDL3DisplayWindow::CreateVulkanSurface(VkInstance instance)
{
	VkSurfaceKHR surfaceHandle = {};
	if (!SDL_Vulkan_CreateSurface(Handle.window, instance, nullptr, &surfaceHandle))
		throw std::runtime_error("Could not create vulkan surface");
	return surfaceHandle;
}

void SDL3DisplayWindow::SetWindowTitle(const std::string& text)
{
	SDL_SetWindowTitle(Handle.window, text.c_str());
}

SDL_PixelFormat SDL3DisplayWindow::ImageFormatToSDLPixelFormat(const ImageFormat& format)
{
	switch (format)
	{
	case ImageFormat::B8G8R8A8:
		return SDL_PIXELFORMAT_BGRA8888;
	case ImageFormat::R8G8B8A8:
		return SDL_PIXELFORMAT_RGBA8888;
	}

	return SDL_PIXELFORMAT_UNKNOWN;
}

void SDL3DisplayWindow::SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images)
{
	const auto imageFormat = ImageFormatToSDLPixelFormat(images[0]->GetFormat());
	SDL_Surface* iconSurface = SDL_CreateSurface(images[0]->GetWidth(), images[0]->GetHeight(), imageFormat);

	// Add the remaining images
	for (size_t i = 1 ; i < images.size() ; i++)
	{
		SDL_Surface* altImageSurface = SDL_CreateSurface(images[i]->GetWidth(), images[i]->GetHeight(), imageFormat);
		if (!SDL_AddSurfaceAlternateImage(iconSurface, altImageSurface))
			throw std::runtime_error(std::string("Error on SDL_AddSurfaceAlternateImage(): ") + SDL_GetError());
		SDL_DestroySurface(altImageSurface); // SDL recommends calling this after AddSurfaceAlternateImage() is successful.
	}

	SDL_SetWindowIcon(Handle.window, iconSurface);
}

void SDL3DisplayWindow::SetClientFrame(const Rect& box)
{
	// Is there a way to set both in one call?

	double uiscale = GetDpiScale();
	int x = (int)std::round(box.x * uiscale);
	int y = (int)std::round(box.y * uiscale);
	int w = (int)std::round(box.width * uiscale);
	int h = (int)std::round(box.height * uiscale);

	// SDL_SetWindowSize sets the client size of the window
	SDL_SetWindowSize(Handle.window, w, h);
	SDL_SetWindowPosition(Handle.window, x, y);
}

void SDL3DisplayWindow::Show()
{
	SDL_ShowWindow(Handle.window);
}

void SDL3DisplayWindow::ShowFullscreen()
{
	SDL_ShowWindow(Handle.window);
	SDL_SetWindowFullscreen(Handle.window, true);
	isFullscreen = true;
}

void SDL3DisplayWindow::ShowMaximized()
{
	SDL_ShowWindow(Handle.window);
	SDL_MaximizeWindow(Handle.window);
}

void SDL3DisplayWindow::ShowMinimized()
{
	SDL_ShowWindow(Handle.window);
	SDL_MinimizeWindow(Handle.window);
}

void SDL3DisplayWindow::ShowNormal()
{
	SDL_ShowWindow(Handle.window);
	SDL_SetWindowFullscreen(Handle.window, false);
	isFullscreen = false;
}

bool SDL3DisplayWindow::IsWindowFullscreen()
{
	return isFullscreen;
}

void SDL3DisplayWindow::Hide()
{
	SDL_HideWindow(Handle.window);
}

void SDL3DisplayWindow::Activate()
{
	SDL_RaiseWindow(Handle.window);
}

void SDL3DisplayWindow::ShowCursor(bool enable)
{
	if (enable)
		SDL_ShowCursor();
	else
		SDL_HideCursor();
}

void SDL3DisplayWindow::LockKeyboard()
{
	// Enables raw keyboard scancode events (OnRawKeyboard should be called for keyboard input)
}

void SDL3DisplayWindow::UnlockKeyboard()
{
	// Disable raw keyboard scancode events (OnKeyDown/OnKeyUp/OnKeyChar should be called for keyboard input)
}

void SDL3DisplayWindow::LockCursor()
{
	SDL_CaptureMouse(true);
	CursorLocked = true;
}

void SDL3DisplayWindow::UnlockCursor()
{
	SDL_CaptureMouse(false);
	CursorLocked = false;
}

void SDL3DisplayWindow::CaptureMouse()
{
	SDL_CaptureMouse(true);
}

void SDL3DisplayWindow::ReleaseMouseCapture()
{
	SDL_CaptureMouse(false);
}

void SDL3DisplayWindow::SetCursor(StandardCursor cursor, std::shared_ptr<CustomCursor> custom)
{
}

void SDL3DisplayWindow::Update()
{
	SDL_Event event = {};
	event.type = PaintEventNumber;
	event.user.windowID = SDL_GetWindowID(Handle.window);
	SDL_PushEvent(&event);
}

bool SDL3DisplayWindow::GetKeyState(InputKey key)
{
	int numkeys = 0;
	const bool* state = SDL_GetKeyboardState(&numkeys);
	if (!state) return false;

	const SDL_Scancode index = InputKeyToScancode(key);
	return (index < numkeys) ? state[index] != 0 : false;
}

Rect SDL3DisplayWindow::GetClientFrame() const
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	double uiscale = GetDpiScale();
	SDL_GetWindowPosition(Handle.window, &x, &y);
	SDL_GetWindowSize(Handle.window, &w, &h);
	return Rect::xywh(x / uiscale, y / uiscale, w / uiscale, h / uiscale);
}

Point SDL3DisplayWindow::MapFromGlobal(const Point& pos) const
{
	int x = 0;
	int y = 0;
	double uiscale = GetDpiScale();
	SDL_GetWindowPosition(Handle.window, &x, &y);
	return Point(pos.x - x / uiscale, pos.y - y / uiscale);
}

Point SDL3DisplayWindow::MapToGlobal(const Point& pos) const
{
	int x = 0;
	int y = 0;
	double uiscale = GetDpiScale();
	SDL_GetWindowPosition(Handle.window, &x, &y);
	return Point(pos.x + x / uiscale, pos.y + y / uiscale);
}

Size SDL3DisplayWindow::GetClientSize() const
{
	int w = 0;
	int h = 0;
	double uiscale = GetDpiScale();
	SDL_GetWindowSize(Handle.window, &w, &h);
	return Size(w / uiscale, h / uiscale);
}

int SDL3DisplayWindow::GetPixelWidth() const
{
	int w = 0;
	int h = 0;
	if (RendererHandle)
		SDL_GetCurrentRenderOutputSize(RendererHandle, &w, &h);
	else
		SDL_GetWindowSizeInPixels(Handle.window, &w, &h);
	return w;
}

int SDL3DisplayWindow::GetPixelHeight() const
{
	int w = 0;
	int h = 0;
	if (RendererHandle)
		SDL_GetCurrentRenderOutputSize(RendererHandle, &w, &h);
	else
		SDL_GetWindowSizeInPixels(Handle.window, &w, &h);
	return h;
}

double SDL3DisplayWindow::GetDpiScale() const
{
	return SDL_GetWindowDisplayScale(Handle.window);
}

void SDL3DisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
	if (!RendererHandle)
		return;

	if (!BackBufferTexture || BackBufferWidth != width || BackBufferHeight != height)
	{
		if (BackBufferTexture)
		{
			SDL_DestroyTexture(BackBufferTexture);
			BackBufferTexture = nullptr;
		}

		BackBufferTexture = SDL_CreateTexture(RendererHandle, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		if (!BackBufferTexture)
			return;

		BackBufferWidth = width;
		BackBufferHeight = height;
	}

	int destpitch = 0;
	void* dest = nullptr;
	if (!SDL_LockTexture(BackBufferTexture, nullptr, &dest, &destpitch))
		return;
	for (int y = 0; y < height; y++)
	{
		const void* sline = pixels + y * width;
		void* dline = (uint8_t*)dest + y * destpitch;
		memcpy(dline, sline, width << 2);
	}
	SDL_UnlockTexture(BackBufferTexture);

	SDL_RenderTexture(RendererHandle, BackBufferTexture, nullptr, nullptr);
	SDL_RenderPresent(RendererHandle);
}

void SDL3DisplayWindow::SetBorderColor(uint32_t bgra8)
{
	// SDL doesn't have this
}

void SDL3DisplayWindow::SetCaptionColor(uint32_t bgra8)
{
	// SDL doesn't have this
}

void SDL3DisplayWindow::SetCaptionTextColor(uint32_t bgra8)
{
	// SDL doesn't have this
}

std::string SDL3DisplayWindow::GetClipboardText()
{
	char* buffer = SDL_GetClipboardText();
	if (!buffer)
		return {};
	std::string text = buffer;
	SDL_free(buffer);
	return text;
}

void SDL3DisplayWindow::SetClipboardText(const std::string& text)
{
	SDL_SetClipboardText(text.c_str());
}

void SDL3DisplayWindow::ProcessEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		DispatchEvent(event);
	}
}

void SDL3DisplayWindow::RunLoop()
{
	ExitRunLoop = false;
	while (!ExitRunLoop)
	{
		SDL_Event event = {};
		if (SDL_WaitEvent(&event))
			DispatchEvent(event); // Silently ignore if it fails and pray it doesn't busy loop, because SDL and Linux utterly sucks!
	}
}

void SDL3DisplayWindow::ExitLoop()
{
	ExitRunLoop = true;
}

std::unordered_map<void *, std::function<void()>> SDL3DisplayWindow::Timers;
std::unordered_map<SDL_TimerID, void *> SDL3DisplayWindow::TimerHandles;
unsigned long SDL3DisplayWindow::TimerIDs = 0;
Uint32 TimerEventID = SDL_RegisterEvents(1);

Uint32 SDL3DisplayWindow::ExecTimer(void* execID, SDL_TimerID timerID, Uint32 interval)
{
	// cancel event and stop loop if function not found
	if (Timers.find(execID) == Timers.end())
		return 0;

	SDL_Event timerEvent;
	SDL_zero(timerEvent);

	timerEvent.user.type = TimerEventID;
	timerEvent.user.data1 = execID;

	SDL_PushEvent(&timerEvent);

	return interval;
}

void* SDL3DisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	void* execID = (void*)(uintptr_t)++TimerIDs;
	SDL_TimerID id = SDL_AddTimer(timeoutMilliseconds, SDL3DisplayWindow::ExecTimer, execID);

	if (!id) return (void*)(uintptr_t) id;

	Timers.insert({execID, onTimer});
	TimerHandles.insert({id, execID});

	return (void*)(uintptr_t) id;
}

void SDL3DisplayWindow::StopTimer(void* timerID)
{
	SDL_TimerID SDLTimerID = (SDL_TimerID)(uintptr_t)timerID;
	SDL_RemoveTimer(SDLTimerID);

	auto execID = TimerHandles.find(SDLTimerID);
	if (execID == TimerHandles.end())
		return;

	Timers.erase(execID->second);
	TimerHandles.erase(SDLTimerID);
}

SDL3DisplayWindow* SDL3DisplayWindow::FindEventWindow(const SDL_Event& event)
{
	static unsigned int windowID;

	if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
		windowID = event.window.windowID;
	else if (event.type == SDL_EVENT_TEXT_INPUT)
		windowID = event.text.windowID;
	else if (event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN)
		windowID = event.key.windowID;
	else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		windowID = event.button.windowID;
	else if (event.type == SDL_EVENT_MOUSE_WHEEL)
		windowID = event.wheel.windowID;
	else if (event.type == SDL_EVENT_MOUSE_MOTION)
		windowID = event.motion.windowID;
	// else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP || event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
	//  these don't have a windowID
	else if (event.type == PaintEventNumber)
		windowID = event.user.windowID;
	else
		return nullptr;

	const auto it = WindowList.find(windowID);
	return it != WindowList.end() ? it->second : nullptr;
}

void SDL3DisplayWindow::DispatchEvent(const SDL_Event& event)
{
	// timers are created in a non-window context
	if (event.type == TimerEventID)
		return OnTimerEvent(event.user);

	SDL3DisplayWindow* window = FindEventWindow(event);
	if (!window) return;

	static bool dropJoyDown, joyDown, keyDown;
	static unsigned long eventFrame = 0;

	if (eventFrame != event.common.timestamp / 10) // millis / 10
	{
		eventFrame = event.common.timestamp / 10;
		joyDown = keyDown = false;
	}

	if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) joyDown = true;
	if (event.type == SDL_EVENT_KEY_DOWN) keyDown = true;
	if (joyDown && keyDown) dropJoyDown = true;

	// steamdeck desktop mode fires double events
	if (dropJoyDown && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) return;

	if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
		window->OnWindowEvent(event.window);
	else if (event.type == SDL_EVENT_TEXT_INPUT || event.type == SDL_EVENT_TEXT_EDITING)
		window->OnTextInput(event.text);
	else if (event.type == SDL_EVENT_KEY_UP)
		window->OnKeyUp(event.key);
	else if (event.type == SDL_EVENT_KEY_DOWN)
		window->OnKeyDown(event.key);
	else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		window->OnMouseButtonUp(event.button);
	else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		window->OnMouseButtonDown(event.button);
	else if (event.type == SDL_EVENT_MOUSE_WHEEL)
		window->OnMouseWheel(event.wheel);
	else if (event.type == SDL_EVENT_MOUSE_MOTION)
		window->OnMouseMotion(event.motion);
	else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
		window->OnJoyButtonUp(event.gbutton);
	else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
		window->OnJoyButtonDown(event.gbutton);
	else if (event.type == PaintEventNumber)
		window->OnPaintEvent();
}

void SDL3DisplayWindow::OnWindowEvent(const SDL_WindowEvent& event)
{
	if (!(event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST))
		return;

	if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		WindowHost->OnWindowClose();
	else if (event.type == SDL_EVENT_WINDOW_MOVED ||
			 event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
			 event.type == SDL_EVENT_WINDOW_RESIZED)
		WindowHost->OnWindowGeometryChanged();
	else if (event.type == SDL_EVENT_WINDOW_SHOWN ||
			 event.type == SDL_EVENT_WINDOW_EXPOSED)
		WindowHost->OnWindowPaint();
	else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
		WindowHost->OnWindowActivated();
	else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		WindowHost->OnWindowDeactivated();
	else if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE)
		WindowHost->OnWindowMouseLeave();
}

void SDL3DisplayWindow::OnTextInput(const SDL_TextInputEvent& event)
{
	WindowHost->OnWindowKeyChar(event.text);
}

void SDL3DisplayWindow::OnJoyButtonUp(const SDL_GamepadButtonEvent& event)
{
	WindowHost->OnWindowKeyUp(GameControllerButtonToInputKey((SDL_GamepadButton)event.button));
}

void SDL3DisplayWindow::OnJoyButtonDown(const SDL_GamepadButtonEvent& event)
{
	WindowHost->OnWindowKeyDown(GameControllerButtonToInputKey((SDL_GamepadButton)event.button));
}

void SDL3DisplayWindow::OnKeyUp(const SDL_KeyboardEvent& event)
{
	WindowHost->OnWindowKeyUp(ScancodeToInputKey(event.scancode));
}

void SDL3DisplayWindow::OnKeyDown(const SDL_KeyboardEvent& event)
{
	WindowHost->OnWindowKeyDown(ScancodeToInputKey(event.scancode));
}

InputKey SDL3DisplayWindow::GetMouseButtonKey(const SDL_MouseButtonEvent& event)
{
	switch (event.button)
	{
		case SDL_BUTTON_LEFT: return InputKey::LeftMouse;
		case SDL_BUTTON_MIDDLE: return InputKey::MiddleMouse;
		case SDL_BUTTON_RIGHT: return InputKey::RightMouse;
		// case SDL_BUTTON_X1: return InputKey::XButton1;
		// case SDL_BUTTON_X2: return InputKey::XButton2;
		default: return InputKey::None;
	}
}

void SDL3DisplayWindow::OnMouseButtonUp(const SDL_MouseButtonEvent& event)
{
	InputKey key = GetMouseButtonKey(event);
	if (key != InputKey::None)
	{
		WindowHost->OnWindowMouseUp(GetMousePos(event), key);
	}
}

void SDL3DisplayWindow::OnMouseButtonDown(const SDL_MouseButtonEvent& event)
{
	InputKey key = GetMouseButtonKey(event);
	if (key != InputKey::None)
	{
		WindowHost->OnWindowMouseDown(GetMousePos(event), key);
	}
}

void SDL3DisplayWindow::OnMouseWheel(const SDL_MouseWheelEvent& event)
{
	float x = 0, y = 0;
	SDL_GetMouseState(&x, &y);
	double uiscale = GetDpiScale();
	Point mousepos(x / uiscale, y / uiscale);

	if (event.y > 0)
		WindowHost->OnWindowMouseWheel(mousepos, InputKey::MouseWheelUp);
	else if (event.y < 0)
		WindowHost->OnWindowMouseWheel(mousepos, InputKey::MouseWheelDown);
}

void SDL3DisplayWindow::OnMouseMotion(const SDL_MouseMotionEvent& event)
{
	if (CursorLocked)
	{
		WindowHost->OnWindowRawMouseMove(event.xrel, event.yrel);
	}
	else
	{
		WindowHost->OnWindowMouseMove(GetMousePos(event));
	}
}

void SDL3DisplayWindow::OnPaintEvent()
{
	WindowHost->OnWindowPaint();
}

void SDL3DisplayWindow::OnTimerEvent(const SDL_UserEvent& event)
{
	auto func = Timers.find(event.data1);

	// incase timer was cancelled before we get here
	if (func == Timers.end())
		return;

	func->second();
}

InputKey SDL3DisplayWindow::ScancodeToInputKey(SDL_Scancode keycode)
{
	switch (keycode)
	{
		case SDL_SCANCODE_BACKSPACE:    return InputKey::Backspace;
		case SDL_SCANCODE_TAB:          return InputKey::Tab;
		case SDL_SCANCODE_CLEAR:        return InputKey::OEMClear;
		case SDL_SCANCODE_RETURN:       return InputKey::Enter;
		case SDL_SCANCODE_MENU:         return InputKey::Alt;
		case SDL_SCANCODE_PAUSE:        return InputKey::Pause;
		case SDL_SCANCODE_ESCAPE:       return InputKey::Escape;
		case SDL_SCANCODE_SPACE:        return InputKey::Space;
		case SDL_SCANCODE_END:          return InputKey::End;
		case SDL_SCANCODE_PAGEDOWN:     return InputKey::PageDown;
		case SDL_SCANCODE_HOME:         return InputKey::Home;
		case SDL_SCANCODE_PAGEUP:       return InputKey::PageUp;
		case SDL_SCANCODE_LEFT:         return InputKey::Left;
		case SDL_SCANCODE_UP:           return InputKey::Up;
		case SDL_SCANCODE_RIGHT:        return InputKey::Right;
		case SDL_SCANCODE_DOWN:         return InputKey::Down;
		case SDL_SCANCODE_SELECT:       return InputKey::Select;
		case SDL_SCANCODE_PRINTSCREEN:  return InputKey::Print;
		case SDL_SCANCODE_EXECUTE:      return InputKey::Execute;
		case SDL_SCANCODE_INSERT:       return InputKey::Insert;
		case SDL_SCANCODE_DELETE:       return InputKey::Delete;
		case SDL_SCANCODE_HELP:         return InputKey::Help;
		case SDL_SCANCODE_0:            return InputKey::_0;
		case SDL_SCANCODE_1:            return InputKey::_1;
		case SDL_SCANCODE_2:            return InputKey::_2;
		case SDL_SCANCODE_3:            return InputKey::_3;
		case SDL_SCANCODE_4:            return InputKey::_4;
		case SDL_SCANCODE_5:            return InputKey::_5;
		case SDL_SCANCODE_6:            return InputKey::_6;
		case SDL_SCANCODE_7:            return InputKey::_7;
		case SDL_SCANCODE_8:            return InputKey::_8;
		case SDL_SCANCODE_9:            return InputKey::_9;
		case SDL_SCANCODE_A:            return InputKey::A;
		case SDL_SCANCODE_B:            return InputKey::B;
		case SDL_SCANCODE_C:            return InputKey::C;
		case SDL_SCANCODE_D:            return InputKey::D;
		case SDL_SCANCODE_E:            return InputKey::E;
		case SDL_SCANCODE_F:            return InputKey::F;
		case SDL_SCANCODE_G:            return InputKey::G;
		case SDL_SCANCODE_H:            return InputKey::H;
		case SDL_SCANCODE_I:            return InputKey::I;
		case SDL_SCANCODE_J:            return InputKey::J;
		case SDL_SCANCODE_K:            return InputKey::K;
		case SDL_SCANCODE_L:            return InputKey::L;
		case SDL_SCANCODE_M:            return InputKey::M;
		case SDL_SCANCODE_N:            return InputKey::N;
		case SDL_SCANCODE_O:            return InputKey::O;
		case SDL_SCANCODE_P:            return InputKey::P;
		case SDL_SCANCODE_Q:            return InputKey::Q;
		case SDL_SCANCODE_R:            return InputKey::R;
		case SDL_SCANCODE_S:            return InputKey::S;
		case SDL_SCANCODE_T:            return InputKey::T;
		case SDL_SCANCODE_U:            return InputKey::U;
		case SDL_SCANCODE_V:            return InputKey::V;
		case SDL_SCANCODE_W:            return InputKey::W;
		case SDL_SCANCODE_X:            return InputKey::X;
		case SDL_SCANCODE_Y:            return InputKey::Y;
		case SDL_SCANCODE_Z:            return InputKey::Z;
		case SDL_SCANCODE_KP_0:         return InputKey::NumPad0;
		case SDL_SCANCODE_KP_1:         return InputKey::NumPad1;
		case SDL_SCANCODE_KP_2:         return InputKey::NumPad2;
		case SDL_SCANCODE_KP_3:         return InputKey::NumPad3;
		case SDL_SCANCODE_KP_4:         return InputKey::NumPad4;
		case SDL_SCANCODE_KP_5:         return InputKey::NumPad5;
		case SDL_SCANCODE_KP_6:         return InputKey::NumPad6;
		case SDL_SCANCODE_KP_7:         return InputKey::NumPad7;
		case SDL_SCANCODE_KP_8:         return InputKey::NumPad8;
		case SDL_SCANCODE_KP_9:         return InputKey::NumPad9;
		// case SDL_SCANCODE_KP_ENTER:     return InputKey::NumPadEnter;
		// case SDL_SCANCODE_KP_MULTIPLY:  return InputKey::Multiply;
		// case SDL_SCANCODE_KP_PLUS:      return InputKey::Add;
		case SDL_SCANCODE_SEPARATOR:    return InputKey::Separator;
		// case SDL_SCANCODE_KP_MINUS:     return InputKey::Subtract;
		case SDL_SCANCODE_KP_PERIOD:    return InputKey::NumPadPeriod;
		// case SDL_SCANCODE_KP_DIVIDE:    return InputKey::Divide;
		case SDL_SCANCODE_F1:           return InputKey::F1;
		case SDL_SCANCODE_F2:           return InputKey::F2;
		case SDL_SCANCODE_F3:           return InputKey::F3;
		case SDL_SCANCODE_F4:           return InputKey::F4;
		case SDL_SCANCODE_F5:           return InputKey::F5;
		case SDL_SCANCODE_F6:           return InputKey::F6;
		case SDL_SCANCODE_F7:           return InputKey::F7;
		case SDL_SCANCODE_F8:           return InputKey::F8;
		case SDL_SCANCODE_F9:           return InputKey::F9;
		case SDL_SCANCODE_F10:          return InputKey::F10;
		case SDL_SCANCODE_F11:          return InputKey::F11;
		case SDL_SCANCODE_F12:          return InputKey::F12;
		case SDL_SCANCODE_F13:          return InputKey::F13;
		case SDL_SCANCODE_F14:          return InputKey::F14;
		case SDL_SCANCODE_F15:          return InputKey::F15;
		case SDL_SCANCODE_F16:          return InputKey::F16;
		case SDL_SCANCODE_F17:          return InputKey::F17;
		case SDL_SCANCODE_F18:          return InputKey::F18;
		case SDL_SCANCODE_F19:          return InputKey::F19;
		case SDL_SCANCODE_F20:          return InputKey::F20;
		case SDL_SCANCODE_F21:          return InputKey::F21;
		case SDL_SCANCODE_F22:          return InputKey::F22;
		case SDL_SCANCODE_F23:          return InputKey::F23;
		case SDL_SCANCODE_F24:          return InputKey::F24;
		case SDL_SCANCODE_NUMLOCKCLEAR: return InputKey::NumLock;
		case SDL_SCANCODE_SCROLLLOCK:   return InputKey::ScrollLock;
		case SDL_SCANCODE_LSHIFT:       return InputKey::LShift;
		case SDL_SCANCODE_RSHIFT:       return InputKey::RShift;
		case SDL_SCANCODE_LCTRL:        return InputKey::LControl;
		case SDL_SCANCODE_RCTRL:        return InputKey::RControl;
		case SDL_SCANCODE_GRAVE:        return InputKey::Tilde;
		default:                        return InputKey::None;
	}
}

InputKey SDL3DisplayWindow::GameControllerButtonToInputKey(const SDL_GamepadButton button)
{
	switch (button)
	{
		case SDL_GAMEPAD_BUTTON_DPAD_UP:        return InputKey::Up;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return InputKey::Down;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return InputKey::Left;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return InputKey::Right;
		case SDL_GAMEPAD_BUTTON_SOUTH:          return InputKey::Enter;
		case SDL_GAMEPAD_BUTTON_EAST:           return InputKey::Backspace;
		case SDL_GAMEPAD_BUTTON_WEST:           return InputKey::Space;
		case SDL_GAMEPAD_BUTTON_NORTH:          return InputKey::Escape;
		case SDL_GAMEPAD_BUTTON_START:          return InputKey::Enter;
		case SDL_GAMEPAD_BUTTON_BACK:           return InputKey::Escape;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return InputKey::Tab;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return InputKey::Tab;
		default:                                return InputKey::None;
	}
}

SDL_Scancode SDL3DisplayWindow::InputKeyToScancode(InputKey inputkey)
{
	switch (inputkey)
	{
		case InputKey::Backspace:     return SDL_SCANCODE_BACKSPACE;
		case InputKey::Tab:           return SDL_SCANCODE_TAB;
		case InputKey::OEMClear:      return SDL_SCANCODE_CLEAR;
		case InputKey::Enter:         return SDL_SCANCODE_RETURN;
		case InputKey::Alt:           return SDL_SCANCODE_MENU;
		case InputKey::Pause:         return SDL_SCANCODE_PAUSE;
		case InputKey::Escape:        return SDL_SCANCODE_ESCAPE;
		case InputKey::Space:         return SDL_SCANCODE_SPACE;
		case InputKey::End:           return SDL_SCANCODE_END;
		case InputKey::Home:          return SDL_SCANCODE_HOME;
		case InputKey::Left:          return SDL_SCANCODE_LEFT;
		case InputKey::Up:            return SDL_SCANCODE_UP;
		case InputKey::Right:         return SDL_SCANCODE_RIGHT;
		case InputKey::Down:          return SDL_SCANCODE_DOWN;
		case InputKey::Select:        return SDL_SCANCODE_SELECT;
		case InputKey::Print:         return SDL_SCANCODE_PRINTSCREEN;
		case InputKey::Execute:       return SDL_SCANCODE_EXECUTE;
		case InputKey::Insert:        return SDL_SCANCODE_INSERT;
		case InputKey::Delete:        return SDL_SCANCODE_DELETE;
		case InputKey::Help:          return SDL_SCANCODE_HELP;
		case InputKey::_0:            return SDL_SCANCODE_0;
		case InputKey::_1:            return SDL_SCANCODE_1;
		case InputKey::_2:            return SDL_SCANCODE_2;
		case InputKey::_3:            return SDL_SCANCODE_3;
		case InputKey::_4:            return SDL_SCANCODE_4;
		case InputKey::_5:            return SDL_SCANCODE_5;
		case InputKey::_6:            return SDL_SCANCODE_6;
		case InputKey::_7:            return SDL_SCANCODE_7;
		case InputKey::_8:            return SDL_SCANCODE_8;
		case InputKey::_9:            return SDL_SCANCODE_9;
		case InputKey::A:             return SDL_SCANCODE_A;
		case InputKey::B:             return SDL_SCANCODE_B;
		case InputKey::C:             return SDL_SCANCODE_C;
		case InputKey::D:             return SDL_SCANCODE_D;
		case InputKey::E:             return SDL_SCANCODE_E;
		case InputKey::F:             return SDL_SCANCODE_F;
		case InputKey::G:             return SDL_SCANCODE_G;
		case InputKey::H:             return SDL_SCANCODE_H;
		case InputKey::I:             return SDL_SCANCODE_I;
		case InputKey::J:             return SDL_SCANCODE_J;
		case InputKey::K:             return SDL_SCANCODE_K;
		case InputKey::L:             return SDL_SCANCODE_L;
		case InputKey::M:             return SDL_SCANCODE_M;
		case InputKey::N:             return SDL_SCANCODE_N;
		case InputKey::O:             return SDL_SCANCODE_O;
		case InputKey::P:             return SDL_SCANCODE_P;
		case InputKey::Q:             return SDL_SCANCODE_Q;
		case InputKey::R:             return SDL_SCANCODE_R;
		case InputKey::S:             return SDL_SCANCODE_S;
		case InputKey::T:             return SDL_SCANCODE_T;
		case InputKey::U:             return SDL_SCANCODE_U;
		case InputKey::V:             return SDL_SCANCODE_V;
		case InputKey::W:             return SDL_SCANCODE_W;
		case InputKey::X:             return SDL_SCANCODE_X;
		case InputKey::Y:             return SDL_SCANCODE_Y;
		case InputKey::Z:             return SDL_SCANCODE_Z;
		case InputKey::NumPad0:       return SDL_SCANCODE_KP_0;
		case InputKey::NumPad1:       return SDL_SCANCODE_KP_1;
		case InputKey::NumPad2:       return SDL_SCANCODE_KP_2;
		case InputKey::NumPad3:       return SDL_SCANCODE_KP_3;
		case InputKey::NumPad4:       return SDL_SCANCODE_KP_4;
		case InputKey::NumPad5:       return SDL_SCANCODE_KP_5;
		case InputKey::NumPad6:       return SDL_SCANCODE_KP_6;
		case InputKey::NumPad7:       return SDL_SCANCODE_KP_7;
		case InputKey::NumPad8:       return SDL_SCANCODE_KP_8;
		case InputKey::NumPad9:       return SDL_SCANCODE_KP_9;
		// case InputKey::NumPadEnter:   return SDL_SCANCODE_KP_ENTER;
		// case InputKey::Multiply       return SDL_SCANCODE_KP_MULTIPLY:;
		// case InputKey::Add:           return SDL_SCANCODE_KP_PLUS;
		case InputKey::Separator:     return SDL_SCANCODE_SEPARATOR;
		// case InputKey::Subtract:      return SDL_SCANCODE_KP_MINUS;
		case InputKey::NumPadPeriod:  return SDL_SCANCODE_KP_PERIOD;
		// case InputKey::Divide:        return SDL_SCANCODE_KP_DIVIDE;
		case InputKey::F1:            return SDL_SCANCODE_F1;
		case InputKey::F2:            return SDL_SCANCODE_F2;
		case InputKey::F3:            return SDL_SCANCODE_F3;
		case InputKey::F4:            return SDL_SCANCODE_F4;
		case InputKey::F5:            return SDL_SCANCODE_F5;
		case InputKey::F6:            return SDL_SCANCODE_F6;
		case InputKey::F7:            return SDL_SCANCODE_F7;
		case InputKey::F8:            return SDL_SCANCODE_F8;
		case InputKey::F9:            return SDL_SCANCODE_F9;
		case InputKey::F10:           return SDL_SCANCODE_F10;
		case InputKey::F11:           return SDL_SCANCODE_F11;
		case InputKey::F12:           return SDL_SCANCODE_F12;
		case InputKey::F13:           return SDL_SCANCODE_F13;
		case InputKey::F14:           return SDL_SCANCODE_F14;
		case InputKey::F15:           return SDL_SCANCODE_F15;
		case InputKey::F16:           return SDL_SCANCODE_F16;
		case InputKey::F17:           return SDL_SCANCODE_F17;
		case InputKey::F18:           return SDL_SCANCODE_F18;
		case InputKey::F19:           return SDL_SCANCODE_F19;
		case InputKey::F20:           return SDL_SCANCODE_F20;
		case InputKey::F21:           return SDL_SCANCODE_F21;
		case InputKey::F22:           return SDL_SCANCODE_F22;
		case InputKey::F23:           return SDL_SCANCODE_F23;
		case InputKey::F24:           return SDL_SCANCODE_F24;
		case InputKey::NumLock:       return SDL_SCANCODE_NUMLOCKCLEAR;
		case InputKey::ScrollLock:    return SDL_SCANCODE_SCROLLLOCK;
		case InputKey::LShift:        return SDL_SCANCODE_LSHIFT;
		case InputKey::RShift:        return SDL_SCANCODE_RSHIFT;
		case InputKey::LControl:      return SDL_SCANCODE_LCTRL;
		case InputKey::RControl:      return SDL_SCANCODE_RCTRL;
		case InputKey::Tilde:         return SDL_SCANCODE_GRAVE;
		default:                      return SDL_SCANCODE_UNKNOWN;
	}
}
