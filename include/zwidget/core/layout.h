#pragma once

#include "widget.h"
#include <vector>

class Layout;

// A widget solely used to take the rest of the available space.
// Only Layout classes can create them.
class Stretch : public Widget
{
	Stretch();
	friend class Layout;
};

// The Base Layout class. Shouldn't be used by itself.
// Use its subclasses HBoxLayout and VBoxLayout instead
class Layout
{
public:
	explicit Layout(Widget* parent = nullptr);
	virtual ~Layout() = default;

	void AddWidget(Widget* widget); // Add an already existing Widget to layout
	void AddLayout(Layout* layout); // Add a sub-Layout by making it reside in a new Widget
	void AddStretch();              // Add a special Stretch widget to use the remaining available space

	Widget* Parent() const;
	void SetParent(Widget* parent);

	virtual double GetPreferredWidth() = 0;
	virtual double GetPreferredHeight() = 0;
	virtual void OnGeometryChanged() = 0;

protected:
	static double GetFrameWidth(Widget* widget);
	static double GetFrameHeight(Widget* widget);

	std::vector<Widget*> Widgets;
	Widget* ParentWidget = nullptr;
};
