
#include "core/resourcedata.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <cmath>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ShlObj.h>

static std::vector<SingleFontData> LoadWindowsFont(const wchar_t* fontname, double pointSize)
{
	HDC dc = GetDC(0);
	if (!dc)
		throw std::runtime_error("GetDC(0) failed");

	HFONT font = CreateFont((int)std::round(-pointSize * GetDeviceCaps(dc, LOGPIXELSY) / 72.0), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, fontname);
	if (!font)
	{
		ReleaseDC(0, dc);
		throw std::runtime_error("CreateFont failed");
	}

	HFONT oldFont = (HFONT)SelectObject(dc, font);
	DWORD size = GetFontData(dc, 0, 0, nullptr, 0);
	if (size == GDI_ERROR)
	{
		SelectObject(dc, oldFont);
		DeleteObject(font);
		ReleaseDC(0, dc);
		throw std::runtime_error("GetFontData(size) failed");
	}

	SingleFontData fontdata;
	fontdata.fontdata.resize(size);
	DWORD result = GetFontData(dc, 0, 0, fontdata.fontdata.data(), size);
	if (result == GDI_ERROR)
	{
		SelectObject(dc, oldFont);
		DeleteObject(font);
		ReleaseDC(0, dc);
		throw std::runtime_error("GetFontData(data) failed");
	}

	SelectObject(dc, oldFont);
	DeleteObject(font);
	ReleaseDC(0, dc);

	return { std::move(fontdata) };
}

std::vector<SingleFontData> ResourceData::LoadSystemFont()
{
	return LoadWindowsFont(L"Segoe UI", GetSystemFontSize());
}

std::vector<SingleFontData> ResourceData::LoadMonospaceSystemFont()
{
	return LoadWindowsFont(L"Consolas", GetSystemFontSize());
}

double ResourceData::GetSystemFontSize()
{
	return 11.0;
}

class ResourceLoaderWin32 : public ResourceLoader
{
public:
	ResourceLoaderWin32()
	{
		// Tbd: what should be base directory be? The executable directory? CMAKE_CURRENT_BINARY_DIR doesn't point to it unfortunately
		// ResourcePath = FindExePath();
	}

	std::vector<SingleFontData> LoadFont(const std::string& name)
	{
		if (name == "system")
			return ResourceData::LoadSystemFont();
		else if (name == "monospace")
			return ResourceData::LoadMonospaceSystemFont();
		else
			return { SingleFontData{ReadAllBytes(name + ".ttf"), ""} };
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& filename)
	{
		HANDLE handle = CreateFile((ResourcePath + to_utf16(filename)).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (handle == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Could not open " + filename);

		LARGE_INTEGER fileSize;
		BOOL result = GetFileSizeEx(handle, &fileSize);
		if (result == FALSE)
		{
			CloseHandle(handle);
			throw std::runtime_error("GetFileSizeEx failed");
		}

		std::vector<uint8_t> buffer(fileSize.QuadPart);

		DWORD bytesRead = 0;
		result = ReadFile(handle, buffer.data(), (DWORD)buffer.size(), &bytesRead, nullptr);
		if (result == FALSE || bytesRead != buffer.size())
		{
			CloseHandle(handle);
			throw std::runtime_error("ReadFile failed");
		}

		CloseHandle(handle);

		return buffer;
	}

	std::wstring ResourcePath;

	static std::wstring FindExePath()
	{
		WCHAR exe_filename[1024];
		DWORD len = GetModuleFileNameW(nullptr, exe_filename, 1024);
		if (len == 0 || len == 1024)
			throw std::runtime_error("GetModuleFileName failed!");

		int lastSlash = 0;
		for (int i = 0; i < 1024 && exe_filename[i] != 0; i++)
		{
			if (exe_filename[i] == '/' || exe_filename[i] == '\\')
				lastSlash = i;
		}
		exe_filename[lastSlash] = 0;
		std::wstring path = exe_filename;
		if (!path.empty())
			path += L'\\';
		return path;
	}

	static std::wstring to_utf16(const std::string& str)
	{
		if (str.empty()) return {};
		int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		std::wstring result;
		result.resize(needed);
		needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		return result;
	}
};

struct ResourceDefaultLoader
{
	ResourceDefaultLoader() { loader = std::make_unique<ResourceLoaderWin32>(); }
	std::unique_ptr<ResourceLoader> loader;
};

static std::unique_ptr<ResourceLoader>& GetLoader()
{
	static ResourceDefaultLoader loader;
	return loader.loader;
}

ResourceLoader* ResourceLoader::Get()
{
	return GetLoader().get();
}

void ResourceLoader::Set(std::unique_ptr<ResourceLoader> instance)
{
	GetLoader() = std::move(instance);
}
