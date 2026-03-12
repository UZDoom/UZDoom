#pragma once

#include "zwidget/core/layout.h"

class HBoxLayout final : public Layout
{
public:
	HBoxLayout(Widget* parent = nullptr);

	void OnGeometryChanged() override;

	double GetPreferredWidth() override;
	double GetPreferredHeight() override;

	void SetGapWidth(double newGapWidth);

private:
	double GapWidth = 4.0;
};
