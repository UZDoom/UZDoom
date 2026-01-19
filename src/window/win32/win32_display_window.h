#pragma once

#include "win32_util.h"

#include <list>
#include <unordered_map>
#include <zwidget/window/window.h>
#include <zwidget/window/win32nativehandle.h>

class Win32CustomCursor;

class Win32DisplayWindow : public DisplayWindow
{
public:
	Win32DisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, Win32DisplayWindow* owner, RenderAPI renderAPI);
	~Win32DisplayWindow();

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
	void UpdateCursor();

	Rect GetWindowFrame() const override;
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

	Point GetLParamPos(LPARAM lparam) const;

	RawKeycode ToDirectInputKeycode(RAWINPUT* rawinput);

	void* GetNativeHandle() override { return &WindowHandle; }

	std::vector<std::string> GetVulkanInstanceExtensions() override;
	VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;

	static void ProcessEvents();
	static void RunLoop();
	static void ExitLoop();
	static Size GetScreenSize();

	static void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer);
	static void StopTimer(void* timerID);

	static bool ExitRunLoop;
	static std::list<Win32DisplayWindow*> Windows;
	std::list<Win32DisplayWindow*>::iterator WindowsIterator;

	static std::unordered_map<UINT_PTR, std::function<void()>> Timers;

	LRESULT OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam);

	DisplayWindowHost* WindowHost = nullptr;
	bool PopupWindow = false;

	Win32NativeHandle WindowHandle;
	bool Fullscreen = false;

	bool KeyboardLocked = false;

	bool MouseLocked = false;
	POINT MouseLockPos = {};

	bool TrackMouseActive = false;

	HDC PaintDC = 0;

	HICON SmallIcon = {};
	HICON LargeIcon = {};

	StandardCursor CurrentCursor = StandardCursor::arrow;
	std::shared_ptr<CustomCursor> CurrentCustomCursor;
	std::unique_ptr<Win32CustomCursor> CurrentWin32CustomCursor;

	uint16_t E1Prefix = 0;
};

class Win32CustomCursor
{
public:
	Win32CustomCursor(const std::shared_ptr<CustomCursor>& cursor_description, double dpiscale);
	~Win32CustomCursor();

	HCURSOR handle = 0;

private:
	static HCURSOR create_cursor(const std::shared_ptr<CustomCursor>& cursor_description, double dpiscale);
	static std::vector<uint8_t> create_cur_file(const std::shared_ptr<Image>& image, const Point& hotspot);
	static std::vector<uint8_t> create_ani_file(const std::shared_ptr<CustomCursor>& cursor_description);
	static std::vector<uint8_t> create_ico_helper(const std::vector<std::shared_ptr<Image>>& images, WORD type, const std::vector<Point>& hotspots);
	static void set_riff_header(uint8_t* data, const char* type, DWORD size);

	struct ANIHeader
	{
		DWORD cbSizeOf;              // Number of bytes in AniHeader (36 bytes)
		DWORD cFrames;               // Number of unique Icons in this cursor
		DWORD cSteps;                // Number of Blits before the animation cycles
		DWORD cx, cy;                // reserved, must be zero.
		DWORD cBitCount, cPlanes;    // reserved, must be zero.
		DWORD JifRate;               // Default Jiffies (1/60th of a second) if rate chunk not present.
		DWORD flags;                 // Animation Flag (see AF_ constants)
	};

	#define AF_ICON 0x0001L // Windows format icon/cursor animation

	struct ICONHEADER
	{
		WORD idReserved;
		WORD idType; // 1 = ICO, 2 = CUR
		WORD idCount;
	};

	struct IconDirectoryEntry
	{
		BYTE bWidth;
		BYTE bHeight;
		BYTE bColorCount;
		BYTE bReserved;
		union
		{
			WORD wPlanes;	// ICO format
			SHORT XHotspot; // CUR format
		};
		union
		{
			WORD wBitCount; // ICO format
			SHORT YHotspot; // CUR format
		};
		DWORD dwBytesInRes;
		DWORD dwImageOffset;
		/** WORD nID; // Mentioned by http://msdn2.microsoft.com/en-us/library/ms997538.aspx but not in other ICO docs.*/
	};

	struct ANIInfo
	{
		int length() const
		{
			// todo: dword align string lengths
			return 5 * 4 + (int)title.length() + 1 + (int)author.length() + 1;
		}

		void write(uint8_t* d)
		{
			memcpy(d, "INFOINAM", 8);
			*(DWORD*)(d + 8) = (DWORD)title.length() + 1;
			memcpy(d + 12, title.c_str(), title.length() + 1);
			int offset_art = 12 + (int)title.length() + 1;
			memcpy(d + offset_art, "IART", 4);
			*(DWORD*)(d + offset_art + 4) = (DWORD)author.length() + 1;
			memcpy(d + offset_art + 8, author.c_str(), author.length() + 1);
		}

		std::string title;
		std::string author;
	};

	struct ANIFrames
	{
		int length() const
		{
			size_t s = 4 + 8 * icons.size();
			for (size_t i = 0; i < icons.size(); i++)
				s += icons[i].size();
			return (int)s;
		}

		void write(uint8_t* d)
		{
			memcpy(d, "fram", 4);
			size_t p = 4;
			for (size_t i = 0; i < icons.size(); i++)
			{
				memcpy(d + p, "icon", 4);
				*(DWORD*)(d + p + 4) = (DWORD)icons[i].size();
				memcpy(d + p + 8, icons[i].data(), icons[i].size());
				p += 8 + icons[i].size();
			}
		}

		std::vector<std::vector<uint8_t>> icons;
	};

	enum { size_header = 6, size_direntry = 16 };
};
