/*
** textureid.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2004-2016 Marisa Heit
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

#include <cstddef>
#include "tarray.h"

enum class ETextureType : uint8_t
{
	Any,
	Wall,
	Flat,
	Sprite,
	WallPatch,
	Build,		// no longer used but needs to remain for ZScript
	SkinSprite,
	Decal,
	MiscPatch,
	FontChar,
	Override,	// For patches between TX_START/TX_END
	Autopage,	// Automap background - used to enable the use of FAutomapTexture
	SkinGraphic,
	Null,
	FirstDefined,
	Special,
	SWCanvas,
};

class FTextureID
{
	friend class FTextureManager;
	friend void R_InitSpriteDefs();

public:
	FTextureID() = default;
	FTextureID(std::nullptr_t) : texnum(0) {}
	bool isNull() const { return texnum == 0; }
	bool isValid() const { return texnum > 0; }
	bool Exists() const { return texnum >= 0; }
	void SetInvalid() { texnum = -1; }
	void SetNull() { texnum = 0; }
	bool operator ==(const FTextureID &other) const { return texnum == other.texnum; }
	bool operator !=(const FTextureID &other) const { return texnum != other.texnum; }
	FTextureID operator +(int offset) const noexcept(true);
	int GetIndex() const { return texnum; }	// Use this only if you absolutely need the index!
	void SetIndex(int index) { texnum = index; }	// Use this only if you absolutely need the index!

											// The switch list needs these to sort the switches by texture index
	int operator -(FTextureID other) const { return texnum - other.texnum; }
	bool operator < (FTextureID other) const { return texnum < other.texnum; }
	bool operator > (FTextureID other) const { return texnum > other.texnum; }

protected:
	constexpr FTextureID(int num) : texnum(num) { }
private:
	int texnum;
};

class FNullTextureID : public FTextureID
{
public:
	constexpr FNullTextureID() : FTextureID(0) {}
};

// This is for the script interface which needs to do casts from int to texture.
class FSetTextureID : public FTextureID
{
public:
	constexpr FSetTextureID(int v) : FTextureID(v) {}
};

template<> struct THashTraits<FTextureID>
{

	hash_t Hash(const FTextureID key) { return (hash_t)key.GetIndex(); }

	// Compares two keys, returning zero if they are the same.
	int Compare(const FTextureID left, const FTextureID right) { return left != right; }
};
