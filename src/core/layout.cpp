#include "zwidget/core/layout.h"

Stretch::Stretch()
{
	SetStretching(true);
}

Layout::Layout(Widget* parent) : ParentWidget(parent)
{
	if (ParentWidget)
		SetParent(ParentWidget);
}

void Layout::AddWidget(Widget* widget)
{
	if (!widget)
		return;

	Widgets.push_back(widget);
	if (ParentWidget)
		widget->SetParent(ParentWidget);
}

void Layout::AddLayout(Layout* layout)
{
	if (!layout)
		return;

	const auto widget = new Widget();

	widget->SetLayout(layout);
	widget->SetParent(ParentWidget);

	Widgets.push_back(widget);
}

void Layout::AddStretch()
{
	Widgets.push_back(new Stretch());
}

Widget* Layout::Parent() const
{
	return ParentWidget;
}

void Layout::SetParent(Widget* parent)
{
	ParentWidget = parent;

	for (const auto widget : Widgets)
	{
		widget->SetParent(ParentWidget);
	}
}

double Layout::GetFrameWidth(Widget* widget)
{
	if (widget->GetFixedWidth().has_value())
	{
		return widget->GetNoncontentLeft() + widget->GetFixedWidth().value() + widget->GetNoncontentRight();
	}
	else
	{
		return widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
	}
}

double Layout::GetFrameHeight(Widget* widget)
{
	if (widget->GetFixedHeight().has_value())
	{
		return widget->GetNoncontentTop() + widget->GetFixedHeight().value() + widget->GetNoncontentBottom();
	}
	else
	{
		return widget->GetNoncontentTop() + widget->GetPreferredHeight() + widget->GetNoncontentBottom();
	}
}
