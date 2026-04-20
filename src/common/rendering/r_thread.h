/*
** r_thread.h
**
** Renderer multithreading framework
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "c_cvars.h"
#include "basics.h"

// Use multiple threads when drawing
EXTERN_CVAR(Int, r_multithreaded)

namespace swrenderer { class WallColumnDrawerArgs; }

// Worker data for each thread executing drawer commands
class DrawerThread
{
public:
	std::thread thread;
	size_t current_queue = 0;

	// Thread line index of this thread
	int core = 0;

	// Number of active threads
	int num_cores = 1;

	// NUMA node this thread belongs to
	int numa_node = 0;

	// Number of active NUMA nodes
	int num_numa_nodes = 1;

	// Active range for the numa block the cores are part of
	int numa_start_y = 0;
	int numa_end_y = MAXHEIGHT;

	// Working buffer used by the tilted (sloped) span drawer
	const uint8_t *tiltlighting[MAXWIDTH];

	size_t debug_draw_pos = 0;

	// Checks if a line is rendered by this thread
	bool line_skipped_by_thread(int line)
	{
		return line < numa_start_y || line >= numa_end_y || line % num_cores != core;
	}

	// The number of lines to skip to reach the first line to be rendered by this thread
	int skipped_by_thread(int first_line)
	{
		int clip_first_line = max(first_line, numa_start_y);
		int core_skip = (num_cores - (clip_first_line - core) % num_cores) % num_cores;
		return clip_first_line + core_skip - first_line;
	}

	// The number of lines to be rendered by this thread
	int count_for_thread(int first_line, int count)
	{
		count = min(count, numa_end_y - first_line);
		int c = (count - skipped_by_thread(first_line) + num_cores - 1) / num_cores;
		return max(c, 0);
	}

	// Calculate the dest address for the first line to be rendered by this thread
	template<typename T>
	T *dest_for_thread(int first_line, int pitch, T *dest)
	{
		return dest + skipped_by_thread(first_line) * pitch;
	}

	// The first line in the dc_temp buffer used this thread
	int temp_line_for_thread(int first_line)
	{
		return (first_line + skipped_by_thread(first_line)) / num_cores;
	}
};

// Task to be executed by each worker thread
class DrawerCommand
{
public:
	virtual ~DrawerCommand() { }

	virtual void Execute(DrawerThread *thread) = 0;
};

// Wait for all worker threads before executing next command
class GroupMemoryBarrierCommand : public DrawerCommand
{
public:
	void Execute(DrawerThread *thread);

private:
	std::mutex mutex;
	std::condition_variable condition;
	size_t count = 0;
};

// Copy finished rows to video memory
class MemcpyCommand : public DrawerCommand
{
public:
	MemcpyCommand(void *dest, int destpitch, const void *src, int width, int height, int srcpitch, int pixelsize);
	void Execute(DrawerThread *thread);

private:
	void *dest;
	const void *src;
	int destpitch;
	int width;
	int height;
	int srcpitch;
	int pixelsize;
};

class DrawerCommandQueue;
typedef std::shared_ptr<DrawerCommandQueue> DrawerCommandQueuePtr;

class DrawerThreads
{
public:
	// Runs the collected commands on worker threads
	static void Execute(DrawerCommandQueuePtr queue);

	// Waits for all commands to finish executing
	static void WaitForWorkers();

	static void ResetDebugDrawPos();

private:
	DrawerThreads();
	~DrawerThreads();

	void StartThreads();
	void StopThreads();
	void WorkerMain(DrawerThread *thread);

	static DrawerThreads *Instance();

	std::mutex threads_mutex;
	std::vector<DrawerThread> threads;

	std::mutex start_mutex;
	std::condition_variable start_condition;
	std::vector<DrawerCommandQueuePtr> active_commands;
	bool shutdown_flag = false;

	std::mutex end_mutex;
	std::condition_variable end_condition;
	size_t tasks_left = 0;

	size_t debug_draw_end = 0;

	DrawerThread single_core_thread;

	friend class DrawerCommandQueue;
};

class RenderMemory;

class DrawerCommandQueue
{
public:
	DrawerCommandQueue(RenderMemory *memoryAllocator);

	void Clear() { commands.clear(); }

	// Queue command to be executed by drawer worker threads
	template<typename T, typename... Types>
	void Push(Types &&... args)
	{
		DrawerThreads *threads = DrawerThreads::Instance();
		if (r_multithreaded != 0)
		{
			void *ptr = AllocMemory(sizeof(T));
			T *command = new (ptr)T(std::forward<Types>(args)...);
			commands.push_back(command);
		}
		else
		{
			T command(std::forward<Types>(args)...);
			command.Execute(&threads->single_core_thread);
		}
	}

private:
	// Allocate memory valid for the duration of a command execution
	void *AllocMemory(size_t size);

	std::vector<DrawerCommand *> commands;
	RenderMemory *FrameMemory;

	friend class DrawerThreads;
};
