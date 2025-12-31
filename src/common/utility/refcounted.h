/*
** refcounted.h
**
** Simple lightweight reference counting pointer
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Marisa Heit
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
** Simple lightweight reference counting pointer alternative
** for std::shared_ptr which stores the reference counter in
** the handled object itself.
*/

#pragma once

//

// Base class for handled objects
class RefCountedBase
{
public:
	void IncRef() { refCount++; }
	void DecRef() { if (--refCount <= 0) delete this; }
private:
	int refCount = 0;
protected:
	virtual ~RefCountedBase() = default;
};

// The actual pointer object
template<class T> 
class RefCountedPtr
{
public:
    ~RefCountedPtr() 
	{
        if (ptr) ptr->DecRef();
    }

    RefCountedPtr() : ptr(nullptr) 
	{}

    explicit RefCountedPtr(T* p) : ptr(p)
    {
        if (ptr) ptr->IncRef();
    }

	RefCountedPtr(const RefCountedPtr& r) : ptr(r.ptr)
	{
		if (ptr) ptr->IncRef();
	}

	RefCountedPtr(RefCountedPtr&& r) : ptr(r.ptr)
	{
		r.ptr = nullptr;
	}

    RefCountedPtr & operator=(const RefCountedPtr& r) 
    {
		if (this != &r)
		{
            if (ptr) ptr->DecRef();
			ptr = r.ptr;
            if (ptr) ptr->IncRef();
		}
        return *this;
    }

    RefCountedPtr& operator=(T* r)
    {
        if (ptr != r)
        {
            if (ptr) ptr->DecRef();
            ptr = r;
            if (ptr) ptr->IncRef();
        }
        return *this;
    }

    RefCountedPtr & operator=(RefCountedPtr&& r)
    {
		if (this != &r)
		{
			if (ptr) ptr->DecRef();
			ptr = r.ptr;
			r.ptr = nullptr;
		}
        return *this;
    }

    bool operator==(T* p) const
    {
        return ptr == p;
    }

    bool operator!=(T* p) const
    {
        return ptr != p;
    }

    bool operator==(const RefCountedPtr &p) const
    {
        return ptr == p.ptr;
    }

    bool operator!=(const RefCountedPtr& p) const
    {
        return ptr != p.ptr;
    }

    T& operator* () const
	{
        return *ptr;
    }

    T* operator-> () const 
	{
        return ptr;
    }

    T* get() const 
	{
        return ptr;
    }

private:

    T * ptr;
};
