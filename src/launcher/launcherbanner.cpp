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

#include "launcherbanner.h"
#include "gstrings.h"
#include "version.h"
#include <zwidget/widgets/imagebox/imagebox.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/core/image.h>

LauncherBanner::LauncherBanner(Widget* parent) : Widget(parent)
{
	Logo = new ImageBox(this);
	Logo->SetImage(Image::LoadResource("widgets/banner.png"));
}

double LauncherBanner::GetPreferredHeight() const
{
	return Logo->GetPreferredHeight();
}

void LauncherBanner::OnGeometryChanged()
{
	Logo->SetFrameGeometry(0.0, 0.0, GetWidth(), Logo->GetPreferredHeight());
}
