/*
** dynarrays.zs
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
** The VM uses 7 integral data types, so for dynamic array support we need
** one specific set of functions for each of these types.
**
** Do not use these structs directly, they are incomplete and only needed
** to create prototypes for the needed functions.
*/

struct DynArray_I8 native unsafe(internal)
{
	native readonly int Size;
	
	native void Copy(DynArray_I8 other);
	native void Move(DynArray_I8 other);
	native void Append (DynArray_I8 other);
	native int Find(int item) const;
	native int Push(int item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, int item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_I16 native unsafe(internal)
{
	native readonly int Size;

	native void Copy(DynArray_I16 other);
	native void Move(DynArray_I16 other);
	native void Append (DynArray_I16 other);
	native int Find(int item) const;
	native int Push(int item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, int item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_I32 native unsafe(internal)
{
	native readonly int Size;

	native void Copy(DynArray_I32 other);
	native void Move(DynArray_I32 other);
	native void Append (DynArray_I32 other);
	native int Find(int item) const;
	native int Push(int item);
	native vararg uint PushV (int item, ...);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, int item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_F32 native unsafe(internal)
{
	native readonly int Size;
	
	native void Copy(DynArray_F32 other);
	native void Move(DynArray_F32 other);
	native void Append (DynArray_F32 other);
	native int Find(double item) const;
	native int Push(double item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, double item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_F64 native unsafe(internal)
{
	native readonly int Size;
	
	native void Copy(DynArray_F64 other);
	native void Move(DynArray_F64 other);
	native void Append (DynArray_F64 other);
	native int Find(double item) const;
	native int Push(double item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, double item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_Ptr native unsafe(internal)
{
	native readonly int Size;
	
	native void Copy(DynArray_Ptr other);
	native void Move(DynArray_Ptr other);
	native void Append (DynArray_Ptr other);
	native int Find(voidptr item) const;
	native int Push(voidptr item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, voidptr item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_Obj native unsafe(internal)
{
	native readonly int Size;
	
	native void Copy(DynArray_Obj other);
	native void Move(DynArray_Obj other);
	native void Append (DynArray_Obj other);
	native int Find(Object item) const;
	native int Push(Object item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, Object item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_String native unsafe(internal)
{
	native readonly int Size;

	native void Copy(DynArray_String other);
	native void Move(DynArray_String other);
	native void Append (DynArray_String other);
	native int Find(String item) const;
	native int Push(String item);
	native vararg uint PushV(String item, ...);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, String item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}

struct DynArray_TRS native unsafe(internal)
{
	native readonly int Size;

	native void Copy(DynArray_TRS other);
	native void Move(DynArray_TRS other);
	native void Append (DynArray_TRS other);
	native int Find(TRS item) const;
	native int Push(TRS item);
	native bool Pop ();
	native void Delete (uint index, int deletecount = 1);
	native void Insert (uint index, TRS item);
	native void ShrinkToFit ();
	native void Grow (uint amount);
	native void Resize (uint amount);
	native int Reserve(uint amount);
	native int Max() const;
	native void Clear ();
}
