/*
** struct_extensions.cpp
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

#include "struct_extensions.h"

namespace dap
{
DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT(
	PDSAttachRequest,
	AttachRequest,
	"attach",
	DAP_FIELD(name, "name"),
	DAP_FIELD(type, "type"),
	DAP_FIELD(request, "request"),
	DAP_FIELD(projectSources, "projectSources"));

DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT(
	PDSLaunchRequest,
	LaunchRequest,
	"launch",
	DAP_FIELD(name, "name"),
	DAP_FIELD(type, "type"),
	DAP_FIELD(request, "request"),
	DAP_FIELD(projectSources, "projectSources"));
}
