#include "core/widget.h"
#include "core/timer.h"
#include "core/colorf.h"
#include "core/theme.h"
#include "core/layout.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>

Widget::Widget(Widget* parent, WidgetType type, RenderAPI renderAPI) : Type(type)
{
	if (type != WidgetType::Child)
	{
		Widget* owner = parent ? parent->Window() : nullptr;
		DispWindow = DisplayWindow::Create(this, type, owner ? owner->DispWindow.get() : nullptr, renderAPI);
		if (renderAPI == RenderAPI::Unspecified || renderAPI == RenderAPI::Bitmap)
		{
			DispCanvas = Canvas::create();
			DispCanvas->attach(DispWindow.get());
		}
		SetStyleState("root");

		SetWindowBackground(GetStyleColor("window-background"));
		if (GetStyleColor("window-border").a > 0.0f)
			SetWindowBorderColor(GetStyleColor("window-border"));
		if (GetStyleColor("window-caption-color").a > 0.0f)
			SetWindowCaptionColor(GetStyleColor("window-caption-color"));
		if (GetStyleColor("window-caption-text-color").a > 0.0f)
			SetWindowCaptionTextColor(GetStyleColor("window-caption-text-color"));
	}

	SetParent(parent);
}

Widget::~Widget()
{
	for (auto subscription: Subscriptions)
		subscription->Subscribers.erase(this);

	for (auto subscriber: Subscribers)
		subscriber->Subscriptions.erase(this);

	if (DispCanvas)
		DispCanvas->detach();

	while (LastChildObj)
		delete LastChildObj;

	while (FirstTimerObj)
		delete FirstTimerObj;

	DetachFromParent();
}

void Widget::Subscribe(Widget* subscriber)
{
	Subscribers.insert(subscriber);
	subscriber->Subscriptions.insert(this);
}

void Widget::Unsubscribe(Widget* subscriber)
{
	Subscribers.erase(subscriber);
	subscriber->Subscriptions.erase(this);
}

void Widget::NotifySubscribers(const WidgetEvent type)
{
	for (auto subscriber: Subscribers)
		subscriber->Notify(this, type);
}

void Widget::SetCanvas(std::unique_ptr<Canvas> canvas)
{
	if (DispWindow)
	{
		if (DispCanvas)
			DispCanvas->detach();
		DispCanvas = std::move(canvas);
		DispCanvas->attach(DispWindow.get());
	}
}

void Widget::SetParent(Widget* newParent)
{
	if (ParentObj != newParent)
	{
		if (ParentObj)
			DetachFromParent();

		if (newParent)
		{
			PrevSiblingObj = newParent->LastChildObj;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			newParent->LastChildObj = this;
			if (!newParent->FirstChildObj) newParent->FirstChildObj = this;
			ParentObj = newParent;
		}
	}
}

void Widget::SetLayout(Layout* layout)
{
	m_Layout = layout;
	m_Layout->SetParent(this);
}

void Widget::MoveBefore(Widget* sibling)
{
	if (sibling && sibling->ParentObj != ParentObj) throw std::runtime_error("Invalid sibling passed to Widget.MoveBefore");
	if (!ParentObj) throw std::runtime_error("Widget must have a parent before it can be moved");

	if (NextSiblingObj != sibling)
	{
		Widget* p = ParentObj;
		DetachFromParent();

		ParentObj = p;
		if (sibling)
		{
			NextSiblingObj = sibling;
			PrevSiblingObj = sibling->PrevSiblingObj;
			sibling->PrevSiblingObj = this;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			if (ParentObj->FirstChildObj == sibling) ParentObj->FirstChildObj = this;
		}
		else
		{
			PrevSiblingObj = ParentObj->LastChildObj;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			ParentObj->LastChildObj = this;
			if (!ParentObj->FirstChildObj) ParentObj->FirstChildObj = this;
		}
	}
}

