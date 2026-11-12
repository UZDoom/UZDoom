/*
** serializer_internal.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
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
	rapidjson::Value::MemberIterator mHopefulIterator;
	int mIndex;

	FJSONObject(rapidjson::Value* v)
	{
		mObject = v;
		if (v->IsObject()) {
			mIterator = v->MemberBegin();
			mHopefulIterator = v->MemberBegin();
		}
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

	Writer *mWriter;
	TArray<bool> mInObject;
	rapidjson::StringBuffer mOutString;
	TArray<DObject *> mDObjects;
	TMap<DObject *, int> mObjectMap;

	FWriter(bool pretty, FWriterBuffer existingBuffer)
	{
		existingBuffer.buffer.Clear();
		mOutString = std::move(existingBuffer.buffer);
		if (!pretty)
		{
			mWriter = new Writer(mOutString);
		}
		else
		{
			mWriter = new PrettyWriter(mOutString);
		}
	}
	FWriter(bool pretty) : FWriter(pretty, FWriterBuffer(rapidjson::StringBuffer {})) {}

	~FWriter()
	{
		if (mWriter) delete mWriter;
	}

	FWriterBuffer MoveBufferOut() {
		return FWriterBuffer(std::move(mOutString));
	}


	bool inObject() const
	{
		return mInObject.Size() > 0 && mInObject.Last();
	}

	void StartObject()
	{
		mWriter->StartObject();
	}

	void EndObject()
	{
		mWriter->EndObject();
	}

	void StartArray()
	{
		mWriter->StartArray();
	}

	void EndArray()
	{
		mWriter->EndArray();
	}

	void Key(const char *k)
	{
		mWriter->Key(k);
	}

	void Null()
	{
		mWriter->Null();
	}

	template<bool encode>
	void StringU(const char *k)
	{
		if constexpr (encode) k = StringToUnicode(k);
		mWriter->String(k);
	}

	void String(const char *k)
	{
		k = StringToUnicode(k);
		mWriter->String(k);
	}

	void String(const char *k, int size)
	{
		k = StringToUnicode(k, size);
		mWriter->String(k);
	}

	void Bool(bool k)
	{
		mWriter->Bool(k);
	}

	void Int(int32_t k)
	{
		mWriter->Int(k);
	}

	void Int64(int64_t k)
	{
		mWriter->Int64(k);
	}

	void Uint(uint32_t k)
	{
		mWriter->Uint(k);
	}

	void Uint64(int64_t k)
	{
		mWriter->Uint64(k);
	}

	void Double(double k)
	{
		mWriter->Double(k);
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

	FReader(FReaderAllocator allocator, const char *buffer, size_t length) : mDoc(rapidjson::Document(&allocator.buffer))
	{
		mDoc.Parse(buffer, length);
		mObjects.Push(FJSONObject(&mDoc));
	}

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
				while (obj.mHopefulIterator != obj.mObject->MemberEnd()) [[likely]] {
					auto name = std::string_view(obj.mHopefulIterator->name.GetString(), obj.mHopefulIterator->name.GetStringLength());
					if (key == name) [[likely]] {
						auto ret = &obj.mHopefulIterator->value;
						++obj.mHopefulIterator;
						return ret;
					}
					if (name.starts_with("class:")) [[unlikely]] {
						++obj.mHopefulIterator;
						continue;
					}
					break;
				}
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
	if (!arc.canSkip() || defval == nullptr || value != *defval)
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
