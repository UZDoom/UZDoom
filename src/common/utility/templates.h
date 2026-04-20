/*
** templates.h
**
** Some useful template functions
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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

#ifndef __TEMPLATES_H__
#define __TEMPLATES_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <stdlib.h>
#include <utility>
#include <algorithm>

//==========================================================================
//
// BinarySearch
//
// Searches an array sorted in ascending order for an element matching
// the desired key.
//
// Template parameters:
//		ClassType -		The class to be searched
//		KeyType -		The type of the key contained in the class
//
// Function parameters:
//		first -			Pointer to the first element in the array
//		max -			The number of elements in the array
//		keyptr -		Pointer to the key member of ClassType
//		key -			The key value to look for
//
// Returns:
//		A pointer to the element with a matching key or NULL if none found.
//==========================================================================

template<class ClassType, class KeyType>
inline
const ClassType *BinarySearch (const ClassType *first, int max,
	const KeyType ClassType::*keyptr, const KeyType key)
{
	int min = 0;
	--max;

	while (min <= max)
	{
		int mid = (min + max) / 2;
		const ClassType *probe = &first[mid];
		const KeyType &seekey = probe->*keyptr;
		if (seekey == key)
		{
			return probe;
		}
		else if (seekey < key)
		{
			min = mid + 1;
		}
		else
		{
			max = mid - 1;
		}
	}
	return NULL;
}


#endif //__TEMPLATES_H__
