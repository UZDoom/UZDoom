/*
** s_playlist.h
**
** FPlayList class. Handles m3u playlist parsing
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __S_PLAYLIST_H__
#define __S_PLAYLIST_H__

#include "files.h"

class FPlayList
{
public:
	~FPlayList ();

	bool ChangeList (const char *path);

	int GetNumSongs () const;
	int SetPosition (int position);
	int GetPosition () const;
	int Advance ();
	int Backup ();
	void Shuffle ();
	const char *GetSong (int position) const;
	void Clear()
	{
		Songs.Clear();
		Position = 0;
	}

private:
	static FString NextLine (FileReader &file);

	unsigned int Position = 0;
	TArray<FString> Songs;
};

#endif //__S_PLAYLIST_H__