void Widget::DetachFromParent()
{
	for (Widget* cur = ParentObj; cur; cur = cur->ParentObj)
	{
		if (cur->FocusWidget == this)
			cur->FocusWidget = nullptr;
		if (cur->KeyboardLockWidget == this)
			cur->KeyboardLockWidget = nullptr;
		if (cur->CursorLockWidget == this)
			cur->CursorLockWidget = nullptr;
		if (cur->HoverWidget == this)
			cur->HoverWidget = nullptr;

		if (cur->DispWindow)
			break;
	}

	if (PrevSiblingObj)
		PrevSiblingObj->NextSiblingObj = NextSiblingObj;
	if (NextSiblingObj)
		NextSiblingObj->PrevSiblingObj = PrevSiblingObj;
	if (ParentObj)
	{
		if (ParentObj->FirstChildObj == this)
			ParentObj->FirstChildObj = NextSiblingObj;
		if (ParentObj->LastChildObj == this)
			ParentObj->LastChildObj = PrevSiblingObj;
	}
	PrevSiblingObj = nullptr;
	NextSiblingObj = nullptr;
	ParentObj = nullptr;
}

std::string Widget::GetWindowTitle() const
{
	return WindowTitle;
}

void Widget::SetWindowTitle(const std::string& text)
{
	if (WindowTitle != text)
	{
		WindowTitle = text;
		if (DispWindow)
			DispWindow->SetWindowTitle(WindowTitle);
	}
}

std::vector<std::shared_ptr<Image>> Widget::GetWindowIcon() const
{
	return WindowIcon;
}

void Widget::SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images)
{
	WindowIcon = images;
	if (DispWindow)
		DispWindow->SetWindowIcon(WindowIcon);
}

double Widget::GetPreferredWidth()
{
	return m_Layout ? m_Layout->GetPreferredWidth() : 0.0;
}

double Widget::GetPreferredHeight()
{
	return m_Layout ? m_Layout->GetPreferredHeight() : 0.0;
}

Size Widget::GetSize() const
{
	return ContentGeometry.size();
}

Rect Widget::GetFrameGeometry() const
{
	if (Type == WidgetType::Child)
	{
		return FrameGeometry;
	}
	else
	{
		return DispWindow->GetClientFrame();
	}
}

void Widget::SetNoncontentSizes(double left, double top, double right, double bottom)
{
	SetStyleDouble("noncontent-left", left);
	SetStyleDouble("noncontent-top", top);
	SetStyleDouble("noncontent-right", right);
	SetStyleDouble("noncontent-bottom", bottom);
}

void Widget::SetFrameGeometry(const Rect& geometry)
{
	if (Type == WidgetType::Child)
	{
		FrameGeometry = geometry;
		double left = FrameGeometry.left() + GetNoncontentLeft();
		double top = FrameGeometry.top() + GetNoncontentTop();
		double right = FrameGeometry.right() - GetNoncontentRight();
		double bottom = FrameGeometry.bottom() - GetNoncontentBottom();
		left = std::min(left, FrameGeometry.right());
		top = std::min(top, FrameGeometry.bottom());
		right = std::max(right, FrameGeometry.left());
		bottom = std::max(bottom, FrameGeometry.top());
		left = GridFitPoint(left);
		top = GridFitPoint(top);
		right = GridFitPoint(right);
		bottom = GridFitPoint(bottom);
		ContentGeometry = Rect::ltrb(left, top, std::max(right, left), std::max(bottom, top));

		if (m_Layout)
			m_Layout->OnGeometryChanged();

		OnGeometryChanged();
	}
	else
	{
		DispGeometrySet = true;
		DispWindow->SetClientFrame(geometry);
	}
}

void Widget::CheckInitialShow()
{
	if (Type != WidgetType::Child && !DispGeometrySet)
	{
		// If there is not a size set up, try to get it from the layout attached to the widget
		double layoutWidth = GetPreferredWidth();
		double layoutHeight = GetPreferredHeight();
		if (layoutWidth > 0.0 && layoutHeight > 0.0)
		{
			double frameWidth = layoutWidth + GetNoncontentLeft() + GetNoncontentRight();
			double frameHeight = layoutHeight + GetNoncontentTop() + GetNoncontentBottom();
			if (Widget* parentWindow = ParentObj ? ParentObj->Window() : nullptr)
			{
				// Center on parent
				Rect parentBox = parentWindow->GetFrameGeometry();
				double x = parentBox.x + (parentBox.width - frameWidth) * 0.5;
				double y = parentBox.y + (parentBox.height - frameHeight) * 0.5;
				SetFrameGeometry(Rect::xywh(x, y, frameWidth, frameHeight));
			}
			else
			{
				// Center the window on primary screen
				auto screenSize = DisplayWindow::GetScreenSize();
				double x = (screenSize.width - frameWidth) * 0.5;
				double y = (screenSize.height - frameHeight) * 0.5;
				SetFrameGeometry(Rect::xywh(x, y, frameWidth, frameHeight));
			}
		}
	}
}

