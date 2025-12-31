/*
** maps.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Jay
** Copyright 2022-2025 GZDoom Maintainers and Contributors
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
#include "refcounted.h"
#include "vm.h"

class ZSMapInfo : public RefCountedBase
{
public:
    void * self = nullptr;
    int rev = 0;
};

struct ZSFMap : FMap {
    RefCountedPtr<RefCountedBase> info;
};

template<class KT, class VT>
class ZSMap : public TMap<KT,VT>
{
public:
    RefCountedPtr<ZSMapInfo> info;
    ZSMap() :
        TMap<KT,VT>(), info(new ZSMapInfo)
    {
        info->self = this;
    }
    ~ZSMap()
    {
        info->self = nullptr;
    }
};

template<class KT, class VT>
struct ZSMapIterator
{
    RefCountedPtr<ZSMapInfo> info;
    TMapIterator<KT,VT>  *it = nullptr;
    typename ZSMap<KT,VT>::Pair *p = nullptr;

    typedef KT KeyType;
    typedef VT ValueType;

    int rev = 0;

    ~ZSMapIterator()
    {
        if(it) delete it;
    }

    bool Valid()
    {
        return it && p && info.get() && info->self && info->rev == rev;
    }

    bool ReInit()
    {
        if(info.get() && info->self) {
            if(it) delete it;
            it = new TMapIterator<KT,VT>(*static_cast<ZSMap<KT,VT>*>(info->self));
            rev = info->rev;
            p = nullptr;
            return true;
        }
        return false;
    }

    bool Init(ZSMap<KT,VT> &m)
    {
        info = m.info;
        return ReInit();
    }

    bool Next()
    {
        if(it && info.get() && info->self && info->rev == rev)
        {
            p = nullptr;
            return it->NextPair(p);
        }
        else
        {
            ThrowAbortException(X_OTHER, "MapIterator::Next called from invalid iterator");
        }
    }

    VT& GetValue()
    {
        if(p && info.get() && info->self && info->rev == rev)
        {
            return p->Value;
        }
        else
        {
            ThrowAbortException(X_OTHER, p ? "MapIterator::GetValue called from invalid iterator" :  "MapIterator::GetValue called from invalid position");
        }
    }

    const KT& GetKey()
    {
        if(p && info.get() && info->self && info->rev == rev)
        {
            return p->Key;
        }
        else
        {
            ThrowAbortException(X_OTHER, p ? "MapIterator::GetKey called from invalid iterator" : "MapIterator::GetKey called from invalid position");
        }
    }
};
