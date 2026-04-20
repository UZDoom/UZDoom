/*
** IdHandleBase.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: MIT
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "IdMap.h"

namespace DebugServer
{
template <class T> class IdHandleBase
{
	IdMap<T> *m_idMap;
	public:
	explicit IdHandleBase(IdMap<T> *idMap) : m_idMap(idMap)
	{
		static_assert(std::is_base_of<IdHandleBase<T>, T>());
		m_idMap->Add(static_cast<T *>(this));
	}

	virtual ~IdHandleBase() { m_idMap->Remove(static_cast<T *>(this)); }

	uint32_t GetId() const { return m_idMap->GetId(static_cast<T *>(this)); }
};
}
