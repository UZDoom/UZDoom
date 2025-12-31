/*
** animations.h
**
** Routines for handling texture animation.
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#include <stdint.h>
#include "name.h"
#include "textureid.h"
#include "tarray.h"
#include "s_soundinternal.h"
#include "firetexture.h"

struct FStandaloneAnimation
{
	double		SwitchTic;
	uint32_t	AnimIndex;
	uint16_t	CurFrame;
	bool		ok = false;
	uint8_t		AnimType;
};

static_assert(sizeof(FStandaloneAnimation) == sizeof(uint64_t)*2);

struct FFireTexture
{
	uint32_t Duration; // Duration before updates.
	uint64_t SwitchTime; // Absolute time before update.
	FGameTexture* texture;
};

struct FAnimDef
{
	struct FAnimFrame
	{
		uint32_t	SpeedMin;		// Speeds are in ms, not tics
		uint32_t	SpeedRange;
		FTextureID	FramePic;
	};

	FTextureID 	BasePic;
	uint16_t	NumFrames;
	uint16_t	CurFrame;
	uint8_t	AnimType;
	bool	bDiscrete;			// taken out of AnimType to have better control
	uint64_t	SwitchTime;			// Time to advance to next frame
	FAnimFrame* Frames;
	enum
	{
		ANIM_Forward,
		ANIM_Backward,
		ANIM_OscillateUp,
		ANIM_OscillateDown,
		ANIM_Random
	};

	void SetSwitchTime(uint64_t mstime);
};

struct FSwitchDef
{
	FTextureID PreTexture;		// texture to switch from
	FSwitchDef* PairDef;		// switch def to use to return to PreTexture
	uint16_t NumFrames;		// # of animation frames
	bool QuestPanel;	// Special texture for Strife mission
	FSoundID Sound;			// sound to play at start of animation. Changed to int to avoiud having to include s_sound here.
	struct frame		// Array of times followed by array of textures
	{					//   actual length of each array is <NumFrames>
		uint16_t TimeMin;
		uint16_t TimeRnd;
		FTextureID Texture;
	} frames[1];
};

struct FDoorAnimation
{
	FTextureID BaseTexture;
	FTextureID* TextureFrames;
	int NumTextureFrames;
	FName OpenSound;
	FName CloseSound;
};



class FTextureAnimator
{
	TMap<FTextureID, uint16_t> mAnimationIndices;
	TArray<FAnimDef> mAnimations;
	TArray<FSwitchDef*> mSwitchDefs;
	TArray<FDoorAnimation> mAnimatedDoors;
	TArray<FFireTexture> mFireTextures;

	void ParseAnim(FScanner& sc, ETextureType usetype);
	FAnimDef* ParseRangeAnim(FScanner& sc, FTextureID picnum, ETextureType usetype, bool missing);
	void ParsePicAnim(FScanner& sc, FTextureID picnum, ETextureType usetype, bool missing, TArray<FAnimDef::FAnimFrame>& frames);
	void ParseWarp(FScanner& sc);
	void ParseCanvasTexture(FScanner& sc);
	void ParseCameraTexture(FScanner& sc);
	void ParseFireTexture(FScanner& sc);
	FTextureID ParseFramenum(FScanner& sc, FTextureID basepicnum, ETextureType usetype, bool allowMissing);
	void ParseTime(FScanner& sc, uint32_t& min, uint32_t& max);

	void FixAnimations();
	void InitAnimated();
	void InitAnimDefs();
	void InitSwitchList();
	void ProcessSwitchDef(FScanner& sc);
	FSwitchDef* ParseSwitchDef(FScanner& sc, bool ignoreBad);
	void AddSwitchPair(FSwitchDef* def1, FSwitchDef* def2);
	void ParseAnimatedDoor(FScanner& sc);

public:

	~FTextureAnimator()
	{
		DeleteAll();
	}

	// Animation stuff
	FAnimDef* AddAnim(FAnimDef& anim);
	void DeleteAll();

	FAnimDef* AddSimpleAnim(FTextureID picnum, int animcount, uint32_t speedmin, uint32_t speedrange = 0);
	FAnimDef* AddComplexAnim(FTextureID picnum, const TArray<FAnimDef::FAnimFrame>& frames);

	FSwitchDef* FindSwitch(FTextureID texture);
	FDoorAnimation* FindAnimatedDoor(FTextureID picnum);
	void UpdateAnimations(uint64_t mstime);

	const TArray<FAnimDef>& GetAnimations() const { return mAnimations; }

	void Init()
	{
		DeleteAll();
		InitAnimated();
		InitAnimDefs();
		FixAnimations();
		InitSwitchList();
	}

	bool InitStandaloneAnimation(FStandaloneAnimation &animInfo, FTextureID tex, uint32_t curTic);
	FTextureID UpdateStandaloneAnimation(FStandaloneAnimation &animInfo, double curTic);
	void ResetTimers();
};

extern FTextureAnimator TexAnim;


