#pragma once

struct SDL_Window;

class SDLNativeHandle
{
public:
	SDL_Window* window = nullptr;
};
