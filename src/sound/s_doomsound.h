/*
** s_doomsound.h
**
** Information about one playing sound.
**
**---------------------------------------------------------------------------
**
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
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

#pragma once

struct sector_t;
struct FPolyObj;
struct FLevelLocals;

void S_Init();
void S_InitData();
void S_Start();
void S_Shutdown();

void S_UpdateSounds(AActor* listenactor, int tics);

void S_PrecacheLevel(FLevelLocals* l);

// Start sound for thing at <ent>
void S_Sound(int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation);
inline void S_Sound(int channel, EChanFlags flags, const char* sfxid, float volume, float attenuation)
{
	S_Sound(channel, flags, S_FindSound(sfxid), volume, attenuation);
}
inline void S_Sound(int channel, EChanFlags flags, const FString& sfxid, float volume, float attenuation)
{
	S_Sound(channel, flags, S_FindSound(sfxid), volume, attenuation);
}
void S_SoundPitch(int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation, float pitch, float startTime = 0.f);


void S_Sound (AActor *ent, int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation);
inline void S_Sound(AActor* ent, int channel, EChanFlags flags, const char* sfxid, float volume, float attenuation)
{
	S_Sound(ent, channel, flags, S_FindSound(sfxid), volume, attenuation);
}
void S_SoundMinMaxDist (AActor *ent, int channel, EChanFlags flags, FSoundID sfxid, float volume, float mindist, float maxdist);
void S_Sound (const FPolyObj *poly, int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation);
void S_Sound (const sector_t *sec, int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation);
void S_Sound(FLevelLocals *Level, const DVector3 &pos, int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation);

void S_SoundPitchActor (AActor *ent, int channel, EChanFlags flags, FSoundID sfxid, float volume, float attenuation, float pitch, float startTime = 0.f);

// [Nash] Used by ACS and DECORATE
void S_PlaySound(AActor *a, int chan, EChanFlags flags, FSoundID sid, float vol, float atten);

// Stops a sound emanating from one of an emitter's channels.
void S_StopSound (AActor *ent, int channel);
void S_StopSound (const sector_t *sec, int channel);
void S_StopSound (const FPolyObj *poly, int channel);
void S_StopActorSounds(AActor *actor, int chanmin, int chanmax);

// Moves all sounds from one mobj to another
void S_RelinkSound (AActor *from, AActor *to);

// Is the sound playing on one of the emitter's channels?
bool S_GetSoundPlayingInfo (const AActor *actor, FSoundID sound_id = INVALID_SOUND);
bool S_GetSoundPlayingInfo (const sector_t *sector, FSoundID sound_id = INVALID_SOUND);
bool S_GetSoundPlayingInfo (const FPolyObj *poly, FSoundID sound_id = INVALID_SOUND);

bool S_IsActorPlayingSomething (AActor *actor, int channel, FSoundID sound_id = INVALID_SOUND);

// Change a playing sound's volume
void S_ChangeActorSoundVolume(AActor *actor, int channel, double volume);

// Change a playing sound's pitch
void S_ChangeActorSoundPitch(AActor *actor, int channel, double pitch);

// Stores/retrieves playing channel information in an archive.
void S_SerializeSounds(FSerializer &arc);

// these must retain their integer sound IDs because they are direct native functions for ZScript.
void A_PlaySound(AActor *self, int soundid, int channel, double volume, int looping, double attenuation, int local, double pitch);
void A_StartSound(AActor* self, int soundid, int channel, int flags, double volume, double attenuation,  double pitch, double startTime = 0.);
static void S_SetListener(AActor *listenactor);
void S_SoundReset();
void S_ResumeSound(bool state);
void S_PauseSound(bool state1, bool state);
void S_NoiseDebug();

inline void S_StopSound(int chan)
{
	soundEngine->StopSound(chan);
}

inline void S_StopAllChannels()
{
	soundEngine->StopAllChannels();
}

inline const char* S_GetSoundName(FSoundID id)
{
	return soundEngine->GetSoundName(id);
}

