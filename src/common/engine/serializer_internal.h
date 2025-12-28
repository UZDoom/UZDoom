/*
** serializer_internal.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christoph Oelckers
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
const char* UnicodeToString(const char* cc);
const char* StringToUnicode(const char* cc, int size = -1);

//==========================================================================
//
//
//
//==========================================================================

struct FJSONObject
{
	rapidjson::Value* mObject;
	rapidjson::Value::MemberIterator mIterator;
	int mIndex;

	FJSONObject(rapidjson::Value* v)
	{
		mObject = v;
		if (v->IsObject()) mIterator = v->MemberBegin();
		else if (v->IsArray())
		{
			mIndex = 0;
		}
	}
};

//==========================================================================
//
// some wrapper stuff to keep the RapidJSON dependencies out of the global headers.
// FSerializer should not expose any of this, although it is needed by special serializers.
//
//==========================================================================

struct FWriter
{
	typedef rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<> > Writer;
	typedef rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<> > PrettyWriter;

	Writer *mWriter1;
	PrettyWriter *mWriter2;
	TArray<bool> mInObject;
	rapidjson::StringBuffer mOutString;
	TArray<DObject *> mDObjects;
	TMap<DObject *, int> mObjectMap;

	FWriter(bool pretty)
	{
		if (!pretty)
		{
			mWriter1 = new Writer(mOutString);
			mWriter2 = nullptr;
		}
		else
		{
			mWriter1 = nullptr;
			mWriter2 = new PrettyWriter(mOutString);
		}
	}

	~FWriter()
	{
		if (mWriter1) delete mWriter1;
		if (mWriter2) delete mWriter2;
	}


	bool inObject() const
	{
		return mInObject.Size() > 0 && mInObject.Last();
	}

	void StartObject()
	{
		if (mWriter1) mWriter1->StartObject();
		else if (mWriter2) mWriter2->StartObject();
	}

	void EndObject()
	{
		if (mWriter1) mWriter1->EndObject();
		else if (mWriter2) mWriter2->EndObject();
	}

	void StartArray()
	{
		if (mWriter1) mWriter1->StartArray();
		else if (mWriter2) mWriter2->StartArray();
	}

	void EndArray()
	{
		if (mWriter1) mWriter1->EndArray();
		else if (mWriter2) mWriter2->EndArray();
	}

	void Key(const char *k)
	{
		if (mWriter1) mWriter1->Key(k);
		else if (mWriter2) mWriter2->Key(k);
	}

	void Null()
	{
		if (mWriter1) mWriter1->Null();
		else if (mWriter2) mWriter2->Null();
	}

	void StringU(const char *k, bool encode)
	{
		if (encode) k = StringToUnicode(k);
		if (mWriter1) mWriter1->String(k);
		else if (mWriter2) mWriter2->String(k);
	}

	void String(const char *k)
	{
		k = StringToUnicode(k);
		if (mWriter1) mWriter1->String(k);
		else if (mWriter2) mWriter2->String(k);
	}

	void String(const char *k, int size)
	{
		k = StringToUnicode(k, size);
		if (mWriter1) mWriter1->String(k);
		else if (mWriter2) mWriter2->String(k);
	}

	void Bool(bool k)
	{
		if (mWriter1) mWriter1->Bool(k);
		else if (mWriter2) mWriter2->Bool(k);
	}

	void Int(int32_t k)
	{
		if (mWriter1) mWriter1->Int(k);
		else if (mWriter2) mWriter2->Int(k);
	}

	void Int64(int64_t k)
	{
		if (mWriter1) mWriter1->Int64(k);
		else if (mWriter2) mWriter2->Int64(k);
	}

	void Uint(uint32_t k)
	{
		if (mWriter1) mWriter1->Uint(k);
		else if (mWriter2) mWriter2->Uint(k);
	}

	void Uint64(int64_t k)
	{
		if (mWriter1) mWriter1->Uint64(k);
		else if (mWriter2) mWriter2->Uint64(k);
	}

	void Double(double k)
	{
		if (mWriter1)
		{
			mWriter1->Double(k);
		}
		else if (mWriter2)
		{
			mWriter2->Double(k);
		}
	}

};

//==========================================================================
//
//
//
//==========================================================================

struct FReader
{
	TArray<FJSONObject> mObjects;
	rapidjson::Document mDoc;
	TArray<DObject *> mDObjects;
	rapidjson::Value *mKeyValue = nullptr;
	bool mObjectsRead = false;

	FReader(const char *buffer, size_t length)
	{
		mDoc.Parse(buffer, length);
		mObjects.Push(FJSONObject(&mDoc));
	}

	rapidjson::Value *FindKey(const char *key)
	{
		FJSONObject &obj = mObjects.Last();

		if (obj.mObject->IsObject())
		{
			if (key == nullptr)
			{
				// we are performing an iteration of the object through GetKey.
				auto p = mKeyValue;
				mKeyValue = nullptr;
				return p;
			}
			else
			{
				// Find the given key by name;
				auto it = obj.mObject->FindMember(key);
				if (it == obj.mObject->MemberEnd()) return nullptr;
				return &it->value;
			}
		}
		else if (obj.mObject->IsArray() && (unsigned)obj.mIndex < obj.mObject->Size())
		{
			return &(*obj.mObject)[obj.mIndex++];
		}
		return nullptr;
	}
};

//==========================================================================
//
//
//
//==========================================================================

template<class T>
FSerializer &SerializePointer(FSerializer &arc, const char *key, T *&value, T **defval, T *base, const int64_t count)
{
	assert(base != nullptr);
	assert(count > 0);
	if (arc.isReading() || !arc.w->inObject() || defval == nullptr || value != *defval)
	{
		int64_t vv = -1;
		if (value != nullptr)
		{
			vv = value - base;
			if (vv < 0 || vv >= count)
			{
				Printf("Trying to serialize out-of-bounds array value with key '%s', index = %" PRId64 ", size = %" PRId64 "\n", key, vv, count);
				vv = -1;
			}
		}
		Serialize(arc, key, vv, nullptr);
		if (vv == -1)
			value = nullptr;
		else if (vv < 0 || vv >= count)
		{
			Printf("Trying to serialize out-of-bounds array value with key '%s', index = %" PRId64 ", size = %" PRId64 "\n", key, vv, count);
			value = nullptr;
		}
		else
			value = base + vv;
	}
	return arc;
}

template<class T>
FSerializer &SerializePointer(FSerializer &arc, const char *key, T *&value, T **defval, TArray<T> &array)
{
	if (array.Size() == 0)
	{
		Printf("Trying to serialize a value with key '%s' from empty array\n", key);
		return arc;
	}
	return SerializePointer(arc, key, value, defval, array.Data(), array.Size());
}

