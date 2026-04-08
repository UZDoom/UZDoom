/*
** starter.h
**
** Header for starter.cpp
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "const.h"
#include <stdio.h>
#include <string>

// ImGui & SDL Backends
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <SDL.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#ifdef _WIN32
#include <windows.h> // SetProcessDPIAware()
#endif

struct FStartupSelectionInfo;

class Starter
{

  public:
	// Context structure holding the state of the subsystem initialization (reused with error and netstart windows ,
	// too)
	struct ImGuiContextState
	{
		SDL_Window   *window = nullptr;
		SDL_GLContext gl_context = nullptr;
		float scale = 1.0f;
	};

	static ImGuiContextState SetupContext(const char *title, int width, int height, Uint32 sdl_init_flags);
	static void TeardownContext(ImGuiContextState &state);

	static bool Init();
	static void RunLoop();
	static void Shutdown();

	// callback for other UIs to piggyback off the main loop, e.g. error and netstart windows
	using RenderCallback = std::function<void(bool &done)>;
	static void RunImGuiLoop(ImGuiContextState &context, const RenderCallback &renderCallback);
};

bool ImGuiKickStarter(const FStartupSelectionInfo &info);