void Widget::Show()
{
	if (Type != WidgetType::Child)
	{
		CheckInitialShow();
		DispWindow->Show();
		NotifySubscribers(WidgetEvent::VisibilityChange);
	}
	else if (HiddenFlag)
	{
		HiddenFlag = false;
		NotifySubscribers(WidgetEvent::VisibilityChange);
		Update();
	}
}

void Widget::ShowFullscreen()
{
	if (Type != WidgetType::Child)
	{
		CheckInitialShow();
		DispWindow->ShowFullscreen();
	}
}

bool Widget::IsFullscreen()
{
	if (Type != WidgetType::Child)
	{
		return DispWindow->IsWindowFullscreen();
	}
	return false;
}

void Widget::ShowMaximized()
{
	if (Type != WidgetType::Child)
	{
		CheckInitialShow();
		DispWindow->ShowMaximized();
	}
}

void Widget::ShowMinimized()
{
	if (Type != WidgetType::Child)
	{
		CheckInitialShow();
		DispWindow->ShowMinimized();
	}
}

void Widget::ShowNormal()
{
	if (Type != WidgetType::Child)
	{
		CheckInitialShow();
		DispWindow->ShowNormal();
	}
}

void Widget::Hide()
{
	if (Type != WidgetType::Child)
	{
		if (DispWindow)
		{
			DispWindow->Hide();
			NotifySubscribers(WidgetEvent::VisibilityChange);
		}
	}
	else if (!HiddenFlag)
	{
		HiddenFlag = true;
		NotifySubscribers(WidgetEvent::VisibilityChange);
		Update();
	}
}

void Widget::ActivateWindow()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->Activate();
	}
}

void Widget::Close()
{
	OnClose();
}

void Widget::SetWindowBackground(const Colorf& color)
{
	Widget* w = Window();
	if (w && w->WindowBackground != color)
	{
		w->WindowBackground = color;
		Update();
	}
}

void Widget::SetWindowBorderColor(const Colorf& color)
{
	Widget* w = Window();
	if (w)
	{
		w->DispWindow->SetBorderColor(color.toBgra8());
		w->DispWindow->Update();
	}
}

void Widget::SetWindowCaptionColor(const Colorf& color)
{
	Widget* w = Window();
	if (w)
	{
		w->DispWindow->SetCaptionColor(color.toBgra8());
		w->DispWindow->Update();
	}
}

void Widget::SetWindowCaptionTextColor(const Colorf& color)
{
	Widget* w = Window();
	if (w)
	{
		w->DispWindow->SetCaptionTextColor(color.toBgra8());
		w->DispWindow->Update();
	}
}

void Widget::Update()
{
	Widget* w = Window();
	if (w)
	{
		w->DispWindow->Update();
	}
}

void Widget::Repaint()
{
	Widget* w = Window();
	if (!w || !w->DispCanvas)
		return;

	Canvas* canvas = w->DispCanvas.get();
	canvas->begin(w->WindowBackground);
	w->Paint(canvas);
	canvas->end();
}

void Widget::Paint(Canvas* canvas)
{
	Point oldOrigin = canvas->getOrigin();
	canvas->pushClip(FrameGeometry);
	canvas->setOrigin(oldOrigin + FrameGeometry.topLeft());
	OnPaintFrame(canvas);
	canvas->setOrigin(oldOrigin);
	canvas->popClip();

	canvas->pushClip(ContentGeometry);
	canvas->setOrigin(oldOrigin + ContentGeometry.topLeft());
	OnPaint(canvas);
	for (Widget* w = FirstChild(); w != nullptr; w = w->NextSibling())
	{
		if (w->Type == WidgetType::Child && !w->HiddenFlag)
			w->Paint(canvas);
	}
	canvas->setOrigin(oldOrigin);
	canvas->popClip();
}

