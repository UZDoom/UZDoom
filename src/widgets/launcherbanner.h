/*
** launcherbanner.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include <zwidget/core/widget.h>

#include <memory>
#include <vector>

#include "name.h"

class ImageBox;
class TextLabel;

class LauncherBanner : public Widget
{
public:
	LauncherBanner(Widget* parent, FName colors, float mix);

	double GetPreferredHeight() override;

private:
	void OnGeometryChanged() override;

	std::vector<std::tuple<std::unique_ptr<Widget>,std::unique_ptr<Widget>>> stripes;

	ImageBox* Logo = nullptr;
};
