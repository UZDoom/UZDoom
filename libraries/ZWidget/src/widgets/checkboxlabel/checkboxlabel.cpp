
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

bool CheckboxLabel::GetChecked() const
{
	return checked;
}

double CheckboxLabel::GetPreferredHeight()
{
	return 24.0;
}

void CheckboxLabel::OnPaint(Canvas* canvas)
{
	double h = GetHeight();
	double center = GridFitPoint(h * 0.5);
	double borderwidth = GridFitSize(1.0);
	double boxSize = 16.0;
	double boxY = GridFitPoint(center - boxSize * 0.5);
	double radius = radiostyle ? boxSize * 0.5 : 4.5;
	Rect box = Rect::xywh(0.0, boxY, boxSize, boxSize);

	if (checked)
	{
		Colorf fill = GetStyleColor("checked-color");
		Colorf check = GetStyleColor("checked-inner-border-color");
		canvas->fillRoundedRect(box, radius, fill);
		if (radiostyle)
		{
			canvas->fillRoundedRect(Rect::xywh(5.0, boxY + 5.0, 6.0, 6.0), 3.0, check);
		}
		else
		{
			canvas->line(Point(4.0, boxY + 8.5), Point(7.0, boxY + 11.5), check);
			canvas->line(Point(7.0, boxY + 11.5), Point(12.0, boxY + 5.0), check);
		}
	}
	else
	{
		canvas->fillRoundedRect(box, radius, GetStyleColor("unchecked-inner-border-color"));
		canvas->strokeRoundedRect(box, radius, borderwidth, GetStyleColor("unchecked-outer-border-color"));
	}

	VerticalTextPosition vtp = canvas->verticalTextAlign();
	double textY = (h - (vtp.bottom - vtp.top)) * 0.5 + vtp.baseline;
	canvas->drawText(Point(24.0, textY), GetStyleColor("color"), text);
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
