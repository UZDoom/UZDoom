/*
** i_gui.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2008-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#include <string.h>

#include <SDL2/SDL.h>

#include "bitmap.h"
#include "textures.h"

bool I_SetCursor(FGameTexture *cursorpic)
{
	static SDL_Cursor *cursor;
	static SDL_Surface *cursorSurface;

	if (cursorpic != NULL && cursorpic->isValid())
	{
		auto src = cursorpic->GetTexture()->GetBgraBitmap(nullptr);
		// Must be no larger than 32x32.
		if (src.GetWidth() > 32 || src.GetHeight() > 32)
		{
			return false;
		}

		SDL_ShowCursor(SDL_DISABLE);
		if (cursorSurface == NULL)
			cursorSurface = SDL_CreateRGBSurface (0, 32, 32, 32, MAKEARGB(0,255,0,0), MAKEARGB(0,0,255,0), MAKEARGB(0,0,0,255), MAKEARGB(255,0,0,0));

		SDL_LockSurface(cursorSurface);
		uint8_t buffer[32*32*4];
		memset(buffer, 0, 32*32*4);
		FBitmap bmp(buffer, 32*4, 32, 32);
		bmp.Blit(0, 0, src);	// expand to 32*32
		memcpy(cursorSurface->pixels, bmp.GetPixels(), 32*32*4);
		SDL_UnlockSurface(cursorSurface);

		if (cursor)
			SDL_FreeCursor (cursor);
		cursor = SDL_CreateColorCursor (cursorSurface, 0, 0);
		SDL_SetCursor (cursor);
		SDL_ShowCursor(SDL_ENABLE);
	}
	else
	{
		if (cursor)
		{
			SDL_SetCursor (NULL);
			SDL_FreeCursor (cursor);
			cursor = NULL;
		}
		if (cursorSurface != NULL)
		{
			SDL_FreeSurface(cursorSurface);
			cursorSurface = NULL;
		}
	}
	return true;
}
