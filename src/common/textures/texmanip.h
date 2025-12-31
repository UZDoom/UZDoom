/*
** texmanip.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christoph Oelckers
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

#include "palentry.h"

struct TextureManipulation
{
	enum
	{
		BlendNone = 0,
		BlendAlpha = 1,
		BlendScreen = 2,
		BlendOverlay = 3,
		BlendHardLight = 4,
		BlendMask = 7,
		InvertBit = 8,
		ActiveBit = 16,	// Must be set for the shader to do something
	};
	PalEntry AddColor;		// Alpha contains the blend flags
	PalEntry ModulateColor;	// Alpha may contain a multiplier to get higher values than 1.0 without promoting this to 4 full floats.
	PalEntry BlendColor;
	float DesaturationFactor;

	bool CheckIfEnabled()	// check if this manipulation is doing something. NoOps do not need to be preserved, unless they override older setttings.
	{
		if (AddColor != 0 ||	// this includes a check for the blend mode without which BlendColor is not active
			ModulateColor != 0x01ffffff ||	// 1 in alpha must be the default for a no-op.
			DesaturationFactor != 0)
		{
			AddColor.a |= ActiveBit; // mark as active for the shader's benefit.
			return true;
		}
		return false;
	}

	void SetTextureModulateColor(int slot, PalEntry rgb)
	{
		rgb.a = ModulateColor.a;
		ModulateColor = rgb;
	}

	void SetTextureModulateScaleFactor(int slot, int fac)
	{
		ModulateColor.a = (uint8_t)fac;
	}

	void SetTextureAdditiveColor(int slot, PalEntry rgb)
	{
		rgb.a = AddColor.a;
		AddColor = rgb;
	}

	void SetTextureBlendColor(int slot, PalEntry rgb)
	{
		BlendColor = rgb;
	}

	void SetTextureDesaturationFactor(int slot, double fac)
	{
		DesaturationFactor = (float)fac;
	}

	void SetTextureBlendMode(int slot, int mode)
	{
		AddColor.a = (AddColor.a & ~TextureManipulation::BlendMask) | (mode & TextureManipulation::BlendMask);
	}

	void SetTextureInvert(bool on)
	{
		AddColor.a |= TextureManipulation::InvertBit;
		AddColor.a &= ~TextureManipulation::InvertBit;
	}

};


