/*
** curl_loader.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#define NOSHADOW
#include "curl_loader_internal.h"

FModule CurlModule {"Curl"};

TReqProc<CurlModule, decltype(&curl_global_init)> p_curl_global_init {"curl_global_init"};
TReqProc<CurlModule, decltype(&curl_easy_strerror)> p_curl_easy_strerror {"curl_easy_strerror"};
TReqProc<CurlModule, decltype(&curl_easy_init)> p_curl_easy_init {"curl_easy_init"};
TReqProc<CurlModule, decltype(&curl_easy_setopt)> p_curl_easy_setopt {"curl_easy_setopt"};
TReqProc<CurlModule, decltype(&curl_easy_perform)> p_curl_easy_perform {"curl_easy_perform"};
TReqProc<CurlModule, decltype(&curl_easy_cleanup)> p_curl_easy_cleanup {"curl_easy_cleanup"};

bool IsCurlLoaded()
{
	return CurlModule.IsLoaded();
}

bool LoadCurl()
{
#ifdef _M_ARM64
	return CurlModule.Load({"libcurl-arm64.dll"});
#else
	return CurlModule.Load({"libcurl-x64.dll"});
#endif
}
