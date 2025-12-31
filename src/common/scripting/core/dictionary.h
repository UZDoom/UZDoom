/*
** dictionary.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Alexander Kromm
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

#include "tarray.h"
#include "zstring.h"
#include "dobject.h"

#include <memory>

/**
 * @brief The Dictionary class exists to be exported to ZScript.
 *
 * It is a string-to-string map.
 *
 * It is derived from DObject to be a part of normal GC process.
 */
class Dictionary final : public DObject
{
	DECLARE_CLASS(Dictionary, DObject)

public:

	using StringMap = TMap<FString, FString>;
	using ConstIterator = StringMap::ConstIterator;
	using ConstPair = StringMap::ConstPair;

	void Serialize(FSerializer &arc) override;

	StringMap Map;
};

/**
 * @brief The DictionaryIterator class exists to be exported to ZScript.
 *
 * It provides iterating over a Dictionary. The order is not specified.
 *
 * It is derived from DObject to be a part of normal GC process.
 */
class DictionaryIterator final : public DObject
{
	DECLARE_CLASS(DictionaryIterator, DObject)
	HAS_OBJECT_POINTERS

public:

	~DictionaryIterator() override = default;

	/**
	 * IMPLEMENT_CLASS macro needs a constructor without parameters.
	 *
	 * @see init().
	 */
	DictionaryIterator();

	void Serialize(FSerializer &arc) override;

	/**
	 * @brief init function complements constructor.
	 * @attention always call init after constructing DictionaryIterator.
	 */
	void init(Dictionary *dict);

	std::unique_ptr<Dictionary::ConstIterator> Iterator;
	Dictionary::ConstPair *Pair;

	/**
	 * @brief Dictionary attribute exists for holding a pointer to iterated
	 * dictionary, so it is known by GC.
	 */
	Dictionary *Dict;
};