void Widget::OnPaintFrame(Canvas* canvas)
{
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	if (style)
	{
		style->Paint(this, canvas, GetFrameGeometry().size());
	}
}

bool Widget::GetKeyState(InputKey key)
{
	Widget* window = Window();
	return window ? window->DispWindow->GetKeyState(key) : false;
}

bool Widget::HasFocus()
{
	Widget* window = Window();
	return window ? window->FocusWidget == this : false;
}

bool Widget::IsEnabled()
{
	return true;
}

bool Widget::IsHidden()
{
	return !IsVisible();
}

bool Widget::IsVisible()
{
	if (Type != WidgetType::Child)
	{
		return true; // DispWindow->IsVisible();
	}
	else
	{
		return !HiddenFlag;
	}
}

void Widget::SetFocus()
{
	Widget* window = Window();
	if (window && window->FocusWidget != this)
	{
		if (window->FocusWidget)
			window->FocusWidget->OnLostFocus();
		window->FocusWidget = this;
		window->FocusWidget->OnSetFocus();
		window->ActivateWindow();
	}
}

void Widget::SetEnabled(bool value)
{
}

void Widget::LockKeyboard()
{
	Widget* w = Window();
	if (w && w->KeyboardLockWidget != this)
	{
		w->KeyboardLockWidget = this;
		w->DispWindow->LockKeyboard();
	}
}

void Widget::UnlockKeyboard()
{
	Widget* w = Window();
	if (w && w->KeyboardLockWidget != nullptr)
	{
		w->KeyboardLockWidget = nullptr;
		w->DispWindow->UnlockKeyboard();
	}
}

void Widget::LockCursor()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != this)
	{
		w->CursorLockWidget = this;
		w->DispWindow->LockCursor();
	}
}

void Widget::UnlockCursor()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != nullptr)
	{
		w->CursorLockWidget = nullptr;
		w->DispWindow->UnlockCursor();
	}
}

void Widget::SetCursor(StandardCursor cursor)
{
	if (CurrentCursor != cursor)
	{
		CurrentCursor = cursor;
		if (HoverWidget == this || CursorLockWidget == this)
		{
			Widget* w = Window();
			if (w)
			{
				w->DispWindow->SetCursor(CurrentCursor, CurrentCustomCursor);
			}
		}
	}
}

void Widget::SetCursor(std::shared_ptr<CustomCursor> cursor)
{
	if (CurrentCustomCursor != cursor)
	{
		CurrentCustomCursor = cursor;
		if (HoverWidget == this || CursorLockWidget == this)
		{
			Widget* w = Window();
			if (w)
			{
				w->DispWindow->SetCursor(CurrentCursor, CurrentCustomCursor);
			}
		}
	}
}

void Widget::SetPointerCapture()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != this)
	{
		w->CursorLockWidget = this;
		w->DispWindow->CaptureMouse();
	}
}

void Widget::ReleasePointerCapture()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != nullptr)
	{
		w->CursorLockWidget = nullptr;
		w->DispWindow->ReleaseMouseCapture();
	}
}

void Widget::SetModalCapture()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != this)
	{
		w->CursorLockWidget = this;
	}
}

void Widget::ReleaseModalCapture()
{
	Widget* w = Window();
	if (w && w->CursorLockWidget != nullptr)
	{
		w->CursorLockWidget = nullptr;
	}
}

std::string Widget::GetClipboardText()
{
	Widget* w = Window();
	if (w)
		return w->DispWindow->GetClipboardText();
	else
		return {};
}

void Widget::SetClipboardText(const std::string& text)
{
	Widget* w = Window();
	if (w)
		w->DispWindow->SetClipboardText(text);
}

Widget* Widget::Window() const
{
	for (const Widget* w = this; w != nullptr; w = w->Parent())
	{
		if (w->DispWindow)
			return const_cast<Widget*>(w);
	}
	return nullptr;
}

