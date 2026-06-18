/*
** system_theme.cpp
**
** Gets system theme from operating system
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** On Windows:
** - Reads registry
** On Linux:
** - Queries the org.freedesktop.portal.Settings interface
** On MacOS:
** - Unimplemented
** On Haiku:
** - Unimplemented
*/

// HEADER FILES ------------------------------------------------------------

#include <memory>
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <array>
#include <string>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "system_theme.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

#ifdef _WIN32

#elif defined(__linux__)

std::string pipe_read(const char *);
std::string gdbus_read(std::string);

#endif

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifdef _WIN32

#elif defined(__linux__)

std::string pipe_read(const char * cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

	while (pipe && fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}

	return result;
}

std::string gdbus_read(std::string prop)
{
	std::string str = "gdbus call --session "
		"--dest org.freedesktop.portal.Desktop "
		"--object-path /org/freedesktop/portal/desktop "
		"--method org.freedesktop.portal.Settings.Read "
		"org.freedesktop.appearance " + prop + " 2>/dev/null";
	return pipe_read(str.c_str());
}

#endif

//==========================================================================
//
// Detect system theme and if the user has requested high-contrast mode
//
//==========================================================================

InterfaceTheme GetSystemTheme()
{
	InterfaceTheme result = Default;

#ifdef _WIN32

	DWORD use_light = -1;
	DWORD cb_data = sizeof(use_light);
	if (ERROR_SUCCESS == RegGetValueW( HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &use_light, &cb_data))
	{
		result = use_light == 0? Dark: Light;
	}

	HIGHCONTRASTW hc = {0};
	hc.cbSize = sizeof(HIGHCONTRASTW);
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &hc, 0))
	{
		if (hc.dwFlags & HCF_HIGHCONTRASTON)
			result = static_cast<InterfaceTheme>(result | HighContrast);
	}

#elif defined(__linux__)

	auto color = gdbus_read("color-scheme");
	if      (color.find("uint32 1") != std::string::npos) result = Dark;
	else if (color.find("uint32 2") != std::string::npos) result = Light;

	auto contrast = gdbus_read("contrast");
	if (contrast.find("uint32 1") != std::string::npos) result = static_cast<InterfaceTheme>(result | HighContrast);

#elif defined(__APPLE__)

	auto test = []<typename T>(CFTypeID type, CFPropertyListRef ref, auto &&pret)
	{
		bool res = false;
		if (ref)
		{
			res = CFGetTypeID(ref) == type && pret(static_cast<T>(ref));
			CFRelease(ref);
		}
		return res;
	};

	if (test.template operator()<CFStringRef>(CFStringGetTypeID(),
		CFPreferencesCopyAppValue(CFSTR("AppleInterfaceStyle"), kCFPreferencesAnyApplication),
		[](CFStringRef r) { return CFStringCompare(r, CFSTR("Dark"), 0) == kCFCompareEqualTo; }
	)) result = Dark;

	if (test.template operator()<CFBooleanRef>(CFBooleanGetTypeID(),
		CFPreferencesCopyValue(CFSTR("increaseContrast"), CFSTR("com.apple.universalaccess"), kCFPreferencesCurrentUser, kCFPreferencesAnyHost),
		[](CFBooleanRef r) { return CFBooleanGetValue(r); }
	)) result = static_cast<InterfaceTheme>(result | HighContrast);

#else

	// TODO: haiku

#endif

	return result;
}
