/*
** autosegs.cpp
**
** This file contains the heads of lists stored in special data segments
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
** The particular scheme used here was chosen because it's small.
**
** An alternative that will work with any C++ compiler is to use static
** classes to build these lists at run time. Under Visual C++, doing things
** that way can require a lot of extra space, which is why I'm doing things
** this way.
**
** In the case of PClass lists (section creg), I orginally used the
** constructor to do just that, and the code for that still exists if you
** compile with something other than Visual C++ or GCC.
*/

#include "autosegs.h"

FAutoSeg<AFuncDesc> AutoSegs::ActionFunctons;
FAutoSeg<FieldDesc> AutoSegs::ClassFields;
FAutoSeg<ClassReg> AutoSegs::TypeInfos;
FAutoSeg<FPropertyInfo> AutoSegs::Properties;
FAutoSeg<FMapOptInfo> AutoSegs::MapInfoOptions;
FAutoSeg<FCVarDecl> AutoSegs::CVarDecl;