Canvas* Widget::GetCanvas() const
{
	for (const Widget* w = this; w != nullptr; w = w->Parent())
	{
		if (w->DispCanvas)
			return w->DispCanvas.get();
	}

	struct DummyCanvas
	{
		DummyCanvas()
		{
			canvas = Canvas::create();
			canvas->attach(nullptr);
		}
		std::unique_ptr<Canvas> canvas;
	};

	static DummyCanvas dummy;
	return dummy.canvas.get();
}

bool Widget::IsParent(const Widget* w) const
{
	while (w)
	{
		w = w->Parent();
		if (w == this)
			return true;
	}
	return false;
}

bool Widget::IsChild(const Widget* w) const
{
	if (!w)
		return false;
	return w->IsParent(this);
}

Widget* Widget::ChildAt(const Point& pos)
{
	for (Widget* cur = LastChild(); cur != nullptr; cur = cur->PrevSibling())
	{
		if (cur->Type == WidgetType::Child && !cur->HiddenFlag && cur->FrameGeometry.contains(pos))
		{
			Widget* cur2 = cur->ChildAt(pos - cur->ContentGeometry.topLeft());
			return cur2 ? cur2 : cur;
		}
	}
	return nullptr;
}

Point Widget::MapFrom(const Widget* parent, const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur == parent)
			return p;
		p -= cur->ContentGeometry.topLeft();
	}
	throw std::runtime_error("MapFrom: not a parent of widget");
}

Point Widget::MapFromGlobal(const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur->DispWindow)
		{
			return cur->DispWindow->MapFromGlobal(p);
		}
		p -= cur->ContentGeometry.topLeft();
	}
	throw std::runtime_error("MapFromGlobal: no window widget found");
}

Point Widget::MapTo(const Widget* parent, const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur == parent)
			return p;
		p += cur->ContentGeometry.topLeft();
	}
	throw std::runtime_error("MapTo: not a parent of widget");
}

Point Widget::MapToGlobal(const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur->DispWindow)
		{
			return cur->DispWindow->MapToGlobal(p);
		}
		p += cur->ContentGeometry.topLeft();
	}
	throw std::runtime_error("MapFromGlobal: no window widget found");
}

void Widget::OnWindowPaint()
{
	Repaint();
}

Widget* Widget::CommonAncestor(Widget* a, Widget* b)
{
	if (a == b)
		return a;

	std::vector<Widget*> list1;
	std::vector<Widget*> list2;
	list1.reserve(16);
	list2.reserve(16);
	for (Widget* w = a; w != nullptr; w = w->Parent())
		list1.push_back(w);
	for (Widget* w = b; w != nullptr; w = w->Parent())
		list2.push_back(w);

	if (list1.empty() || list2.empty() || list1.back() != list2.back())
		return nullptr;

	auto it1 = list1.rbegin();
	auto it2 = list2.rbegin();
	while (it1 != list1.rend() && it2 != list2.rend())
	{
		if (*it1 != *it2)
		{
			return *(--it1);
		}
		++it1;
		++it2;
	}

	if (it1 == list1.rend())
		return *(--it1);
	else if (it2 == list2.rend())
		return *(--it2);

	return nullptr;
}

void Widget::OnWindowMouseMove(const Point& pos)
{
	Point contentPos = pos - ContentGeometry.topLeft();
	if (CursorLockWidget)
	{
		DispWindow->SetCursor(CursorLockWidget->CurrentCursor, CursorLockWidget->CurrentCustomCursor);
		CursorLockWidget->OnMouseMove(CursorLockWidget->MapFrom(this, contentPos));
	}
	else
	{
		Widget* widget = ChildAt(contentPos);
		if (!widget)
			widget = this;

		if (HoverWidget != widget)
		{
			if (HoverWidget)
			{
				if (Widget* ancestor = CommonAncestor(HoverWidget, widget))
				{
					for (Widget* w = HoverWidget; w != ancestor; w = w->Parent())
					{
						w->OnMouseLeave();
					}
				}
			}
			HoverWidget = widget;
		}

		DispWindow->SetCursor(widget->CurrentCursor, widget->CurrentCustomCursor);

		do
		{
			widget->OnMouseMove(widget->MapFrom(this, contentPos));
			if (widget == this)
				break;
			widget = widget->Parent();
		} while (widget);
	}
}

