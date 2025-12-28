/*
** r_swcolormaps.h
**
** Colormap handling for the software renderer
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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

#pragma once


#include "g_levellocals.h"

struct FSWColormap
{
	uint8_t *Maps = nullptr;
	PalEntry Color = 0xffffffff;
	PalEntry Fade = 0xff000000;
	int Desaturate = 0;
};

struct FDynamicColormap : FSWColormap
{
	void ChangeFade (PalEntry fadecolor);
	void ChangeColor (PalEntry lightcolor, int desaturate);
	void ChangeColorFade (PalEntry lightcolor, PalEntry fadecolor);
	void ChangeFogDensity(int newdensity);
	void BuildLights ();
	static void RebuildAllLights();

	FDynamicColormap *Next;
};

extern FSWColormap realcolormaps;					// [RH] make the colormaps externally visible

extern FDynamicColormap NormalLight;
extern FDynamicColormap FullNormalLight;
extern bool NormalLightHasFixedLights;
extern TArray<FSWColormap> SpecialSWColormaps;

void DeinitSWColorMaps();
void InitSWColorMaps();
FDynamicColormap *GetSpecialLights (PalEntry lightcolor, PalEntry fadecolor, int desaturate);
void SetDefaultColormap (const char *name);


// Give the compiler a strong hint we want these functions inlined:
#ifndef FORCEINLINE
#if defined(_MSC_VER)
#define FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#else
#define FORCEINLINE inline
#endif
#endif

// MSVC needs the forceinline here.
FORCEINLINE FDynamicColormap *GetColorTable(const FColormap &cm, PalEntry SpecialColor = 0xffffff)
{
	PalEntry c =  SpecialColor.Modulate(cm.LightColor);

	// First colormap is the default Doom colormap.
	// testcolor and testfade CCMDs modifies the first colormap, so we have to do the check without looking at the actual values stored in NormalLight.
	if (c == PalEntry(255, 255, 255) && cm.FadeColor == 0 && cm.Desaturation == 0)
		return &NormalLight;

	return GetSpecialLights(c, cm.FadeColor, cm.Desaturation);
}

FORCEINLINE FDynamicColormap *GetSpriteColorTable(const FColormap &cm, PalEntry SpecialColor, bool nocoloredspritelighting)
{
	PalEntry c;
	if (!nocoloredspritelighting) c =  SpecialColor.Modulate(cm.LightColor);
	else
	{
		c = cm.LightColor;
		c.Decolorize();
		c = SpecialColor.Modulate(c);
	}
	
	// First colormap is the default Doom colormap.
	// testcolor and testfade CCMDs modifies the first colormap, so we have to do the check without looking at the actual values stored in NormalLight.
	if (c == PalEntry(255, 255, 255) && cm.FadeColor == 0 && cm.Desaturation == 0)
		return &NormalLight;
	
	return GetSpecialLights(c, cm.FadeColor, cm.Desaturation);
}
