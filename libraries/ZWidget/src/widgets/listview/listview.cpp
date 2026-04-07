#include "widgets/listview/listview.h"
#include "widgets/scrollbar/scrollbar.h"
#include <algorithm>

ListView::ListView(Widget* parent) : Widget(parent)
{
	SetStretching(true);
	SetStyleClass("listview");

	scrollbar = new Scrollbar(this);
	scrollbar->FuncScroll = [this]() { OnScrollbarScroll(); };

	header = new ListViewHeader(this);
	header->SetVisible(false);

	body = new ListViewBody(this);
}

int ListView::GetColumnCount() const
{
	return header->GetColumnCount();
}

double ListView::GetColumnWidth(int index) const
{
	if (index >= 0 && index < header->GetColumnCount())
		return header->GetColumnWidth(index);
	return 0.0;
}

const std::string& ListView::GetColumnText(int index) const
{
	if (index >= 0 && index < header->GetColumnCount())
		return header->GetColumnText(index);
	static std::string empty;
	return empty;
}

void ListView::ClearColumns()
{
	if (header->GetColumnCount() != 0)
	{
		header->Clear();
		header->SetVisible(false);
		OnGeometryChanged();
		Update();
	}
}

void ListView::SetColumn(int index, const std::string& text, double width)
{
	header->SetColumn(index, text, width);
	OnGeometryChanged();
	Update();
}

void ListView::ShowHeader(bool value)
{
	if (header->IsVisible() != value)
	{
		header->SetVisible(value);
		OnGeometryChanged();
		Update();
	}
}

void ListView::AddItem(std::initializer_list<std::string> columns, int index)
{
	auto item = std::make_unique<ListViewItem>();
	item->columns = columns;

	if (index < 0)
		items.push_back(std::move(item));
	else
		items.insert(items.begin() + index, std::move(item));

	scrollbar->SetRanges(body->GetHeight(), items.size() * body->GetItemHeight());
	Update();
}

void ListView::AddItem(const std::string& text, int index, int column)
{
	auto item = std::make_unique<ListViewItem>();
	item->columns.resize(column + 1);
	item->columns[column] = text;

	if (index < 0)
		items.push_back(std::move(item));
	else
		items.insert(items.begin() + index, std::move(item));

	scrollbar->SetRanges(body->GetHeight(), items.size() * body->GetItemHeight());
	Update();
}

void ListView::UpdateItem(const std::string& text, int index, int column)
{
	if (index < 0 || (size_t)index >= items.size() || column < 0)
		return;

	ListViewItem* item = items[index].get();
	item->columns.resize(std::max((size_t)column + 1, item->columns.size()));
	item->columns[column] = text;

	Update();
}

void ListView::RemoveItem(int index)
{
	if (!items.size() || index >= (int)items.size())
		return;

	if (index < 0)
		index = static_cast<int>(items.size()) - 1;

	if (selectedItem == index)
		SetSelectedItem(0);

	items.erase(items.begin() + index);
	scrollbar->SetRanges(body->GetHeight(), items.size() * body->GetItemHeight());
	Update();
}

void ListView::Activate()
{
	if (OnActivated)
		OnActivated();
}

void ListView::SetSelectedItem(int index, bool notify)
{
	if (selectedItem != index && index >= 0 && index < (int)items.size())
	{
		selectedItem = index;
		Update();
		if (notify && OnChanged)
			OnChanged(selectedItem);
	}
}

void ListView::ScrollToItem(int index)
{
	double itemHeight = body->GetItemHeight();
	double y = itemHeight * index;
	if (y < scrollbar->GetPosition())
	{
		scrollbar->SetPosition(y);
	}
	else if (y + itemHeight > scrollbar->GetPosition() + GetHeight())
	{
		scrollbar->SetPosition(std::max(y + itemHeight - GetHeight(), 0.0));
	}
}

void ListView::OnScrollbarScroll()
{
	Update();
}

double ListView::GetHeaderHeight()
{
	if (header->IsVisible())
	{
		return header->GetPreferredHeight() + header->GetNoncontentTop() + header->GetNoncontentBottom();
	}
	else
	{
		return 0.0;
	}
}

void ListView::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();
	double hh = GetHeaderHeight();
	double sw = scrollbar->GetPreferredWidth();
	header->SetFrameGeometry(Rect::xywh(0.0, 0.0, w - sw, hh));
	body->SetFrameGeometry(Rect::xywh(0.0, hh, w - sw, h - hh));
	scrollbar->SetFrameGeometry(Rect::xywh(w - sw, 0.0, sw, h));
	scrollbar->SetRanges(body->GetHeight(), items.size() * body->GetItemHeight());
}

double ListView::GetPreferredWidth()
{
	double total = 0.0;

	if (header->GetColumnCount() != 0)
	{
		for (int i = 0, count = header->GetColumnCount(); i < count; i++)
		{
			total += header->GetColumnWidth(i);
		}
	}
	else
	{
		auto canvas = GetCanvas();
		auto font = GetFont();
		for (const auto& row : items)
		{
			double wRow = 0.0;
			for (const auto& cell : row->columns)
			{
				wRow += canvas->measureText(font, cell).width;
			}
			total = std::max(wRow, total);
		}
	}

	return total + 10.0*2 + scrollbar->GetPreferredWidth();
}

double ListView::GetPreferredHeight()
{
	return items.size()*20.0 + 10.0*2; // Items plus top/bottom padding
}