void Widget::OnWindowMouseLeave()
{
	if (HoverWidget)
	{
		for (Widget* w = HoverWidget; w; w = w->Parent())
		{
			w->OnMouseLeave();
		}
		HoverWidget = nullptr;
	}
}

void Widget::OnWindowMouseDown(const Point& pos, InputKey key)
{
	Point contentPos = pos - ContentGeometry.topLeft();
	if (CursorLockWidget)
	{
		CursorLockWidget->OnMouseDown(CursorLockWidget->MapFrom(this, contentPos), key);
	}
	else
	{
		Widget* widget = ChildAt(contentPos);
		if (!widget)
			widget = this;
		while (widget)
		{
			bool stopPropagation = widget->OnMouseDown(widget->MapFrom(this, contentPos), key);
			if (stopPropagation || widget == this)
				break;
			widget = widget->Parent();
		}
	}
}

void Widget::OnWindowMouseDoubleclick(const Point& pos, InputKey key)
{
	Point contentPos = pos - ContentGeometry.topLeft();
	if (CursorLockWidget)
	{
		CursorLockWidget->OnMouseDoubleclick(CursorLockWidget->MapFrom(this, contentPos), key);
	}
	else
	{
		Widget* widget = ChildAt(contentPos);
		if (!widget)
			widget = this;
		while (widget)
		{
			bool stopPropagation = widget->OnMouseDoubleclick(widget->MapFrom(this, contentPos), key);
			if (stopPropagation || widget == this)
				break;
			widget = widget->Parent();
		}
	}
}

void Widget::OnWindowMouseUp(const Point& pos, InputKey key)
{
	Point contentPos = pos - ContentGeometry.topLeft();
	if (CursorLockWidget)
	{
		CursorLockWidget->OnMouseUp(CursorLockWidget->MapFrom(this, contentPos), key);
	}
	else
	{
		Widget* widget = ChildAt(contentPos);
		if (!widget)
			widget = this;
		while (widget)
		{
			bool stopPropagation = widget->OnMouseUp(widget->MapFrom(this, contentPos), key);
			if (stopPropagation || widget == this)
				break;
			widget = widget->Parent();
		}
	}
}

void Widget::OnWindowMouseWheel(const Point& pos, InputKey key)
{
	Point contentPos = pos - ContentGeometry.topLeft();
	if (CursorLockWidget)
	{
		CursorLockWidget->OnMouseWheel(CursorLockWidget->MapFrom(this, contentPos), key);
	}
	else
	{
		Widget* widget = ChildAt(contentPos);
		if (!widget)
			widget = this;
		while (widget)
		{
			bool stopPropagation = widget->OnMouseWheel(widget->MapFrom(this, contentPos), key);
			if (stopPropagation || widget == this)
				break;
			widget = widget->Parent();
		}
	}
}

void Widget::OnWindowRawKey(RawKeycode keycode, bool down)
{
	if (KeyboardLockWidget)
	{
		KeyboardLockWidget->OnRawKey(keycode, down);
	}
}

void Widget::OnWindowRawMouseMove(int dx, int dy)
{
	if (CursorLockWidget)
	{
		CursorLockWidget->OnRawMouseMove(dx, dy);
	}
}

void Widget::OnWindowKeyChar(std::string chars)
{
	if (FocusWidget)
		FocusWidget->OnKeyChar(chars);
}

void Widget::OnWindowKeyDown(InputKey key)
{
	if (FocusWidget)
		FocusWidget->OnKeyDown(key);
}

void Widget::OnWindowKeyUp(InputKey key)
{
	if (FocusWidget)
		FocusWidget->OnKeyUp(key);
}

