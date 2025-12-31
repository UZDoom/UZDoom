/*
** m_alloc.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __M_ALLOC_H__
#define __M_ALLOC_H__

#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#define _msize(p)				malloc_size(p)
#elif defined(__solaris__) || defined(__OpenBSD__) || defined(__DragonFly__)
#define _msize(p)				(*((size_t*)(p)-1))
#elif !defined(_WIN32)
#ifdef __FreeBSD__
#include <malloc_np.h>
#else
#include <malloc.h>
#endif
#define _msize(p)				malloc_usable_size(p)	// from glibc/FreeBSD
#endif

// These are the same as the same stdlib functions,
// except they bomb out with a fatal error
// when they can't get the memory.

#if defined(_DEBUG)
#define M_Calloc(s,t)		M_Calloc_Dbg(s, t, __FILE__, __LINE__)
#define M_Malloc(s)		M_Malloc_Dbg(s, __FILE__, __LINE__)
#define M_Realloc(p,s)	M_Realloc_Dbg(p, s, __FILE__, __LINE__)

void *M_Malloc_Dbg (size_t size, const char *file, int lineno);
void *M_Realloc_Dbg (void *memblock, size_t size, const char *file, int lineno);
inline void* M_Calloc_Dbg(size_t v1, size_t v2, const char* file, int lineno)
{
	auto p = M_Malloc_Dbg(v1 * v2, file, lineno);
	memset(p, 0, v1 * v2);
	return p;
}

#else
void *M_Malloc (size_t size);
void *M_Realloc (void *memblock, size_t size);
void* M_Calloc(size_t v1, size_t v2);

#endif


void M_Free (void *memblock);

#endif //__M_ALLOC_H__
