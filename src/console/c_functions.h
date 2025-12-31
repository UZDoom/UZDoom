/*
** c_functions.h
**
** Miscellaneous console command helper functions.
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Rachael Alexanderson
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

void C_PrintInv(AActor *target);
void C_AimLine(FTranslatedLineTarget *t, bool nonshootable);
void C_PrintInfo(AActor *target, bool verbose);

struct FTranslatedLineTarget;
