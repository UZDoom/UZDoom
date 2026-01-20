#include "zwidget/widgets/layout/vboxlayout.h"

VBoxLayout::VBoxLayout(Widget* parent) : Layout(parent)
{
}

void VBoxLayout::OnGeometryChanged()
{
	// The idea is this:
	// Calculate all elements' preferred heights
	// Then subtract it from the parent widget's own height
	// Then distribute that remaining height evenly among all Stretches

	// Do nothing if there are no widgets added to layout
	if (Widgets.empty())
		return;

	if (!ParentWidget)
		return;

	const double totalHeight = ParentWidget->GetHeight();
	double nonStretchingWidgetsTotalHeight = 0.0;
	int stretchingWidgetsCount = 0;
	double stretchHeights = 0.0;

	bool first = true;
	for (const auto& widget : Widgets)
	{
		if (!first)
			nonStretchingWidgetsTotalHeight += GapHeight;
		else
			first = false;

		if (!widget->GetStretching())
		{
			nonStretchingWidgetsTotalHeight += GetFrameHeight(widget);
			nonStretchingWidgetsTotalHeight += GapHeight;
		}
		else
			stretchingWidgetsCount++;
	}

	if (stretchingWidgetsCount > 0)
		stretchHeights = (totalHeight - nonStretchingWidgetsTotalHeight) / stretchingWidgetsCount;

	double top = 0.0;

	for (const auto& widget: Widgets)
	{
		double frameHeight = 0.0;
		if (!widget->GetStretching())
			frameHeight = GetFrameHeight(widget);
		else
			frameHeight = stretchHeights;

		widget->SetFrameGeometry(Rect::xywh(0, top, ParentWidget->GetWidth(), frameHeight));
		top += frameHeight + GapHeight;
	}
}

double VBoxLayout::GetPreferredWidth()
{
	double w = 0.0;
	for (const auto& widget : Widgets)
	{
		w = std::max(w, GetFrameWidth(widget));
	}
	return w;
}

double VBoxLayout::GetPreferredHeight()
{
	double h = 0.0;
	bool first = true;
	for (const auto& widget : Widgets)
	{
		if (!first)
			h += GapHeight;
		else
			first = false;
		h += GetFrameHeight(widget);
	}
	return h;
}

void VBoxLayout::SetGapHeight(const double newGapHeight)
{
	GapHeight = newGapHeight;
}
