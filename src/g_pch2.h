/*
** g_pch2.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

// This is separate because the files being compiled with it use different compiler settings which may affect how the header is compiled
#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <miniz.h>
#include <new>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <cassert>
#include <direct.h>
#include <io.h>
#include <limits>
#include <fcntl.h>
