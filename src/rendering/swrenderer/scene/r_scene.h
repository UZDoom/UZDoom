/*
** r_scene.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2016 Magnus Norddahl
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <stddef.h>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "r_defs.h"
#include "d_player.h"

extern cycle_t FrameCycles;

namespace swrenderer
{
	extern cycle_t WallCycles, PlaneCycles, MaskedCycles, DrawerWaitCycles;

	class RenderThread;
	
	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		void Deinit();	

		void SetClearColor(int color);
		
		void RenderView(player_t *player, DCanvas *target, void *videobuffer, int bufferpitch);
		void RenderViewToCanvas(AActor *actor, DCanvas *canvas, int x, int y, int width, int height, bool dontmaplines = false);
	
		bool DontMapLines() const { return dontmaplines; }

		RenderThread *MainThread() { return Threads.front().get(); }

	private:
		void RenderActorView(AActor *actor,bool renderplayersprite, bool dontmaplines);
		void RenderThreadSlices();
		void RenderThreadSlice(RenderThread *thread);
		void RenderPSprites();

		void StartThreads(size_t numThreads);
		void StopThreads();
		
		bool dontmaplines = false;
		int clearcolor = 0;

		std::vector<std::unique_ptr<RenderThread>> Threads;
		std::mutex start_mutex;
		std::condition_variable start_condition;
		bool shutdown_flag = false;
		int run_id = 0;
		std::mutex end_mutex;
		std::condition_variable end_condition;
		size_t finished_threads = 0;
	};
}
