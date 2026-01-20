#pragma once

#include <memory>
#include <string>
#include <functional>
#include <cstdint>
#include <cstdlib>
#include "../core/rect.h"

#ifndef VULKAN_H_

#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)

#endif

class Widget;
class OpenFileDialog;
class SaveFileDialog;
class OpenFolderDialog;
class Image;
class CustomCursor;

enum class StandardCursor
{
	arrow,
	appstarting,
	cross,
	hand,
	ibeam,
	no,
	size_all,
	size_nesw,
	size_ns,
	size_nwse,
	size_we,
	uparrow,
	wait
};

enum class InputKey : uint32_t
{
	None, LeftMouse, RightMouse, Cancel,
	MiddleMouse, Unknown05, Unknown06, Unknown07,
	Backspace, Tab, Unknown0A, Unknown0B,
	Unknown0C, Enter, Unknown0E, Unknown0F,
	Shift, Ctrl, Alt, Pause,
	CapsLock, LCommand, RCommand, Unknown17,
	Unknown18, Unknown19, Unknown1A, Escape,
	Unknown1C, Unknown1D, Unknown1E, Unknown1F,
	Space, PageUp, PageDown, End,
	Home, Left, Up, Right,
	Down, Select, Print, Execute,
	PrintScrn, Insert, Delete, Help,
	_0, _1, _2, _3,
	_4, _5, _6, _7,
	_8, _9, Unknown3A, Unknown3B,
	Unknown3C, Unknown3D, Unknown3E, Unknown3F,
	Unknown40, A, B, C,
	D, E, F, G,
	H, I, J, K,
	L, M, N, O,
	P, Q, R, S,
	T, U, V, W,
	X, Y, Z, Unknown5B,
	Unknown5C, Unknown5D, Unknown5E, Unknown5F,
	NumPad0, NumPad1, NumPad2, NumPad3,
	NumPad4, NumPad5, NumPad6, NumPad7,
	NumPad8, NumPad9, GreyStar, GreyPlus,
	Separator, GreyMinus, NumPadPeriod, GreySlash,
	F1, F2, F3, F4,
	F5, F6, F7, F8,
	F9, F10, F11, F12,
	F13, F14, F15, F16,
	F17, F18, F19, F20,
	F21, F22, F23, F24,
	Unknown88, Unknown89, Unknown8A, Unknown8B,
	Unknown8C, Unknown8D, Unknown8E, Unknown8F,
	NumLock, ScrollLock, Unknown92, Unknown93,
	Unknown94, Unknown95, Unknown96, Unknown97,
	Unknown98, Unknown99, Unknown9A, Unknown9B,
	Unknown9C, Unknown9D, Unknown9E, Unknown9F,
	LShift, RShift, LControl, RControl,
	UnknownA4, UnknownA5, UnknownA6, UnknownA7,
	UnknownA8, UnknownA9, UnknownAA, UnknownAB,
	UnknownAC, UnknownAD, UnknownAE, UnknownAF,
	UnknownB0, UnknownB1, UnknownB2, UnknownB3,
	UnknownB4, UnknownB5, UnknownB6, UnknownB7,
	UnknownB8, UnknownB9, Semicolon, Equals,
	Comma, Minus, Period, Slash,
	Tilde, UnknownC1, UnknownC2, UnknownC3,
	UnknownC4, UnknownC5, UnknownC6, UnknownC7,
	Joy1, Joy2, Joy3, Joy4,
	Joy5, Joy6, Joy7, Joy8,
	Joy9, Joy10, Joy11, Joy12,
	Joy13, Joy14, Joy15, Joy16,
	UnknownD8, UnknownD9, UnknownDA, LeftBracket,
	Backslash, RightBracket, SingleQuote, UnknownDF,
	JoyX, JoyY, JoyZ, JoyR,
	MouseX, MouseY, MouseZ, MouseW,
	JoyU, JoyV, UnknownEA, UnknownEB,
	MouseWheelUp, MouseWheelDown, Unknown10E, Unknown10F,
	JoyPovUp, JoyPovDown, JoyPovLeft, JoyPovRight,
	UnknownF4, UnknownF5, Attn, CrSel,
	ExSel, ErEof, Play, Zoom,
	NoName, PA1, OEMClear
};

