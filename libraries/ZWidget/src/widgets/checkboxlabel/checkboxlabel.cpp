
#include "widgets/checkboxlabel/checkboxlabel.h"

CheckboxLabel::CheckboxLabel(Widget* parent) : Widget(parent)
{
	SetStyleClass("checkbox-label");
}

void CheckboxLabel::SetText(const std::string& value)
{
	if (text != value)
	{
		text = value;
		Update();
	}
}

const std::string& CheckboxLabel::GetText() const
{
	return text;
}

void CheckboxLabel::SetChecked(bool value)
{
	if (value != checked)
	{
		checked = value;
		Update();
	}
}

Size CheckboxLabel::GetCheckboxSize()
{
	if (auto image = GetStyleImage("checked-image"))
	{
		return { (double)image->GetWidth(), std::max((double)image->GetHeight(), GetCanvas()->getFontMetrics(GetFont()).height) };
	}
	else
	{
		return { 12.0, 12.0 };
	}
}

bool CheckboxLabel::GetChecked() const
{
	return checked;
}

double CheckboxLabel::GetPreferredWidth()
{
	const auto canvas = GetCanvas();
	return GetCheckboxSize().width + canvas->measureText(GetFont(), text).width + 1.0;
}

double CheckboxLabel::GetPreferredHeight()
{
	return std::max(GetCheckboxSize().height, GetCanvas()->getFontMetrics(GetFont()).height);
}

void CheckboxLabel::OnPaint(Canvas* canvas)
{
	Size s = GetCheckboxSize();

	FontMetrics metrics = canvas->getFontMetrics(GetFont());
	double baseline = (GetHeight() - metrics.height) * 0.5 + metrics.ascent;

	double borderwidth = GridFitSize(1.0);
	double outerboxsize = GridFitSize(10.0);
	double center = GridFitPoint(baseline - outerboxsize * 0.5);
	double innerboxsize = outerboxsize - 2.0 * borderwidth;
	double checkedsize = innerboxsize - 2.0 * borderwidth;

	if (checked)
	{
		if (auto image = GetStyleImage("checked-image"))
		{
			canvas->drawImage(image, Point(0.0, center - s.height * 0.5 - GetStyleDouble("checked-align")));
		}
		else
		{
			canvas->fillRect(Rect::xywh(0.0, center - 6.0 * borderwidth, outerboxsize, outerboxsize), GetStyleColor("checked-outer-border-color"));
			canvas->fillRect(Rect::xywh(1.0 * borderwidth, center - 5.0 * borderwidth, innerboxsize, innerboxsize), GetStyleColor("checked-inner-border-color"));
			canvas->fillRect(Rect::xywh(2.0 * borderwidth, center - 4.0 * borderwidth, checkedsize, checkedsize), GetStyleColor("checked-color"));
		}
	}
	else
	{
		if (auto image = GetStyleImage("unchecked-image"))
		{
			canvas->drawImage(image, Point(0.0, center - s.height * 0.5 - GetStyleDouble("unchecked-align")));
		}
		else
		{
			canvas->fillRect(Rect::xywh(0.0, center - 6.0 * borderwidth, outerboxsize, outerboxsize), GetStyleColor("unchecked-outer-border-color"));
			canvas->fillRect(Rect::xywh(1.0 * borderwidth, center - 5.0 * borderwidth, innerboxsize, innerboxsize), GetStyleColor("unchecked-inner-border-color"));
		}
	}

	canvas->drawText(GetFont(), Point(s.width + 2.0, baseline), text, GetStyleColor("color"));
}

bool CheckboxLabel::OnMouseDown(const Point& pos, InputKey key)
{
	mouseDownActive = true;
	SetFocus();
	return true;
}

bool CheckboxLabel::OnMouseUp(const Point& pos, InputKey key)
{
	if (mouseDownActive)
	{
		Toggle();
	}
	mouseDownActive = false;
	return true;
}

void CheckboxLabel::OnMouseLeave()
{
	mouseDownActive = false;
}

void CheckboxLabel::OnKeyUp(InputKey key)
{
	if (key == InputKey::Space)
		Toggle();
}

void CheckboxLabel::Toggle()
{
	bool oldchecked = checked;
	checked = radiostyle? true : !checked;
	Update();
	if (checked != oldchecked && FuncChanged) FuncChanged(checked);
}