void Widget::OnWindowGeometryChanged()
{
	if (!DispWindow)
		return;
	Size size = DispWindow->GetClientSize();
	FrameGeometry = Rect::xywh(0.0, 0.0, size.width, size.height);

	double left = FrameGeometry.left() + GetNoncontentLeft();
	double top = FrameGeometry.top() + GetNoncontentTop();
	double right = FrameGeometry.right() - GetNoncontentRight();
	double bottom = FrameGeometry.bottom() - GetNoncontentBottom();
	left = std::min(left, FrameGeometry.right());
	top = std::min(top, FrameGeometry.bottom());
	right = std::max(right, FrameGeometry.left());
	bottom = std::max(bottom, FrameGeometry.top());
	left = GridFitPoint(left);
	top = GridFitPoint(top);
	right = GridFitPoint(right);
	bottom = GridFitPoint(bottom);
	ContentGeometry = Rect::ltrb(left, top, std::max(left, right), std::max(bottom, top));

	if (m_Layout)
		m_Layout->OnGeometryChanged();

	OnGeometryChanged();
}

void Widget::OnWindowClose()
{
	Close();
}

void Widget::OnWindowActivated()
{
}

void Widget::OnWindowDeactivated()
{
}

void Widget::OnWindowDpiScaleChanged()
{
}

double Widget::GetDpiScale() const
{
	Widget* w = Window();
	return w ? w->DispWindow->GetDpiScale() : 1.0;
}

double Widget::GridFitPoint(double p) const
{
	double dpiscale = GetDpiScale();
	return std::round(p * dpiscale) / dpiscale;
}

double Widget::GridFitSize(double s) const
{
	if (s <= 0.0)
		return 0.0;
	double dpiscale = GetDpiScale();
	return std::max(std::floor(s * dpiscale + 0.25), 1.0) / dpiscale;
}

Size Widget::GetScreenSize()
{
	return DisplayWindow::GetScreenSize();
}

void* Widget::GetNativeHandle()
{
	Widget* w = Window();
	return w ? w->DispWindow->GetNativeHandle() : nullptr;
}

int Widget::GetNativePixelWidth()
{
	Widget* w = Window();
	return w ? w->DispWindow->GetPixelWidth() : 0;
}

int Widget::GetNativePixelHeight()
{
	Widget* w = Window();
	return w ? w->DispWindow->GetPixelHeight() : 0;
}

void Widget::SetStyleClass(const std::string& themeClass)
{
	if (StyleClass != themeClass)
	{
		StyleClass = themeClass;
		Update();
	}
}

void Widget::SetStyleState(const std::string& state)
{
	if (StyleState != state)
	{
		StyleState = state;
		Update();
	}
}

void Widget::SetStyleBool(const std::string& propertyName, bool value)
{
	StyleProperties[propertyName] = value;
}

void Widget::SetStyleInt(const std::string& propertyName, int value)
{
	StyleProperties[propertyName] = value;
}

void Widget::SetStyleDouble(const std::string& propertyName, double value)
{
	StyleProperties[propertyName] = value;
}

void Widget::SetStyleString(const std::string& propertyName, const std::string& value)
{
	StyleProperties[propertyName] = value;
}

void Widget::SetStyleColor(const std::string& propertyName, const Colorf& value)
{
	StyleProperties[propertyName] = value;
}

void Widget::SetStyleImage(const std::string& propertyName, const std::shared_ptr<Image>& value)
{
	StyleProperties[propertyName] = value;
}

bool Widget::GetStyleBool(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<bool>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetBool(StyleState, propertyName) : false;
}

int Widget::GetStyleInt(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<int>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetInt(StyleState, propertyName) : 0;
}

double Widget::GetStyleDouble(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<double>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetDouble(StyleState, propertyName) : 0.0;
}

std::string Widget::GetStyleString(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<std::string>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetString(StyleState, propertyName) : std::string();
}

Colorf Widget::GetStyleColor(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<Colorf>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetColor(StyleState, propertyName) : Colorf::transparent();
}

std::shared_ptr<Image> Widget::GetStyleImage(const std::string& propertyName) const
{
	auto it = StyleProperties.find(propertyName);
	if (it != StyleProperties.end())
		return std::get<std::shared_ptr<Image>>(it->second);
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetImage(StyleState, propertyName) : std::shared_ptr<Image>();
}

std::shared_ptr<Font> Widget::GetFont() const
{
	WidgetStyle* style = WidgetTheme::GetTheme()->GetStyle(StyleClass);
	return style ? style->GetFont(StyleState) : std::shared_ptr<Font>();
}
