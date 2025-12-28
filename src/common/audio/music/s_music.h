/*
** s_music.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2020-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __S_MUSIC__
#define __S_MUSIC__

#include "zstring.h"
#include "tarray.h"
#include "name.h"
#include "files.h"
#include <zmusic.h>

class SoundStream;


enum MusicCustomStreamType : bool {
	MusicSamples16bit,
	MusicSamplesFloat
};
int MusicEnabled();
typedef bool(*StreamCallback)(SoundStream* stream, void* buff, int len, void* userdata);
SoundStream *S_CreateCustomStream(size_t size, int samplerate, int numchannels, MusicCustomStreamType sampletype, StreamCallback cb, void *userdata);
void S_StopCustomStream(SoundStream* stream);
void S_PauseAllCustomStreams(bool on);

struct MusicCallbacks
{
	FString(*LookupFileName)(const char* fn, int &order);
	int(*FindMusic)(const char* fn);
};
void S_SetMusicCallbacks(MusicCallbacks* cb);

void S_CreateStream();
void S_PauseStream(bool pause);
void S_StopStream();
void S_SetStreamVolume(float vol);


//
void S_InitMusic ();
void S_ResetMusic ();


// Start music using <music_name>
bool S_StartMusic (const char *music_name);

// Start music using <music_name>, and set whether looping
bool S_ChangeMusic (const char *music_name, int order=0, bool looping=true, bool force=false);

// Check if <music_name> exists
bool MusicExists(const char* music_name);

void S_RestartMusic ();
void S_MIDIDeviceChanged(int newdev);

int S_GetMusic (const char **name);

// Stops the music for sure.
void S_StopMusic (bool force);

// Stop and resume music, during game PAUSE.
void S_PauseMusic ();
void S_ResumeMusic ();

//
// Updates music & sounds
//
void S_UpdateMusic ();

struct MidiDeviceSetting
{
	int device;
	FString args;
};

typedef TMap<int, MidiDeviceSetting> MidiDeviceMap;
typedef TMap<int, float> MusicVolumeMap;

extern TMap<int, int> ModPlayers;
extern MidiDeviceMap MidiDevices;
extern MusicVolumeMap MusicVolumes;
extern MusicCallbacks mus_cb;

struct MusPlayingInfo
{
	FString name;
	ZMusic_MusicStream handle;
	int   lumpnum;
	int   baseorder;
	float musicVolume;
	bool  loop;
	bool isfloat;
	FString	 LastSong;			// last music that was played
	FString hash;				// for setting replay gain while playing.
};

extern MusPlayingInfo mus_playing;

extern float relative_volume, saved_relative_volume;


#endif