// Raw keyboard code. Same as the DirectInput keycodes
enum class RawKeycode : uint32_t
{
	None = 0x00,
	Escape = 0x01,
	_1 = 0x02,
	_2 = 0x03,
	_3 = 0x04,
	_4 = 0x05,
	_5 = 0x06,
	_6 = 0x07,
	_7 = 0x08,
	_8 = 0x09,
	_9 = 0x0A,
	_0 = 0x0B,
	Minus = 0x0C,
	Equals = 0x0D,
	Backspace = 0x0E,
	Tab = 0x0F,
	Q = 0x10,
	W = 0x11,
	E = 0x12,
	R = 0x13,
	T = 0x14,
	Y = 0x15,
	U = 0x16,
	I = 0x17,
	O = 0x18,
	P = 0x19,
	LBracket = 0x1A,
	RBracket = 0x1B,
	Return = 0x1C,
	LControl = 0x1D,
	A = 0x1E,
	S = 0x1F,
	D = 0x20,
	F = 0x21,
	G = 0x22,
	H = 0x23,
	J = 0x24,
	K = 0x25,
	L = 0x26,
	Semicolon = 0x27,
	Apostrophe = 0x28,
	Grave = 0x29,
	LShift = 0x2A,
	Backslash = 0x2B,
	Z = 0x2C,
	X = 0x2D,
	C = 0x2E,
	V = 0x2F,
	B = 0x30,
	N = 0x31,
	M = 0x32,
	Comma = 0x33,
	Period = 0x34,
	Slash = 0x35,
	RShift = 0x36,
	NumpadMultiply = 0x37,
	LAlt = 0x38,
	Space = 0x39,
	CapsLock = 0x3A,
	F1 = 0x3B,
	F2 = 0x3C,
	F3 = 0x3D,
	F4 = 0x3E,
	F5 = 0x3F,
	F6 = 0x40,
	F7 = 0x41,
	F8 = 0x42,
	F9 = 0x43,
	F10 = 0x44,
	Numlock = 0x45,
	Scroll = 0x46,
	Numpad7 = 0x47,
	Numpad8 = 0x48,
	Numpad9 = 0x49,
	NumpadSubstract = 0x4A,
	Numpad4 = 0x4B,
	Numpad5 = 0x4C,
	Numpad6 = 0x4D,
	NumpadAdd = 0x4E,
	Numpad1 = 0x4F,
	Numpad2 = 0x50,
	Numpad3 = 0x51,
	Numpad0 = 0x52,
	NumpadDecimal = 0x53,
	OEM_102 = 0x56, // <> or \| on RT 102-key keyboard (Non-U.S.)
	F11 = 0x57,
	F12 = 0x58,
	F13 = 0x64,
	F14 = 0x65,
	F15 = 0x66,
	Kana = 0x70,
	AbntC1 = 0x73,
	Convert = 0x79,
	NoConvert = 0x7B,
	Yen = 0x7D,
	AbntC2 = 0x7E,
	NumpadEquals = 0x8D,
	PrevTrack = 0x90,
	At = 0x91,
	Colon = 0x92,
	Underline = 0x93,
	Kanji = 0x94,
	Stop = 0x95,
	Ax = 0x96,
	Unlabeled = 0x97,
	NextTrack = 0x99,
	NumpadEnter = 0x9C,
	RControl = 0x9D,
	Mute = 0xA0,
	Calculator = 0xA1,
	PlayPause = 0xA2,
	MediaStop = 0xA4,
	VolumeDown = 0xAE,
	VolumeUp = 0xB0,
	WebHome = 0xB2,
	NumpadComma = 0xB3,
	NumpadDivide = 0xB5,
	SysRq = 0xB7,
	RAlt = 0xB8,
	Pause = 0xC5,
	Home = 0xC7,
	Up = 0xC8,
	PageUp = 0xC9,
	Left = 0xCB,
	Right = 0xCD,
	End = 0xCF,
	Down = 0xD0,
	PageDown = 0xD1,
	Insert = 0xD2,
	Delete = 0xD3,
	LCmd = 0xDB,
	RCmd = 0xDC,
	Apps = 0xDD,
	Power = 0xDE,
	Sleep = 0xDF,
	Wake = 0xE3,
	WebSearch = 0xE5,
	WebFavorites = 0xE6,
	WebRefresh = 0xE7,
	WebStop = 0xE8,
	WebForward = 0xE9,
	WebBack = 0xEA,
	MyComputer = 0xEB,
	Mail = 0xEC,
	MediaSelect = 0xED
};

enum class RenderAPI
{
	Unspecified,
	Bitmap,
	Vulkan,
	OpenGL,
	D3D11,
	D3D12,
	Metal
};

class DisplayWindow;