double ListView::GetMinimumHeight()
{
	return 20.0 + 10.0*2; // One item plus top/bottom padding
}

void ListView::OnKeyDown(InputKey key)
{
	if (key == InputKey::Down)
	{
		if (selectedItem + 1 < (int)items.size())
		{
			SetSelectedItem(selectedItem + 1);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::Up)
	{
		if (selectedItem > 0)
		{
			SetSelectedItem(selectedItem - 1);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::Enter)
	{
		Activate();
	}
	else if (key == InputKey::Home)
	{
		if (selectedItem > 0)
		{
			SetSelectedItem(0);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::End)
	{
		if (selectedItem + 1 < (int)items.size())
		{
			SetSelectedItem((int)items.size() - 1);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::PageUp)
	{
		double h = GetHeight();
		if (h <= 0.0 || items.empty())
			return;
		int itemsPerPage = (int)std::max(std::round(h / body->GetItemHeight()), 1.0);
		int nextItem = std::max(selectedItem - itemsPerPage, 0);
		if (nextItem != selectedItem)
		{
			SetSelectedItem(nextItem);
			ScrollToItem(selectedItem);
		}
	}
	else if (key == InputKey::PageDown)
	{
		double h = GetHeight();
		if (h <= 0.0 || items.empty())
			return;
		int itemsPerPage = (int)std::max(std::round(h / body->GetItemHeight()), 1.0);
		int prevItem = std::min(selectedItem + itemsPerPage, (int)items.size() - 1);
		if (prevItem != selectedItem)
		{
			SetSelectedItem(prevItem);
			ScrollToItem(selectedItem);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

ListViewBody::ListViewBody(ListView* parent) : Widget(parent), listview(parent)
{
	SetStyleClass("listview-body");
}

double ListViewBody::GetItemHeight()
{
	return 20.0;
}

void ListViewBody::OnKeyDown(InputKey key)
{
	listview->OnKeyDown(key);
}

void ListViewBody::OnPaint(Canvas* canvas)
{
	double x = 2.0;
	double w = GetWidth();
	double itemHeight = GetItemHeight();

	Colorf textColor = GetStyleColor("color");
	Colorf selectionColor = GetStyleColor("selection-color");
	auto font = GetFont();

	double y = -listview->scrollbar->GetPosition();

	int itemIndex = 0;
	for (const std::unique_ptr<ListViewItem>& item : listview->items)
	{
		double itemY = y;
		if (itemY + itemHeight >= 0.0 && itemY < GetHeight())
		{
			if (itemIndex == listview->selectedItem)
			{
				canvas->fillRect(Rect::xywh(x - 2.0, itemY, w, itemHeight), selectionColor);
			}
			double cx = x;
			int colCount = std::min((int)item->columns.size(), listview->header->GetColumnCount());
			for (int colIndex = 0; colIndex < colCount; ++colIndex)
			{
				double colwidth = listview->header->GetColumnWidth(colIndex);
				if (colIndex + 1 == listview->header->GetColumnCount())
					colwidth = std::max(w - cx, 0.0);
				canvas->pushClip(Rect::xywh(cx, itemY, std::max(colwidth - 5.0, 0.0), itemHeight));
				canvas->drawText(font, Point(cx, y + 15.0), item->columns[colIndex], textColor);
				canvas->popClip();
				cx += colwidth;
			}
		}
		y += itemHeight;
		itemIndex++;
	}
}

bool ListViewBody::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();

	if (key == InputKey::LeftMouse)
	{
		int index = (int)((pos.y + listview->scrollbar->GetPosition()) / GetItemHeight());
		if (index >= 0 && (size_t)index < listview->items.size())
		{
			listview->ScrollToItem(index);
			listview->SetSelectedItem(index);
		}
	}
	return true;
}

bool ListViewBody::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	if (key == InputKey::LeftMouse)
	{
		listview->Activate();
	}
	return true;
}

bool ListViewBody::OnMouseWheel(const Point& pos, InputKey key)
{
	if (key == InputKey::MouseWheelUp)
	{
		listview->scrollbar->SetPosition(std::max(listview->scrollbar->GetPosition() - GetItemHeight(), 0.0));
	}
	else if (key == InputKey::MouseWheelDown)
	{
		listview->scrollbar->SetPosition(std::min(listview->scrollbar->GetPosition() + GetItemHeight(), listview->scrollbar->GetMax()));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

ListViewHeader::ListViewHeader(ListView* parent) : Widget(parent)
{
	SetStyleClass("listview-header");
	Clear();
}

void ListViewHeader::Clear()
{
	columns.clear();
	columns.push_back({});
}

void ListViewHeader::SetColumn(int index, const std::string& text, double width)
{
	if (index < 0)
		return;
	if (columns.size() <= (size_t)index)
		columns.resize(index + 1);

	Column col;
	col.text = text;
	col.width = width;
	columns[index] = std::move(col);
}

void ListViewHeader::OnPaint(Canvas* canvas)
{
	Colorf textColor = GetStyleColor("color");
	auto font = GetFont();
	double cx = 0.0;
	for (size_t idx = 0; idx < columns.size(); idx++)
	{
		canvas->drawText(font, Point(cx, 15.0), columns[idx].text, textColor);
		cx += columns[idx].width;
	}
}

double ListViewHeader::GetPreferredHeight()
{
	return 20.0;
}
