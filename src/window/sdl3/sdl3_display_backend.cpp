#include "sdl3_display_backend.h"
#include "sdl3_display_window.h"
#include <stdexcept>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_joystick.h>
#ifndef WIN32
#include <dlfcn.h>
#endif

SDL3DisplayBackend::SDL3DisplayBackend()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		throw std::runtime_error(std::string("Unable to initialize SDL:") + SDL_GetError());

	SDL3DisplayWindow::PaintEventNumber = SDL_RegisterEvents(1);

	int joystickCount;
	const SDL_JoystickID* joystickIDs = SDL_GetJoysticks(&joystickCount);

	for (auto i = 0; i < joystickCount ; i++)
	{
		SDL_OpenJoystick(joystickIDs[i]);
	}
}

std::unique_ptr<DisplayWindow> SDL3DisplayBackend::Create(DisplayWindowHost* windowHost, WidgetType type, DisplayWindow* owner, RenderAPI renderAPI)
{
	return std::make_unique<SDL3DisplayWindow>(windowHost, type, static_cast<SDL3DisplayWindow*>(owner), renderAPI, UIScale);
}

void SDL3DisplayBackend::ProcessEvents()
{
	SDL3DisplayWindow::ProcessEvents();
}

void SDL3DisplayBackend::RunLoop()
{
	SDL3DisplayWindow::RunLoop();
}

void SDL3DisplayBackend::ExitLoop()
{
	SDL3DisplayWindow::ExitLoop();
}

Size SDL3DisplayBackend::GetScreenSize()
{
	SDL_Rect rect = {};
	SDL_DisplayID *displays = SDL_GetDisplays(nullptr);
	if (!displays || !SDL_GetDisplayBounds(displays[0], &rect))
		throw std::runtime_error(std::string("Unable to get screen size:") + SDL_GetError());
	SDL_free(displays);

	return {rect.w / UIScale, rect.h / UIScale};
}

void* SDL3DisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return SDL3DisplayWindow::StartTimer(timeoutMilliseconds, std::move(onTimer));
}

void SDL3DisplayBackend::StopTimer(void* timerID)
{
	SDL3DisplayWindow::StopTimer(timerID);
}
