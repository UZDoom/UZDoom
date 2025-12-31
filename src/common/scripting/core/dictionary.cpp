/*
** dictionary.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019 Alexander Kromm
** Copyright 2019-2025 GZDoom Maintainers and Contributors
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

#include "dictionary.h"

#include "vm.h"
#include "serializer.h"

#include <cassert>

//=====================================================================================
//
// DObject implementations for Dictionary and DictionaryIterator
//
//=====================================================================================

IMPLEMENT_CLASS(Dictionary, false, false);

IMPLEMENT_CLASS(DictionaryIterator, false, true);

IMPLEMENT_POINTERS_START(DictionaryIterator)
IMPLEMENT_POINTER(Dict)
IMPLEMENT_POINTERS_END

//=====================================================================================
//
// Dictionary functions
//
//=====================================================================================

void Dictionary::Serialize(FSerializer &arc)
{
	Super::Serialize(arc);

	static const char key[] = "dictionary";

	if (arc.isWriting())
	{
		// Pass this instance to serializer.
		Dictionary *pointerToThis = this;
		arc(key, pointerToThis);
	}
	else
	{
		// Receive new Dictionary, copy contents, clean up.
		Dictionary *pointerToDeserializedDictionary;
		arc(key, pointerToDeserializedDictionary);
		Map.TransferFrom(pointerToDeserializedDictionary->Map);
		pointerToDeserializedDictionary->Destroy();
	}
}

static Dictionary *DictCreate()
{
	Dictionary *dict { Create<Dictionary>() };

	return dict;
}

static void DictInsert(Dictionary *dict, const FString &key, const FString &value)
{
	dict->Map.Insert(key, value);
}

static void DictAt(const Dictionary *dict, const FString &key, FString *result)
{
	const FString *value = dict->Map.CheckKey(key);
	*result = value ? *value : FString();
}

static void DictToString(const Dictionary *dict, FString *result)
{
	*result = DictionaryToString(*dict);
}

static void DictRemove(Dictionary *dict, const FString &key)
{
	dict->Map.Remove(key);
}

//=====================================================================================
//
// Dictionary exports
//
//=====================================================================================

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, Create, DictCreate)
{
	ACTION_RETURN_POINTER(DictCreate());
}

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, Insert, DictInsert)
{
	PARAM_SELF_STRUCT_PROLOGUE(Dictionary);
	PARAM_STRING(key);
	PARAM_STRING(value);
	DictInsert(self, key, value);
	return 0;
}

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, At, DictAt)
{
	PARAM_SELF_STRUCT_PROLOGUE(Dictionary);
	PARAM_STRING(key);
	FString result;
	DictAt(self, key, &result);
	ACTION_RETURN_STRING(result);
}

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, ToString, DictToString)
{
	PARAM_SELF_STRUCT_PROLOGUE(Dictionary);
	FString result;
	DictToString(self, &result);
	ACTION_RETURN_STRING(result);
}

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, FromString, DictionaryFromString)
{
	PARAM_PROLOGUE;
	PARAM_STRING(string);
	ACTION_RETURN_POINTER(DictionaryFromString(string));
}

DEFINE_ACTION_FUNCTION_NATIVE(_Dictionary, Remove, DictRemove)
{
	PARAM_SELF_STRUCT_PROLOGUE(Dictionary);
	PARAM_STRING(key);
	DictRemove(self, key);
	return 0;
}

//=====================================================================================
//
// DictionaryIterator functions
//
//=====================================================================================

DictionaryIterator::DictionaryIterator()
	: Iterator(nullptr)
	, Pair(nullptr)
	, Dict(nullptr)
{
}

void DictionaryIterator::Serialize(FSerializer &arc)
{
	if (arc.isWriting())
	{
		I_Error("Attempt to save pointer to unhandled type DictionaryIterator");
	}
}

void DictionaryIterator::init(Dictionary *dict)
{
	Iterator = std::make_unique<Dictionary::ConstIterator>(dict->Map);
	Dict = dict;

	GC::WriteBarrier(this, Dict);
}

static DictionaryIterator *DictIteratorCreate(Dictionary *dict)
{
	DictionaryIterator *iterator = Create<DictionaryIterator>();
	iterator->init(dict);

	return iterator;
}

static int DictIteratorNext(DictionaryIterator *self)
{
	assert(self->Iterator != nullptr);

	const bool hasNext { self->Iterator->NextPair(self->Pair) };
	return hasNext;
}

static void DictIteratorKey(const DictionaryIterator *self, FString *result)
{
	*result = self->Pair ? self->Pair->Key : FString {};
}

static void DictIteratorValue(const DictionaryIterator *self, FString *result)
{
	*result = self->Pair ? self->Pair->Value : FString {};
}

//=====================================================================================
//
// DictionaryIterator exports
//
//=====================================================================================

DEFINE_ACTION_FUNCTION_NATIVE(_DictionaryIterator, Create, DictIteratorCreate)
{
	PARAM_PROLOGUE;
	PARAM_POINTER(dict, Dictionary);
	ACTION_RETURN_POINTER(DictIteratorCreate(dict));
}

DEFINE_ACTION_FUNCTION_NATIVE(_DictionaryIterator, Next, DictIteratorNext)
{
	PARAM_SELF_STRUCT_PROLOGUE(DictionaryIterator);
	ACTION_RETURN_BOOL(DictIteratorNext(self));
}

DEFINE_ACTION_FUNCTION_NATIVE(_DictionaryIterator, Key, DictIteratorKey)
{
	PARAM_SELF_STRUCT_PROLOGUE(DictionaryIterator);
	FString result;
	DictIteratorKey(self, &result);
	ACTION_RETURN_STRING(result);
}

DEFINE_ACTION_FUNCTION_NATIVE(_DictionaryIterator, Value, DictIteratorValue)
{
	PARAM_SELF_STRUCT_PROLOGUE(DictionaryIterator);
	FString result;
	DictIteratorValue(self, &result);
	ACTION_RETURN_STRING(result);
}
