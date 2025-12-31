/*
** chexdecorations.zs
**
** Doom items renamed with height changes
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1996-1997 Digital Café
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
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
*/

// Civilians ------------------------------------------------------------------

class ChexCivilian1 : GreenTorch
{
	Default
	{
		height 54;
	}
}

class ChexCivilian2 : ShortGreenTorch
{
	Default
	{
		height 54;
	}
}

class ChexCivilian3 : ShortRedTorch
{
	Default
	{
		height 48;
	}
}

// Landing Zone ---------------------------------------------------------------

class ChexLandingLight : Column
{
	Default
	{
		height 35;
	}
}

class ChexSpaceship : TechPillar
{
	Default
	{
		height 52;
	}
}

// Trees and Plants -----------------------------------------------------------

class ChexAppleTree : Stalagtite
{
	Default
	{
		height 92;
	}
}

class ChexBananaTree : BigTree
{
	Default
	{
		height 108;
	}
}

class ChexOrangeTree : TorchTree
{
	Default
	{
		height 92;
	}
}

class ChexSubmergedPlant : ShortGreenColumn
{
	Default
	{
		height 42;
	}
}

class ChexTallFlower : HeadsOnAStick
{
	Default
	{
		height 25;
	}
}

class ChexTallFlower2 : DeadStick
{
	Default
	{
		height 25;
	}
}

// Slime Fountain -------------------------------------------------------------

class ChexSlimeFountain : BlueTorch
{
	Default
	{
		height 48;
	}
	States
	{
	Spawn:
		TBLU ABCD 4;
		Loop;
	}
}

// Cavern Decorations ---------------------------------------------------------

class ChexCavernColumn : TallRedColumn
{
	Default
	{
		height 128;
	}
}

class ChexCavernStalagmite : TallGreenColumn
{
	Default
	{
		height 60;
	}
}

// Misc. Props ----------------------------------------------------------------

class ChexChemicalBurner : EvilEye
{
	Default
	{
		height 25;
	}
}

class ChexChemicalFlask : Candlestick
{
	Default
	{
		RenderStyle "Translucent";
		alpha 0.75;
	}
}

class ChexFlagOnPole : SkullColumn
{
	Default
	{
		height 128;
	}
}

class ChexGasTank : Candelabra
{
	Default
	{
		height 36;
	}
}

class ChexLightColumn : ShortBlueTorch
{
	Default
	{
		height 86;
	}
}

class ChexMineCart : ShortRedColumn
{
	Default
	{
		height 30;
	}
}
