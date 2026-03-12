
#pragma once

#include "../../core/widget.h"
#include <vector>
#include <functional>
#include <initializer_list>

class Scrollbar;
class Dropdown;
class ListViewItem;
class ListViewHeader;
class ListViewBody;

class ListView : public Widget
{
public:
	ListView(Widget* parent = nullptr);

	void ClearColumns();
	void SetColumn(int index, const std::string& text, double width);
	void ShowHeader(bool value);

	int GetColumnCount() const;
	double GetColumnWidth(int index) const;
	const std::string& GetColumnText(int index) const;

	void AddItem(std::initializer_list<std::string> columns, int index = -1);
	void AddItem(const std::string& text, int index = -1, int column = 0);
	void UpdateItem(const std::string& text, int index, int column = 0);
	void RemoveItem(int index = -1);

	size_t GetItemCount() const { return items.size(); }

	int GetSelectedItem() const { return selectedItem; }
	void SetSelectedItem(int index, bool notify = true);

	void ScrollToItem(int index);

	double GetPreferredWidth() override;
	double GetPreferredHeight() override;
	double GetMinimumHeight();

	void Activate();

	std::function<void(int)> OnChanged;
	std::function<void()> OnActivated;

protected:
	void OnGeometryChanged() override;
	void OnKeyDown(InputKey key) override;
	void OnScrollbarScroll();

private:
	double GetHeaderHeight();

	ListViewHeader* header = nullptr;
	ListViewBody* body = nullptr;
	Scrollbar* scrollbar = nullptr;

	std::vector<std::unique_ptr<ListViewItem>> items;
	int selectedItem = 0;

	friend class ListViewBody;
};

class ListViewBody : public Widget
{
public:
	ListViewBody(ListView* parent);

	static double GetItemHeight();

private:
	void OnPaint(Canvas* canvas) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseDoubleclick(const Point& pos, InputKey key) override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;
	void OnKeyDown(InputKey key) override;

	ListView* listview = nullptr;
};

class ListViewHeader : public Widget
{
public:
	ListViewHeader(ListView* parent);

	void OnPaint(Canvas* canvas) override;

	void Clear();
	void SetColumn(int index, const std::string& text, double width);

	int GetColumnCount() const { return (int)columns.size(); }
	double GetColumnWidth(int index) const { return columns[index].width; }
	const std::string& GetColumnText(int index) const { return columns[index].text; }

	double GetPreferredHeight() override;

private:
	struct Column
	{
		std::string text;
		double width = 0.0;
	};

	std::vector<Column> columns;
};

class ListViewItem
{
public:
	std::vector<std::string> columns;
};
