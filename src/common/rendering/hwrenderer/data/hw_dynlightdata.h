/*
** hw_dynlightdata.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __GLC_DYNLIGHT_H
#define __GLC_DYNLIGHT_H

#include "tarray.h"
#include <algorithm>

struct FDynLightData
{
	TArray<float> arrays[3];

	void Clear()
	{
		arrays[0].Clear();
		arrays[1].Clear();
		arrays[2].Clear();
	}

	inline size_t Vec4Size() const
	{
		// All measurements here are in vec4's.
		int size0 = arrays[0].Size()/4;
		int size1 = arrays[1].Size()/4;
		int size2 = arrays[2].Size()/4;
		return size0 + size1 + size2 + 1;
	}

	bool Combine(TArray<float> &out, size_t max) const
	{
		if(max < (5 * 4)) return false; // not enough space for even a single light
		size_t len = std::min<size_t>(max, Vec4Size() * 4);
		out.Resize(len);
		size_t cur = 4;
		out[0] = 0;
		for(int i = 0; i < 3; i++)
		{
			size_t l = std::min<size_t>(len - cur, arrays[i].Size());
			if(l > 0) memcpy(&out[cur], arrays[i].Data(), l * sizeof(float));
			cur += l;
			out[i + 1] = (cur - 4) / 4;
		}
		return true;
	}


};

extern thread_local FDynLightData lightdata;


#endif
