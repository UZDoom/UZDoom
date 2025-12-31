/*
** serialize_obj.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christoph Oelckers
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

#pragma once

// These are in a separate header because they require some rather 'dirty' headers to work which should not be part of serializer.h

template<class T>
FSerializer &Serialize(FSerializer &arc, const char *key, TObjPtr<T> &value, TObjPtr<T> *)
{
	Serialize(arc, key, value.o, nullptr);
	return arc; 
}

template<class T>
FSerializer &Serialize(FSerializer &arc, const char *key, TObjPtr<T> &value, T *)
{
	Serialize(arc, key, value.o, nullptr);
	return arc;
}
