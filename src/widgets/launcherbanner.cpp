/*
** launcherbanner.cpp
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

#include <zwidget/core/image.h>
#include <zwidget/widgets/imagebox/imagebox.h>
#include <zwidget/widgets/textlabel/textlabel.h>

#include "launcherbanner.h"
#include "themedata.h"

LauncherBanner::LauncherBanner(Widget* parent) : Widget(parent)
{
	Logo = new ImageBox(this);
	auto imgsrc = Theme::getMode() == LIGHT ? "ui/banner-light.png": "ui/banner-dark.png";
	Logo->SetImage(Image::LoadResource(imgsrc));
	this->SetStyleColor("background-color", Theme::getHeader(COLOR_BACKGROUND));
}

double LauncherBanner::GetPreferredHeight() const
{
	return Logo->GetPreferredHeight();
}

void LauncherBanner::OnGeometryChanged()
{
	Logo->SetFrameGeometry(0.0, 0.0, GetWidth(), Logo->GetPreferredHeight());
}
