/*
** maps.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
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

struct Map_I32_I8 native unsafe(internal)
{
    native void Copy(Map_I32_I8 other);
    native void Move(Map_I32_I8 other);
    native void Swap(Map_I32_I8 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") int GetIfExists(int key) const;
    native version("4.11") int, bool CheckValue(int key) const;
    
    native void Insert(int key, int value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_I8 native unsafe(internal)
{
    native bool Init(Map_I32_I8 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_I32_I16 native unsafe(internal)
{
    native void Copy(Map_I32_I16 other);
    native void Move(Map_I32_I16 other);
    native void Swap(Map_I32_I16 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") int GetIfExists(int key) const;
    native version("4.11") int, bool CheckValue(int key) const;
    
    native void Insert(int key, int value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_I16 native unsafe(internal)
{
    native bool Init(Map_I32_I16 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_I32_I32 native unsafe(internal)
{
    native void Copy(Map_I32_I32 other);
    native void Move(Map_I32_I32 other);
    native void Swap(Map_I32_I32 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") int GetIfExists(int key) const;
    native version("4.11") int, bool CheckValue(int key) const;
    
    native void Insert(int key, int value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_I32 native unsafe(internal)
{
    native bool Init(Map_I32_I32 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_I32_F32 native unsafe(internal)
{
    native void Copy(Map_I32_F32 other);
    native void Move(Map_I32_F32 other);
    native void Swap(Map_I32_F32 other);
    native void Clear();
    native uint CountUsed() const;

    native double Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") double GetIfExists(int key) const;
    native version("4.11") double, bool CheckValue(int key) const;
    
    native void Insert(int key, double value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_F32 native unsafe(internal)
{
    native bool Init(Map_I32_F32 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native double GetValue();
    native void SetValue(double value);
}

struct Map_I32_F64 native unsafe(internal)
{
    native void Copy(Map_I32_F64 other);
    native void Move(Map_I32_F64 other);
    native void Swap(Map_I32_F64 other);
    native void Clear();
    native uint CountUsed() const;

    native double Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") double GetIfExists(int key) const;
    native version("4.11") double, bool CheckValue(int key) const;
    
    native void Insert(int key, double value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_F64 native unsafe(internal)
{
    native bool Init(Map_I32_F64 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native double GetValue();
    native void SetValue(double value);
}

struct Map_I32_Obj native unsafe(internal)
{
    native void Copy(Map_I32_Obj other);
    native void Move(Map_I32_Obj other);
    native void Swap(Map_I32_Obj other);
    native void Clear();
    native uint CountUsed() const;

    native Object Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") Object GetIfExists(int key) const;
    native version("4.11") Object, bool CheckValue(int key) const;
    
    native void Insert(int key, Object value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_Obj native unsafe(internal)
{
    native bool Init(Map_I32_Obj other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native Object GetValue();
    native void SetValue(Object value);
}

struct Map_I32_Ptr native unsafe(internal)
{
    native void Copy(Map_I32_Ptr other);
    native void Move(Map_I32_Ptr other);
    native void Swap(Map_I32_Ptr other);
    native void Clear();
    native uint CountUsed() const;

    native voidptr Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") voidptr GetIfExists(int key) const;
    native version("4.11") voidptr, bool CheckValue(int key) const;
    
    native void Insert(int key, voidptr value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_Ptr native unsafe(internal)
{
    native bool Init(Map_I32_Ptr other);
    native bool Next();
    
    native int GetKey();
    native voidptr GetValue();
    native void SetValue(voidptr value);
}

struct Map_I32_Str native unsafe(internal)
{
    native void Copy(Map_I32_Str other);
    native void Move(Map_I32_Str other);
    native void Swap(Map_I32_Str other);
    native void Clear();
    native uint CountUsed() const;

    native String Get(int key);
    native bool CheckKey(int key) const;
    
    native version("4.11") String GetIfExists(int key) const;
    native version("4.11") String, bool CheckValue(int key) const;
    
    native void Insert(int key, String value);
    native void InsertNew(int key);
    native void Remove(int key);
}

struct MapIterator_I32_Str native unsafe(internal)
{
    native bool Init(Map_I32_Str other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native int GetKey();
    native String GetValue();
    native void SetValue(String value);
}

// ---------------

struct Map_Str_I8 native unsafe(internal)
{
    native void Copy(Map_Str_I8 other);
    native void Move(Map_Str_I8 other);
    native void Swap(Map_Str_I8 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") int GetIfExists(String key) const;
    native version("4.11") int, bool CheckValue(String key) const;
    
    native void Insert(String key, int value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_I8 native unsafe(internal)
{
    native bool Init(Map_Str_I8 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_Str_I16 native unsafe(internal)
{
    native void Copy(Map_Str_I16 other);
    native void Move(Map_Str_I16 other);
    native void Swap(Map_Str_I16 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") int GetIfExists(String key) const;
    native version("4.11") int, bool CheckValue(String key) const;
    
    native void Insert(String key, int value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_I16 native unsafe(internal)
{
    native bool Init(Map_Str_I16 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_Str_I32 native unsafe(internal)
{
    native void Copy(Map_Str_I32 other);
    native void Move(Map_Str_I32 other);
    native void Swap(Map_Str_I32 other);
    native void Clear();
    native uint CountUsed() const;

    native int Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") int GetIfExists(String key) const;
    native version("4.11") int, bool CheckValue(String key) const;
    
    native void Insert(String key, int value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_I32 native unsafe(internal)
{
    native bool Init(Map_Str_I32 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native int GetValue();
    native void SetValue(int value);
}

struct Map_Str_F32 native unsafe(internal)
{
    native void Copy(Map_Str_F32 other);
    native void Move(Map_Str_F32 other);
    native void Swap(Map_Str_F32 other);
    native void Clear();
    native uint CountUsed() const;

    native double Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") double GetIfExists(String key) const;
    native version("4.11") double, bool CheckValue(String key) const;
    
    native void Insert(String key, double value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_F32 native unsafe(internal)
{
    native bool Init(Map_Str_F32 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native double GetValue();
    native void SetValue(double value);
}

struct Map_Str_F64 native unsafe(internal)
{
    native void Copy(Map_Str_F64 other);
    native void Move(Map_Str_F64 other);
    native void Swap(Map_Str_F64 other);
    native void Clear();
    native uint CountUsed() const;

    native double Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") double GetIfExists(String key) const;
    native version("4.11") double, bool CheckValue(String key) const;
    
    native void Insert(String key, double value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_F64 native unsafe(internal)
{
    native bool Init(Map_Str_F64 other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native double GetValue();
    native void SetValue(double value);
}

struct Map_Str_Obj native unsafe(internal)
{
    native void Copy(Map_Str_Obj other);
    native void Move(Map_Str_Obj other);
    native void Swap(Map_Str_Obj other);
    native void Clear();
    native uint CountUsed() const;

    native Object Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") Object GetIfExists(String key) const;
    native version("4.11") Object, bool CheckValue(String key) const;
    
    native void Insert(String key, Object value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_Obj native unsafe(internal)
{
    native bool Init(Map_Str_Obj other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native Object GetValue();
    native void SetValue(Object value);
}

struct Map_Str_Ptr native unsafe(internal)
{
    native void Copy(Map_Str_Ptr other);
    native void Move(Map_Str_Ptr other);
    native void Swap(Map_Str_Ptr other);
    native void Clear();
    native uint CountUsed() const;

    native voidptr Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") voidptr GetIfExists(String key) const;
    native version("4.11") voidptr, bool CheckValue(String key) const;
    
    native void Insert(String key, voidptr value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_Ptr native unsafe(internal)
{
    native bool Init(Map_Str_Ptr other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native voidptr GetValue();
    native void SetValue(voidptr value);
}

struct Map_Str_Str native unsafe(internal)
{
    native void Copy(Map_Str_Str other);
    native void Move(Map_Str_Str other);
    native void Swap(Map_Str_Str other);
    native void Clear();
    native uint CountUsed() const;

    native String Get(String key);
    native bool CheckKey(String key) const;
    
    native version("4.11") String GetIfExists(String key) const;
    native version("4.11") String, bool CheckValue(String key) const;
    
    native void Insert(String key, String value);
    native void InsertNew(String key);
    native void Remove(String key);
}

struct MapIterator_Str_Str native unsafe(internal)
{
    native bool Init(Map_Str_Str other);
    native bool ReInit();
    
    native bool Valid();
    native bool Next();
    
    native String GetKey();
    native String GetValue();
    native void SetValue(String value);
}
