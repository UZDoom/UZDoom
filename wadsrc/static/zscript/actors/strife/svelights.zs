/*
** svelights.zs
**
** This turns the helper things for creating lightmaps in SVE into
** actual light sources
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Rogue Entertainment
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class SVELight : PointLight
{
	override void BeginPlay()
	{
		Super.BeginPlay();
		if (!bSpawnCeiling) AddZ(height);
	}
}

class SVELight7958 : SVELight
{
	Default
	{
		+SPAWNCEILING
		+DYNAMICLIGHT.ATTENUATE
		Height 6;
		Args 255,255,224,128;
	}
}

class SVELight7959 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 6;
		Args 255,255,224,128;
	}
}

class SVELight7960 : SVELight
{
	Default
	{
		+SPAWNCEILING
		+DYNAMICLIGHT.ATTENUATE
		Height 6;
		Args 255,255,64,100;
	}
}

class SVELight7961 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 6;
		Args 255,255,64,100;
	}
}

class SVELight7962 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 8;
		Args 255,64,16,128;
	}
}

// 7963 has intentionally been omitted

class SVELight7964 : SVELight
{
	Default
	{
		+SPAWNCEILING
		+DYNAMICLIGHT.ATTENUATE
		Height 64;
		Args 200,200,170,160;
	}
}

class SVELight7965 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 64;
		Args 200,200,170,160;
	}
}

class SVELight7971 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 80;
		Args 248,248,224,144;
	}
}

class SVELight7972 : SVELight
{
	Default
	{
		+SPAWNCEILING
		+DYNAMICLIGHT.ATTENUATE
		Height 24;
		Args 168,175,255,128;
	}
}

class SVELight7973 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 80;
		Args 112,112,112,100;
	}
}

class SVELight7974 : SVELight
{
	Default
	{
		+DYNAMICLIGHT.ATTENUATE
		Height 80;
		Args 100,100,90,100;
	}
}



