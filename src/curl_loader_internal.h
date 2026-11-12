/*
** curl_loader_internal.h
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

#pragma once

#include <curl/curl.h>

#ifdef RUNTIME_CURL

#include "curl_loader.h"
#include "i_module.h"

extern FModule CurlModule;

extern TReqProc<CurlModule, decltype(&curl_global_init)> p_curl_global_init;
extern TReqProc<CurlModule, decltype(&curl_easy_strerror)> p_curl_easy_strerror;
extern TReqProc<CurlModule, decltype(&curl_easy_init)> p_curl_easy_init;
extern TReqProc<CurlModule, decltype(&curl_easy_setopt)> p_curl_easy_setopt;
extern TReqProc<CurlModule, decltype(&curl_easy_perform)> p_curl_easy_perform;
extern TReqProc<CurlModule, decltype(&curl_easy_cleanup)> p_curl_easy_cleanup;

#ifndef NOSHADOW
	#define curl_global_init p_curl_global_init
	#define curl_easy_strerror p_curl_easy_strerror
	#define curl_easy_init p_curl_easy_init
	#define curl_easy_setopt p_curl_easy_setopt
	#define curl_easy_perform p_curl_easy_perform
	#define curl_easy_cleanup p_curl_easy_cleanup
#endif

#endif