class DisplayWindowHost
{
public:
	virtual void OnWindowPaint() = 0;
	virtual void OnWindowMouseMove(const Point& pos) = 0;
	virtual void OnWindowMouseLeave() = 0;
	virtual void OnWindowMouseDown(const Point& pos, InputKey key) = 0;
	virtual void OnWindowMouseDoubleclick(const Point& pos, InputKey key) = 0;
	virtual void OnWindowMouseUp(const Point& pos, InputKey key) = 0;
	virtual void OnWindowMouseWheel(const Point& pos, InputKey key) = 0;
	virtual void OnWindowRawMouseMove(int dx, int dy) = 0;
	virtual void OnWindowRawKey(RawKeycode keycode, bool down) = 0;
	virtual void OnWindowKeyChar(std::string chars) = 0;
	virtual void OnWindowKeyDown(InputKey key) = 0;
	virtual void OnWindowKeyUp(InputKey key) = 0;
	virtual void OnWindowGeometryChanged() = 0;
	virtual void OnWindowClose() = 0;
	virtual void OnWindowActivated() = 0;
	virtual void OnWindowDeactivated() = 0;
	virtual void OnWindowDpiScaleChanged() = 0;
};

class DisplayWindow
{
public:
	static std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI);

	static void ProcessEvents();
	static void RunLoop();
	static void ExitLoop();

	static void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer);
	static void StopTimer(void* timerID);

	static Size GetScreenSize();

	virtual ~DisplayWindow() = default;

	virtual void SetWindowTitle(const std::string& text) = 0;
	virtual void SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images) = 0;
	virtual void SetWindowFrame(const Rect& box) = 0;
	virtual void SetClientFrame(const Rect& box) = 0;
	virtual void Show() = 0;
	virtual void ShowFullscreen() = 0;
	virtual void ShowMaximized() = 0;
	virtual void ShowMinimized() = 0;
	virtual void ShowNormal() = 0;
	virtual bool IsWindowFullscreen() = 0;
	virtual void Hide() = 0;
	virtual void Activate() = 0;
	virtual void ShowCursor(bool enable) = 0;
	virtual void LockKeyboard() = 0;
	virtual void UnlockKeyboard() = 0;
	virtual void LockCursor() = 0;
	virtual void UnlockCursor() = 0;
	virtual void CaptureMouse() = 0;
	virtual void ReleaseMouseCapture() = 0;
	virtual void Update() = 0;
	virtual bool GetKeyState(InputKey key) = 0;

	virtual void SetCursor(StandardCursor cursor, std::shared_ptr<CustomCursor> custom) = 0;

	virtual Rect GetWindowFrame() const = 0;
	virtual Size GetClientSize() const = 0;
	virtual int GetPixelWidth() const = 0;
	virtual int GetPixelHeight() const = 0;
	virtual double GetDpiScale() const = 0;

	virtual Point MapFromGlobal(const Point& pos) const = 0;
	virtual Point MapToGlobal(const Point& pos) const = 0;

	virtual void SetBorderColor(uint32_t bgra8) = 0;
	virtual void SetCaptionColor(uint32_t bgra8) = 0;
	virtual void SetCaptionTextColor(uint32_t bgra8) = 0;

	virtual void PresentBitmap(int width, int height, const uint32_t* pixels) = 0;

	virtual std::string GetClipboardText() = 0;
	virtual void SetClipboardText(const std::string& text) = 0;

	virtual void* GetNativeHandle() = 0;

	virtual std::vector<std::string> GetVulkanInstanceExtensions() = 0;
	virtual VkSurfaceKHR CreateVulkanSurface(VkInstance instance) = 0;
};

class DisplayBackend
{
public:
	static DisplayBackend* Get();
	static void Set(std::unique_ptr<DisplayBackend> instance);

	static std::unique_ptr<DisplayBackend> TryCreateWin32();
	static std::unique_ptr<DisplayBackend> TryCreateSDL2();
	static std::unique_ptr<DisplayBackend> TryCreateSDL3();
	static std::unique_ptr<DisplayBackend> TryCreateX11();
	static std::unique_ptr<DisplayBackend> TryCreateWayland();
	static std::unique_ptr<DisplayBackend> TryCreateCocoa();

	static std::unique_ptr<DisplayBackend> TryCreateBackend();

	virtual ~DisplayBackend() = default;

	virtual bool IsWin32() { return false; }
	virtual bool IsSDL2() { return false; }
	virtual bool IsSDL3() { return false; }
	virtual bool IsX11() { return false; }
	virtual bool IsWayland() { return false; }
	virtual bool IsCocoa() { return false; }

	virtual std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI) = 0;
	virtual void ProcessEvents() = 0;
	virtual void RunLoop() = 0;
	virtual void ExitLoop() = 0;

	virtual void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) = 0;
	virtual void StopTimer(void* timerID) = 0;

	virtual Size GetScreenSize() = 0;

	virtual std::unique_ptr<OpenFileDialog> CreateOpenFileDialog(DisplayWindow* owner);
	virtual std::unique_ptr<SaveFileDialog> CreateSaveFileDialog(DisplayWindow* owner);
	virtual std::unique_ptr<OpenFolderDialog> CreateOpenFolderDialog(DisplayWindow* owner);
};
