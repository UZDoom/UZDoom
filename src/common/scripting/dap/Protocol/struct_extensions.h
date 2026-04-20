/*
** struct_extensions.h
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

#pragma once

#include <dap/typeof.h>
#include <dap/types.h>
#include <dap/protocol.h>

namespace dap
{

// Extended AttachRequest struct for implementation specific parameters

struct PDSAttachRequest : public AttachRequest
{
	using Response = AttachResponse;
	string name;
	string type;
	string request;
	optional<array<Source>> projectSources;
};

struct PDSLaunchRequest : public LaunchRequest
{
	using Response = LaunchResponse;
	string name;
	string type;
	string request;
	optional<array<Source>> projectSources;
};

DAP_DECLARE_STRUCT_TYPEINFO(PDSAttachRequest);
DAP_DECLARE_STRUCT_TYPEINFO(PDSLaunchRequest);

}
