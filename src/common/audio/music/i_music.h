/*
** i_music.h
**
**
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

#ifndef __I_MUSIC_H__
#define __I_MUSIC_H__

//
//	MUSIC I/O
//
void I_InitMusic (int);

// Volume.
void I_SetRelativeVolume(float);
void I_SetMusicVolume (double volume);



inline float AmplitudeTodB(float amplitude)
{
	return 20.0f * log10f(amplitude);
}

inline float dBToAmplitude(float dB)
{
	return powf(10.0f, dB / 20.0f);
}

#endif //__I_MUSIC_H__
