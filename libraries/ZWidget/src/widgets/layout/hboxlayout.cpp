#include "zwidget/widgets/layout/hboxlayout.h"

HBoxLayout::HBoxLayout(Widget* parent) : Layout(parent)
{
}

void HBoxLayout::OnGeometryChanged()
{
	// The idea is this:
	// Calculate all elements' preferred widths
	// Then subtract it from the parent widget's own width
	// Then distribute that remaining width evenly among all Stretches

	// Do nothing if there are no widgets added to layout
	if (Widgets.empty())
		return;

	if (!ParentWidget)
		return;

	const double totalWidth = ParentWidget->GetWidth();
	double nonStretchingWidgetsTotalWidth = 0.0;
	int stretchingWidgetsCount = 0;
	double stretchWidths = 0.0;

	bool first = true;
	for (const auto& widget : Widgets)
	{
		if (!first)
			nonStretchingWidgetsTotalWidth += GapWidth;
		else
			first = false;

		if (!widget->GetStretching())
		{
			nonStretchingWidgetsTotalWidth += GetFrameWidth(widget);
		}
		else
		{
			stretchingWidgetsCount++;
		}
	}

	if (stretchingWidgetsCount > 0)
		stretchWidths = (totalWidth - nonStretchingWidgetsTotalWidth) / stretchingWidgetsCount;

	double left = 0.0;

	for (const auto& widget: Widgets)
	{
		double frameWidth = 0.0;

		if (!widget->GetStretching())
			frameWidth = GetFrameWidth(widget);
		else
			frameWidth = stretchWidths;

		widget->SetFrameGeometry(Rect::xywh(left, 0, frameWidth, ParentWidget->GetHeight()));
		left += frameWidth + GapWidth;
	}
}

double HBoxLayout::GetPreferredWidth()
{
	double w = 0.0;
	bool first = true;
	for (const auto& widget : Widgets)
	{
		if (!first)
			w += GapWidth;
		else
			first = false;

		w += GetFrameWidth(widget);
	}
	return w;
}

double HBoxLayout::GetPreferredHeight()
{
	double h = 0.0;
	for (const auto& widget : Widgets)
	{
		h = std::max(h, GetFrameHeight(widget));
	}
	return h;
}

void HBoxLayout::SetGapWidth(const double newGapWidth)
{
	GapWidth = newGapWidth;
}
