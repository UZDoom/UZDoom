/*
** IdProvider.h
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
#include <mutex>

namespace DebugServer
{
class IdProvider
{
	uint32_t m_currentId = 1000;
	std::mutex m_idMutex;
	public:
	uint32_t GetNext();
};
}
