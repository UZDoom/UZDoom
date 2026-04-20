/*
** IdProvider.cpp
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

#include "IdProvider.h"

namespace DebugServer
{
uint32_t IdProvider::GetNext()
{
	std::lock_guard<std::mutex> lock(m_idMutex);
	return m_currentId++;
}
}
