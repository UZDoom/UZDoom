
#include "widgets/pushbutton/pushbutton.h"

PushButton::PushButton(Widget* parent) : Widget(parent)
{
	SetStyleClass("pushbutton");
}

void PushButton::SetText(const std::string& value)
{
	if (text != value)
	{
		text = value;
		Update();
	}
}

const std::string& PushButton::GetText() const
{
	return text;
}

double PushButton::GetPreferredWidth()
{
	const auto canvas = GetCanvas();
	return std::max(100.0, canvas->measureText(GetFont(), text).width);
}

double PushButton::GetPreferredHeight()
{
	const auto canvas = GetCanvas();
	return std::max(20.0, canvas->measureText(GetFont(), text).height);
}

void PushButton::OnPaint(Canvas* canvas)
{
	FontMetrics metrics = canvas->getFontMetrics(GetFont());
	Rect box = canvas->measureText(GetFont(), text);
	canvas->drawText(GetFont(), Point((GetWidth() - box.width) * 0.5, (GetHeight() - metrics.height) * 0.5 + metrics.ascent), text, GetStyleColor("color"));
}

void PushButton::OnMouseMove(const Point& pos)
{
	if (GetStyleState().empty())
	{
		SetStyleState("hover");
	}
}

bool PushButton::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();
	SetStyleState("down");
	return true;
}

bool PushButton::OnMouseUp(const Point& pos, InputKey key)
{
	if (GetStyleState() == "down")
	{
		SetStyleState("");
		Repaint();
		Click();
	}
	return true;
}

void PushButton::OnMouseLeave()
{
	SetStyleState("");
}

void PushButton::OnKeyDown(InputKey key)
{
	if (key == InputKey::Space || key == InputKey::Enter)
	{
		SetStyleState("down");
		Update();
	}
}

void PushButton::OnKeyUp(InputKey key)
{
	if (key == InputKey::Space || key == InputKey::Enter)
	{
		SetStyleState("");
		Repaint();
		Click();
	}
}

void PushButton::Click()
{
	if (OnClick)
		OnClick();
}
