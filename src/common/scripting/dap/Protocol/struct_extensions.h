/*
** struct_extensions.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2025 nikitalita
** Copyright 2025 UZDoom Maintainers and Contributors
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
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
