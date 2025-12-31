/*
** IdMap.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
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
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "IdProvider.h"

#include <unordered_map>

namespace DebugServer
{
template <typename T> class IdMap
{
	std::shared_ptr<IdProvider> m_idProvider;
	std::unordered_map<uint32_t, T> m_idsToElements;
	std::unordered_map<T, uint32_t> m_elementsToIds;

	std::recursive_mutex m_elementsMutex;
	public:
	explicit IdMap(const std::shared_ptr<IdProvider> idProvider) : m_idProvider(idProvider) { }

	~IdMap() { Clear(); }

	bool Get(uint32_t id, T &value)
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		auto pair = m_idsToElements.find(id);
		if (pair != m_idsToElements.end())
		{
			value = pair->second;
			return true;
		}

		return false;
	}

	bool GetId(T element, uint32_t &id)
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		auto pair = m_elementsToIds.find(element);
		if (pair != m_elementsToIds.end())
		{
			id = pair->second;
			return true;
		}

		return false;
	}

	bool AddOrGetExisting(T element, uint32_t &id)
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		if (GetId(element, id))
		{
			return false;
		}

		id = m_idProvider->GetNext();
		m_idsToElements.emplace(id, element);
		m_elementsToIds.emplace(element, id);

		return true;
	}

	bool Remove(uint32_t id)
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		auto pair = m_idsToElements.find(id);
		if (pair != m_idsToElements.end())
		{
			m_idsToElements.erase(id);
			m_elementsToIds.erase(pair->second);
			return true;
		}

		return false;
	}

	bool Remove(T element)
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		uint32_t id;
		if (GetId(element), id)
		{
			return Remove(id);
		}

		return false;
	}

	void Clear()
	{
		std::lock_guard<std::recursive_mutex> lock(m_elementsMutex);

		m_idsToElements.clear();
		m_elementsToIds.clear();
	}
};

}
