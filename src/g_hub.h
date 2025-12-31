/*
** g_hub.h
**
** Intermission stats for hubs
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
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

#ifndef __G_HUB_H
#define __G_HUB_H

struct cluster_info_t;
struct wbstartstruct_t;
class FSerializer;
struct FLevelLocals;

void G_SerializeHub (FSerializer &file);
void G_LeavingHub(FLevelLocals *Level, int mode, cluster_info_t * cluster, struct wbstartstruct_t * wbs);

#endif

