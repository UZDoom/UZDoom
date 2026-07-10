/*
** updatebuttonbar.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

//ugh something is including windows.h, probably curl? disable parts of it to minimize conflicts
#ifndef NOMINMAX
	#define NOMINMAX // mingw already defines NOMINMAX??????
#endif

#define WIN32_LEAN_AND_MEAN

#include "serializer_rapidjson.h"

#include "updatebuttonbar.h"
#include "launcherwindow.h"
#include "gstrings.h"
#include "c_cvars.h"
#include "m_misc.h"
#include "i_net.h"
#include "engineerrors.h"
#include "widgets/themedata.h"
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include "settingspage.h"
#include <ctime>
#include "curl_loader_internal.h"
#include <algorithm>
#include <format>
#include <thread>
#include <mutex>
#include <ranges>
#include <filesystem>
#include "filesystem.h"
#include "cmdlib.h"
#include "zstring.h"
#ifdef _WIN32
	#include <shellapi.h>
#endif

CVAR(String, updater_cached_update, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(String, updater_skipped_update, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(String, updater_last_update_check, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Bool, updater_check_updates_initialized, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Int, updater_update_interval, 7, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG); // by default, check once per week
CVAR(Bool, updater_auto_updates, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG);
CVAR(Bool, updater_check_updates, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG);
CVAR(Bool, updater_debug_always_update, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Bool, updater_debug_throttle_download, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);

static uint64_t daysToSeconds(uint64_t days)
{
	return days * 86400;
}

static uint64_t getCurrentDate()
{
	time_t t;
	time(&t); //linux might need changing this to time64, maybe?
	return (t / 86400ULL) * 86400ULL; // round to whole day
}

static uint64_t parseDate(FString str)
{
	return str.ToULong();
}

static std::vector<std::string> SplitNewLines(const char * str, size_t len)
{
	TArray<FString> s = FString(str, len).SplitNewLines(60, 70);
	std::vector<std::string> ret(s.size());

	for(int i = 0; i < s.size(); i++)
	{
		ret[i] = std::string(s[i].GetChars(), s[i].Len());
	}

	return ret;
}

static std::vector<std::string> SplitNewLines(std::string str)
{
	return SplitNewLines(str.c_str(), str.length());
}

class PopupBase;

std::unique_ptr<PopupBase> currentPopup;

enum PopupButtonActionFlags
{
	ACTIONF_FLOAT_RIGHT = 0x1,
};

struct PopupButtonAction
{
	std::string text;
	std::function<void(PopupBase&)> action;
	int flags = 0;
	int row = 0;
};

enum PopupFlags
{
	POPUPF_DISALLOW_CLOSE =			0x1,
	POPUPF_JUSTIFY_BUTTONS =		0x2,
	POPUPF_LAST_BUTTON_ALIGN_LEFT =	0x4,
	POPUPF_CENTER_BUTTONS =			0x8,
};

class PopupBase : public Widget
{ // TODO move popups to their own header/file so that more things can use them, and change it so that multiple popups can open and stack, instead of just overwriting one another
public:
	using ActionListType = std::vector<PopupButtonAction>;
	virtual ~PopupBase() = default;

protected:
	std::vector<std::unique_ptr<Widget>> cleanup;
	bool allowCloseButton;

	using Widget::Widget;

	double windowHeight;
	double windowWidth;

	virtual void OnClose() override
	{
		ReleaseModalCapture(true);
		currentPopup = nullptr;
	}

	virtual void OnWindowClose() override
	{
		if(allowCloseButton) Close();
	}

public:
	static ActionListType ConcatActions(const ActionListType &a, const ActionListType &b)
	{
		ActionListType tmp;
		tmp.insert(tmp.end(), a.begin(), a.end());
		tmp.insert(tmp.end(), b.begin(), b.end());
		return tmp;
	}
	static std::vector<std::string> ConcatText(const std::vector<std::string> &a, const std::vector<std::string> &b)
	{
		std::vector<std::string> tmp;
		tmp.insert(tmp.end(), a.begin(), a.end());
		tmp.insert(tmp.end(), b.begin(), b.end());
		return tmp;
	}
};

template<class Derived>
class ChoicePopup : public PopupBase
{
protected:
	int GetExtraHeight() { return 0; }

	void AddExtraElements(int top) {}

	std::vector<TextLabel *> text;
public:
	ChoicePopup(Widget * parent, const std::string &title, const std::vector<std::string> &text, const PopupBase::ActionListType &actions, double _windowWidth, int flags)
		: PopupBase(parent->Window(), WidgetType::Utility, RenderAPI::Unspecified, { .resizable = false })
	{
		allowCloseButton = !(flags & POPUPF_DISALLOW_CLOSE);

		Size screenSize = GetScreenSize();

		int extraHeight = static_cast<Derived*>(this)->GetExtraHeight();

		windowWidth = _windowWidth;

		windowHeight = text.size() > 0 ? 90.0 + (20 * text.size()): 80.0;

		int numrows = 1;

		for(auto &act : actions)
		{
			numrows = std::max(act.row + 1, numrows);
		}

		if(actions.size() == 0)
		{
			windowHeight -= 40;
		}
		else if(numrows > 1)
		{
			windowHeight += (numrows - 1) * 35;
		}

		if(extraHeight > 0)
		{
			windowHeight += extraHeight + 10;
		}

		SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);

		SetWindowTitle(GStrings.GetString(title));

		if(text.size() > 0)
		{
			int top = 5;

			for(int i = 0; i < text.size(); i++)
			{
				TextLabel* text_widget = new TextLabel(this);

				text_widget->SetText(text[i]);

				text_widget->SetFrameGeometry(0, top, windowWidth, 20);
				text_widget->SetTextAlignment(TextLabelAlignment::Center);

				cleanup.push_back(std::unique_ptr<Widget>{text_widget});
				this->text.push_back(text_widget);

				top += 20;
			}

			static_cast<Derived*>(this)->AddExtraElements(top + 5);
		}
		else
		{
			static_cast<Derived*>(this)->AddExtraElements(5);
		}

		for(int i = 0; i < numrows; i++)
		{
			std::vector<PushButton *> btns;

			double totalwidth = 0.0;
			double buttonHeight = 30.0;
			double rowHeight = 35.0 * (numrows - i);

			for(auto &act : actions)
			{
				if(act.row != i) continue;

				PushButton * btn = new PushButton(this);

				btn->SetText(GStrings.GetString(act.text));

				btn->OnClick = [this, act]()
				{
					act.action(*this);
				};

				btns.push_back(btn);

				cleanup.push_back(std::unique_ptr<Widget>{btn});

				totalwidth += btn->GetPreferredWidth();
			}

			if(flags & POPUPF_JUSTIFY_BUTTONS)
			{
				int left = 5;
				int count = 0;

				int padding = ((GetWidth() - 10) - totalwidth)/btns.size();

				for(auto &btn : btns)
				{
					count++;

					int len = btn->GetPreferredWidth();

					if(count == btns.size())
					{
						btn->SetFrameGeometry(GetWidth() - (len + 5), GetHeight() - rowHeight, len, buttonHeight);
					}
					else
					{
						btn->SetFrameGeometry(left, GetHeight() - rowHeight, len, buttonHeight);
					}

					left += (len + padding);
				}
			}
			else
			{
				int left = (flags & POPUPF_CENTER_BUTTONS) ? (GetWidth() - (totalwidth + 5 * btns.size())) / 2 : 5;
				int count = 0;

				bool ignore_right_align = (flags & POPUPF_CENTER_BUTTONS);

				bool float_last_right = !(flags & POPUPF_LAST_BUTTON_ALIGN_LEFT);

				for(auto &btn : btns)
				{
					bool align_right = !ignore_right_align && ((actions[count].flags & ACTIONF_FLOAT_RIGHT) || ((count + 1) == btns.size() && float_last_right));

					int len = btn->GetPreferredWidth();

					if(!align_right)
					{
						btn->SetFrameGeometry(left, GetHeight() - rowHeight, len, buttonHeight);

						left += (len + 5);
					}

					count++;
				}

				if(!ignore_right_align)
				{
					count = btns.size();
					int right = GetWidth();

					for(auto &btn : btns | std::views::reverse)
					{
						count--;

						bool align_right = (actions[count].flags & ACTIONF_FLOAT_RIGHT) || ((count + 1) == btns.size() && float_last_right);

						int len = btn->GetPreferredWidth();

						if(align_right)
						{
							right -= (len + 5);

							btn->SetFrameGeometry(right, GetHeight() - rowHeight, len, buttonHeight);
						}
					}
				}
			}
		}

		Show();
		ActivateWindow();
		SetModalCapture(true);
	}
private:
	static std::unique_ptr<PopupBase> currentPopup;

	friend class LauncherWindow;
};

class BasicPopup : public ChoicePopup<BasicPopup>
{
public:
	using ChoicePopup::ChoicePopup;
};

template<class T = BasicPopup>
T& OpenPopup(Widget * parent, const std::string &title, const std::vector<std::string> &text = {}, const PopupBase::ActionListType &actions = {}, double windowWidth = 500.0, int flags = 0)
{
	if(currentPopup)
	{
		currentPopup->Close();
	}

	currentPopup = std::unique_ptr<PopupBase>(new T(parent, title, text, actions, windowWidth, flags));

	return *static_cast<T*>(currentPopup.get());
}

template<typename T>
class ProgressPopup : public ChoicePopup<T>
{
private:
	Widget * updateBar;
	double updateBarX;
	double updateBarY;
	double updateBarWidth;
	double updateBarHeight;
protected:

	double updateBarPercentage;

	template<class>
	friend void OpenPopup(Widget *, const std::string &, const std::vector<std::string> &, const PopupBase::ActionListType &, double, int);

	int GetExtraHeight()
	{
		return 30;
	}

	void AddExtraElements(int top)
	{
		double updateBarBackgroundHeight = 30;
		double updateBarBackgroundWidth = Widget::GetWidth() - 10;

		updateBarX = 6;
		updateBarY = top + 1;
		updateBarWidth = updateBarBackgroundWidth - 2;
		updateBarHeight = updateBarBackgroundHeight - 2;
		updateBarPercentage = 0.75;

		//border
		Widget * updateBarBackground = new Widget(this);

		updateBarBackground->SetStyleColor("background-color", Theme::getMain(0.9f));
		updateBarBackground->SetFrameGeometry(5, top, updateBarBackgroundWidth, updateBarBackgroundHeight);

		PopupBase::cleanup.push_back(std::unique_ptr<Widget>{updateBarBackground});

		//background
		updateBarBackground = new Widget(this);

		updateBarBackground->SetStyleColor("background-color", Colorf(0.0f, 0.3f, 0.5f, 1.0f));
		updateBarBackground->SetFrameGeometry(updateBarX, updateBarY, updateBarWidth, updateBarHeight);

		PopupBase::cleanup.push_back(std::unique_ptr<Widget>{updateBarBackground});

		//bar
		updateBar = new Widget(this);

		updateBar->SetStyleColor("background-color", Colorf(0.2f, 0.5f, 0.75f, 1.0f));
		RefreshBar();

		PopupBase::cleanup.push_back(std::unique_ptr<Widget>{updateBar});

		top += 30;
	}

	void RefreshBar()
	{
		updateBar->SetFrameGeometry(updateBarX, updateBarY, updateBarWidth * updateBarPercentage, updateBarHeight);
	}
public:

	friend class ChoicePopup<T>;
	using ChoicePopup<T>::ChoicePopup;
};

constexpr int bar_height = 30;
constexpr int close_margin = 6;
constexpr int arrow_margin = 0;

void LauncherWindow::OnWindowClose()
{ // don't close launcher window if popup is being shown
	if(!currentPopup) Close();
}

UpdateButtonBar::UpdateButtonBar(LauncherWindow *parent, SettingsPage* settings) : Widget(parent)
{
	SetStyleColor("bg-default-color", Colorf(0.0f, 0.3f, 0.5f, 1.0f));
	SetStyleColor("bg-highlight-color", Colorf(0.2f, 0.5f, 0.75f, 1.0f));
	SetStyleColor("bg-press-color", Colorf(0.0f, 0.2f, 0.333f, 1.0f));
	SetStyleColor("close-highlight-color", Colorf(0.9f, 0.3f, 0.2f, 1.0f));
	SetStyleColor("close-press-color", Colorf(0.6f, 0.15f, 0.1f, 1.0f));

	SetStyleColor("background-color", GetStyleColor("bg-default-color"));
	SetStyleColor("color", Colorf(1.0f, 1.0f, 1.0f, 1.0f));
	arrow = Image::LoadResource("ui/arrow.png");
	close = Image::LoadResource("ui/close.png");
	_settings = settings;
}

FString UpdateButtonBar::UpdateToString()
{
	FString str = "";

	VersionInfo update = currentUpdate->version;

	switch(CURRENT_UPDATE_CHANNEL)
	{
	case UpdateChannel::STABLE:
		str.Format("%u.%u.%u", update.major, update.minor, update.revision);
		break;
	case UpdateChannel::PREVIEW:
		str.Format("%u.%u.%u-pre-%u", update.major, update.minor, update.revision, update.distance);
		break;
	case UpdateChannel::TESTING:
		str.Format("%u.%u.%u-pre-%u (%s)", update.major, update.minor, update.revision, update.distance, GStrings.GetString("TXT_EXPERIMENTAL"));
		break;
	case UpdateChannel::RELEASE_CANDIDATE:
		if(update.distance != 0 && update.distance != RC_REVISION_NOTRC)
		{
			str.Format("%u.%u.%u-rc%u", update.major, update.minor, update.revision, update.distance);
		}
		else
		{
			str.Format("%u.%u.%u", update.major, update.minor, update.revision);
		}
		break;
	}
	return str;
}

void UpdateButtonBar::UpdateLanguage()
{
	if(currentUpdate.has_value())
	{
		text = FStringf("%s: %s", GStrings.GetString("UPDATER_UPDATE_AVAILABLE"), UpdateToString());
	}
	else
	{
		text = "";
	}
}

void UpdateButtonBar::UpdateSettingsPage()
{
	if (_settings != nullptr)
		_settings->UpdateUpdaterValues(updater_auto_updates, updater_check_updates, updater_update_interval);
}

void UpdateButtonBar::OnPaint(Canvas* canvas)
{
	canvas->fillRect(Rect(0, 0, bar_height, bar_height), close_pressed ? GetStyleColor("close-press-color") : close_highlighted ? GetStyleColor("close-highlight-color") : GetStyleColor("bg-default-color"));

	Rect box = canvas->measureText(text.GetChars());
	canvas->drawText(Point((GetWidth() - bar_height - box.width) * 0.5, (bar_height * 0.5) + (box.height * 0.35)), GetStyleColor("color"), text.GetChars());

	canvas->drawImage(close, Rect(close_margin, close_margin, bar_height - (close_margin * 2), bar_height - (close_margin * 2)));
	canvas->drawImage(arrow, Rect(GetWidth() - (bar_height - arrow_margin), arrow_margin, bar_height - (arrow_margin * 2), bar_height - (arrow_margin * 2)));
}

void UpdateButtonBar::OnMouseMove(const Point& pos)
{
	if(pressed || close_pressed) return;

	if(pos.x > bar_height)
	{
		SetStyleColor("background-color", GetStyleColor("bg-highlight-color"));
		close_highlighted = false;
	}
	else
	{
		SetStyleColor("background-color", GetStyleColor("bg-default-color"));
		close_highlighted = true;
	}

	Update();
}

void UpdateButtonBar::OnMouseLeave()
{
	if(!pressed)
	{
		SetStyleColor("background-color", GetStyleColor("bg-default-color"));
	}

	close_highlighted = false;

	Update();
}

bool UpdateButtonBar::OnMouseDown(const Point& pos, InputKey key)
{
	if(key != InputKey::LeftMouse || pressed || close_pressed) return false;

	SetPointerCapture();

	if(pos.x > bar_height)
	{
		SetStyleColor("background-color", GetStyleColor("bg-press-color"));
		pressed = true;
	}
	else
	{
		close_pressed = true;
	}

	Update();

	return false;
}

void UpdateButtonBar::OpenDismissUpdateMenu()
{
	PopupBase::ActionListType actions = {
		{"UPDATER_UPDATE_SKIP", [=, this](auto &self){
			updater_skipped_update = FString(currentUpdate->version);
			M_SaveDefaults(NULL); // save settings
			Hide();
			self.Close();
		}},
	};

	actions.push_back({"TXT_BACK", [=, this](auto &self){
		self.Close();
	}});

	OpenPopup(this, "UPDATER_DISMISS_UPDATE", {}, actions, 550.0, 0);
}

void UpdateButtonBar::OpenFailedUpdateMenu(const std::string &err, bool checker)
{
	PopupBase::ActionListType actions = {
		{"TXT_DISMISS", [=, this](auto &self){
			Hide();
			self.Close();
		}},
		{"UPDATER_DISABLE", [=, this](auto &self){
			updater_check_updates = false;
			M_SaveDefaults(NULL); // save settings
			Hide();
			self.Close();
		}}
	};

	std::string title = checker
		? "UPDATER_CHECK_FAILED"
		: "UPDATER_UPDATE_FAILED";

	OpenPopup(
		this,
		title,
		SplitNewLines(GStrings.GetString(title) + ("\n" + err)),
		actions,
		550.0,
		POPUPF_DISALLOW_CLOSE
	);
}

void UpdateButtonBar::OpenUpdateMenu(bool isAutoUpdate)
{
	PopupBase::ActionListType actions = {
		{"PICKER_SHOWNOTES", [this, isAutoUpdate](auto &self){
			OpenPopup(this, "PICKER_TAB_RELEASE", this->currentUpdate->release_notes,
			{
				{"TXT_BACK", [this, isAutoUpdate](auto &self){
					OpenUpdateMenu(isAutoUpdate);
				}}
			});
		}},
		{"TXT_UPDATE", [this](auto &currentPopup){
			StartUpdate();
		}}
	};

	if(isAutoUpdate)
	{
		actions.push_back({"TXT_SKIP", [this](auto &self){
			updater_skipped_update = FString(currentUpdate->version);
			M_SaveDefaults(NULL); // save settings
			Hide();
			self.Close();
		}, ACTIONF_FLOAT_RIGHT});

		actions.push_back({"TXT_DISMISS", [this](auto &self){
			UpdateLanguage();
			Show();
			self.Close();
		}});
	}

	if(currentUpdate->cached)
	{
		bool ok = false;
		currentUpdate = GetUpdateInfo(ok); // we only have the cached update number right now, grab full update info
		if(!ok || !currentUpdate.has_value()) return;
		updater_cached_update = FString(currentUpdate->version);
		updater_last_update_check = std::to_string(getCurrentDate()).c_str();
		M_SaveDefaults(NULL); // save settings
		UpdateLanguage();
	}

	std::vector<std::string> updateInfo;

	updateInfo.push_back((GAMENAME + (" " + UpdateToString())).GetChars());

	OpenPopup(this, isAutoUpdate ? "UPDATER_UPDATE_AVAILABLE" : "TXT_UPDATE", updateInfo, actions, 500.0/*, isAutoUpdate ? POPUPF_DISALLOW_CLOSE : 0*/);
}

bool UpdateButtonBar::OnMouseUp(const Point& pos, InputKey key)
{
	if(key != InputKey::LeftMouse) return false;

	ReleasePointerCapture();

	if(pos.y > 0 && pos.y < bar_height && pos.x < GetWidth())
	{
		if(pos.x > bar_height)
		{
			if(pressed)
			{
				OpenUpdateMenu(false);
			}

			SetStyleColor("background-color", GetStyleColor("bg-highlight-color"));
		}
		else if(pos.x > 0 && pos.x < bar_height)
		{
			if(close_pressed)
			{
				OpenPopup(this, "UPDATER_DISMISS_UPDATE", {},
				{
					{"TXT_DISMISS", [this](auto &self){
						this->Hide();
						self.Close();
					}},
					{"UPDATER_UPDATE_SKIP", [this](auto &self){
						updater_skipped_update = FString(currentUpdate->version);
						M_SaveDefaults(NULL); // save settings
						this->Hide();
						self.Close();
					}},
					{"UPDATER_DISABLE", [this](auto &self){
						updater_check_updates = false;
						M_SaveDefaults(NULL); // save settings
						this->Hide();
						self.Close();
					}}
				});
			}

			SetStyleColor("background-color", GetStyleColor("bg-default-color"));
		}
	}
	else
	{
		SetStyleColor("background-color", GetStyleColor("bg-default-color"));
	}

	pressed = false;
	close_pressed = false;

	Update();

	return false;
}

double UpdateButtonBar::GetPreferredHeight()
{
	return bar_height;
}

void UpdateButtonBar::OnUpdateButtonClicked()
{
	OpenUpdateMenu(false);
}

LauncherWindow* UpdateButtonBar::GetLauncher() const
{
	return static_cast<LauncherWindow*>(Parent());
}

void UpdateButtonBar::OpenUpdateInitChoice()
{

	OpenPopup(this, "UPDATER_TITLE", SplitNewLines(GStrings.GetString("UPDATER_ASK_AUTO")),
	{
		{
			"TXT_YES", [this](auto &self)
			{
				updater_auto_updates = false;
				OpenUpdateIntervalChoice();
			},
		},{
			"UPDATER_YES_PROMPT", [this](auto &self)
			{
				updater_auto_updates = true;
				OpenUpdateIntervalChoice();
			},
			//ACTIONF_FLOAT_RIGHT
		},{
			"TXT_NO", [this](auto &self)
			{
				updater_check_updates = false;
				updater_check_updates_initialized = true;
				M_SaveDefaults(NULL); // save settings
				UpdateSettingsPage();
				self.Close();
			},
			//0, 1
		}
	}, 600.0, POPUPF_DISALLOW_CLOSE|POPUPF_CENTER_BUTTONS);
	//}, 600.0, POPUPF_DISALLOW_CLOSE);
}

void UpdateButtonBar::OpenUpdateIntervalChoice()
{
	OpenPopup(this, "UPDATER_TITLE", SplitNewLines(GStrings.GetString("UPDATER_ASK_INTERVAL")),
	{
		{
			"OPTVAL_DAILY", [this](auto &self)
			{
				updater_check_updates = true;
				updater_update_interval = 1;
				updater_check_updates_initialized = true;
				updater_last_update_check = std::to_string(getCurrentDate()).c_str();
				M_SaveDefaults(NULL); // save settings
				UpdateSettingsPage();
				self.Close();
			}
		},{
			"OPTVAL_WEEKLY", [this](auto &self)
			{
				updater_check_updates = true;
				updater_update_interval = 7;
				updater_check_updates_initialized = true;
				updater_last_update_check = std::to_string(getCurrentDate() - daysToSeconds(5)).c_str(); // first check always in 2 days
				M_SaveDefaults(NULL); // save settings
				UpdateSettingsPage();
				self.Close();
			}
		},{
			"OPTVAL_MONTHLY", [this](auto &self)
			{
				updater_check_updates = true;
				updater_update_interval = 30;
				updater_check_updates_initialized = true;
				updater_last_update_check = std::to_string(getCurrentDate() - daysToSeconds(28)).c_str(); // first check always in 2 days
				M_SaveDefaults(NULL); // save settings
				UpdateSettingsPage();
				self.Close();
			}
		},{
			"TXT_BACK", [this](auto &self)
			{
				updater_auto_updates = false;
				OpenUpdateInitChoice();
			}
		}
	}, 550.0, POPUPF_DISALLOW_CLOSE);
}

bool UpdateButtonBar::curl_initialized = false;
bool UpdateButtonBar::curl_initialized_ok = false;

bool UpdateButtonBar::InitCurl()
{
	if(curl_initialized) return curl_initialized_ok;
	if(!IsNetworkStartedLean()) StartNetworkLean();

	if(!curl_initialized)
	{
		CURLcode ret;

		curl_initialized = true;
		curl_initialized_ok = ((ret = curl_global_init(CURL_GLOBAL_DEFAULT)) == CURLE_OK);

		if(!curl_initialized_ok)
		{
			OpenFailedUpdateMenu("curl_global_init failed: "+std::string(curl_easy_strerror(ret)), true);
		}
	}

	return curl_initialized_ok;
}


static size_t callAcceptData(void *buf, size_t sz, size_t num, void *self);
static size_t callAcceptHeader(void *buf, size_t sz, size_t num, void *self);
static int callUpdateProgress(void * self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

class CurlEasy
{
	friend size_t callAcceptData(void *buf, size_t sz, size_t num, void *self);
	friend size_t callAcceptHeader(void *buf, size_t sz, size_t num, void *self);
	friend int callUpdateProgress(void * self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	CURL * curl;

	std::atomic<bool> cancelled = false;

	virtual void UpdateProgress(curl_off_t dltotal, curl_off_t dlnow) {}; //, curl_off_t ultotal, curl_off_t ulnow) {};
	virtual void AcceptHeader(TArrayView<std::byte> data) {};
	virtual void AcceptData(TArrayView<std::byte> data) = 0;
	virtual void OnError(const char * err) {};

protected:

	void CancelDownload()
	{
		cancelled = true;
	}

	CurlEasy(const char * userAgent, bool useProgress, bool readHeader)
	{
		curl = curl_easy_init();

		if(!curl) I_Error("curl_easy_init failed");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callAcceptData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

		if(readHeader)
		{
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, callAcceptHeader);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
		}

		if(useProgress)
		{
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, callUpdateProgress);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		}

		curl_easy_setopt(curl,CURLOPT_USERAGENT, userAgent);

		curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA); // use native SSL CA

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // allow redirects

		if(updater_debug_throttle_download)
		{
			curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, 1000000L); // set max speed of ~1mb/s for testing
		}
	}

	bool Perform(const std::string &url, const std::string &acceptEncoding, bool * cancelled = nullptr)
	{
		if(!curl) return false;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptEncoding.c_str());

		if(CURLcode ret = curl_easy_perform(curl); ret != CURLE_OK)
		{
			if(cancelled) (*cancelled) = this->cancelled;
			OnError(curl_easy_strerror(ret));
			return false;
		}
		if(cancelled) (*cancelled) = this->cancelled;
		return true;
	}
public:

	virtual ~CurlEasy()
	{
		Close();
	}

	bool IsOpen()
	{
		return curl != nullptr;
	}

	void Close()
	{
		if(curl)
		{
			curl_easy_cleanup(curl);
			curl = nullptr;
		}
	}
};

static size_t callAcceptData(void *buf, size_t sz, size_t num, void *self)
{
	assert(sz == 1); // according to curl docs, size is always 1, but doesn't hurt validating it with an assert
	((CurlEasy*)self)->AcceptData({(std::byte*)buf, (unsigned)num});
	return num;
}

static size_t callAcceptHeader(void *buf, size_t sz, size_t num, void *self)
{
	assert(sz == 1); // according to curl docs, size is always 1, but doesn't hurt validating it with an assert
	((CurlEasy*)self)->AcceptHeader({(std::byte*)buf, (unsigned)num});
	return num;
}

static int callUpdateProgress(void * self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	((CurlEasy*)self)->UpdateProgress(dltotal, dlnow); //, ultotal, ulnow);
	return ((CurlEasy*)self)->cancelled;
}

const char * getMainURL(UpdateChannel channel)
{
	switch(channel)
	{
	case UpdateChannel::STABLE:
		return UPDATER_URL_STABLE;
	case UpdateChannel::PREVIEW:
		return UPDATER_URL_PREVIEW;
	case UpdateChannel::TESTING:
		return UPDATER_URL_TESTING;
	case UpdateChannel::RELEASE_CANDIDATE:
		return UPDATER_URL_ALL;
	default:
		I_Error("Unknown Update Channel");
	}
}

const char * getBackupURL(UpdateChannel channel)
{
	switch(channel)
	{
	case UpdateChannel::STABLE:
		return UPDATER_URL_STABLE_BACKUP;
	case UpdateChannel::PREVIEW:
		return UPDATER_URL_PREVIEW_BACKUP;
	case UpdateChannel::TESTING:
		return UPDATER_URL_TESTING_BACKUP;
	case UpdateChannel::RELEASE_CANDIDATE:
		return UPDATER_URL_ALL_BACKUP;
	default:
		I_Error("Unknown Update Channel");
	}
}

class UpdateChecker : public CurlEasy
{
	std::string buffer;

	virtual void AcceptData(TArrayView<std::byte> data) override
	{
		buffer += std::string((const char *)data.Data(), data.Size());
	}

	const char * firstErr = nullptr;

	virtual void OnError(const char * err)
	{
		if(!firstErr) firstErr = err;
	};
public:
	UpdateChecker() : CurlEasy(GAMENAME " Updater", false, false)
	{}

	std::optional<rapidjson::Document> Perform(UpdateButtonBar * buttonBar, UpdateChannel channel)
	{
		if(!CurlEasy::Perform(getMainURL(channel), "application/vnd.github+json"))
		{
			buffer = "";
			if(!CurlEasy::Perform(getBackupURL(channel), "application/vnd.github+json"))
			{
				buttonBar->OpenFailedUpdateMenu(firstErr, true);
				return std::nullopt;
			}
		}
		Close();

		rapidjson::Document doc;
		rapidjson::ParseResult ok = doc.Parse(buffer.c_str(), buffer.length());

		if(!ok)
		{
			buttonBar->OpenFailedUpdateMenu(GStrings.GetString("UPDATER_INVALID_JSON"), true);
			return std::nullopt;
		}

		return doc;
	}
};

class JsonDownloader : public CurlEasy
{
	std::string buffer;

	virtual void AcceptData(TArrayView<std::byte> data) override
	{
		buffer += std::string((const char *)data.Data(), data.Size());
	}

	const char * firstErr = nullptr;

	virtual void OnError(const char * err) override
	{
		if(!firstErr) firstErr = err;
	};
public:
	JsonDownloader() : CurlEasy(GAMENAME " Updater", false, false)
	{}

	std::optional<rapidjson::Document> Perform(UpdateButtonBar * buttonBar, const std::string &url)
	{
		if(!CurlEasy::Perform(url, "application/json"))
		{
			buttonBar->OpenFailedUpdateMenu(firstErr, true);
			return std::nullopt;
		}
		Close();

		rapidjson::Document doc;
		rapidjson::ParseResult ok = doc.Parse(buffer.c_str(), buffer.length());

		if(!ok)
		{
			buttonBar->OpenFailedUpdateMenu(GStrings.GetString("UPDATER_INVALID_JSON"), true);
			return std::nullopt;
		}

		return doc;
	}
};

void CloseWidgetResources();
void M_SaveDefaultsFinal();

class ProgressDownloader : public CurlEasy, public ProgressPopup<ProgressDownloader>
{
	std::vector<std::byte> buffer;

	static inline const std::string names[4] = {
		"B",  // 0
		"KB", // 1
		"MB", // 2
		"GB", // 3
	};

	std::string shortenByteSize(uint64_t size)
	{
		constexpr int maxStep = 3;

		int steps = 0;
		double remainder = 0;

		while(size > 1024 && steps < maxStep)
		{
			remainder = (double)(size % 1024);
			size /= 1024;
			steps++;
		}

		return std::format("{:.3} {}", size + (remainder/1024), names[steps]);
	}
protected:

	virtual void AcceptData(TArrayView<std::byte> data) override
	{
		size_t oldsize = buffer.size();
		buffer.resize(oldsize + data.Size());
		memcpy(buffer.data() + oldsize, data.Data(), data.Size());
	}


	std::mutex progress_lock;
	uint64_t current_download = 0;
	uint64_t total_download = 0;
	bool close_requested = false;
	std::mutex finished_lock;
	bool finished = false;
	bool success = false;

	virtual void UpdateProgress(curl_off_t dltotal, curl_off_t dlnow)
	{
		progress_lock.lock();
		current_download = dlnow;
		total_download = dltotal;
		progress_lock.unlock();
		ProgressPopup::Update();	//FIXME is this thread-safe on SDL2?
									// (not much of a concern for now, since on windows it just
									// calls InvalidateRect, which _is_ thread-safe, but will need
									// looking into once the updater is made cross-platform)

		//TODO calculate average/current download speeds
	};

	virtual void OnPaint(Canvas* canvas) override
	{
		progress_lock.lock();
		updateBarPercentage = std::min(1.0, std::max(0.0, ((double)current_download) / ((double)total_download)));
		text[0]->SetText( std::format("{}, {} / {}", GStrings.GetString("TXT_DOWNLOADING"), shortenByteSize(current_download), shortenByteSize(total_download)));
		progress_lock.unlock();
		ProgressPopup::RefreshBar();
	}

	const char * err = nullptr;

	virtual void OnError(const char * err) override
	{
		this->err = err;
	};

public:
	ProgressDownloader(Widget * parent, const std::string &title, const std::vector<std::string> &text, const PopupBase::ActionListType &actions, double _windowWidth, int flags)
		:	CurlEasy(GAMENAME " Updater", true, false),
		ProgressPopup(parent, title, ConcatText({GStrings.GetString("UPDATER_STARTING")}, text), actions, _windowWidth, flags)
	{}

	static void DownloaderThread(ProgressDownloader * self, const std::string &url)
	{
		if(!self->CurlEasy::Perform(url, "application/octet-stream", &self->close_requested))
		{
			self->finished_lock.lock();
			self->finished = true;
			self->success = false;
			self->finished_lock.unlock();
		}
		else
		{
			self->finished_lock.lock();
			self->finished = true;
			self->success = true;
			self->finished_lock.unlock();
		}

		self->CurlEasy::Close();

		if(!self->close_requested) self->NotifyWindow();
	}

	std::unique_ptr<std::thread> downloader_thread;

	UpdateButtonBar * buttonBar;

	void Perform(UpdateButtonBar * buttonBar, const std::string &url)
	{
		this->buttonBar = buttonBar;
		downloader_thread = std::make_unique<std::thread>(DownloaderThread, this, url);
	}

	virtual void OnWindowNotified() override
	{
		if(downloader_thread)
		{
			downloader_thread->join();
			downloader_thread = nullptr;
		}

		if(finished && !success)
		{
			buttonBar->OpenFailedUpdateMenu(err, false);
		}
		else
		{
			std::unique_ptr<FResourceFile> zip (FResourceFile::OpenResourceFileMemory(buttonBar->GetDownloadURL().c_str(), (void*)buffer.data(), buffer.size(), true));

			if(!zip)
			{
				OpenPopup(buttonBar, "UPDATER_ZIP_FAIL", {GStrings.GetString("UPDATER_CANCELLED")},
				{
					{"TXT_BACK", [](auto &self){
						self.Close();
					}}
				});
			}
			else
			{
				#ifdef _WIN32	// this is technically 'portable' code since it only uses the stdlib, but what it does only makes sense on windows,
								// hence the ifdef - linux would need to either use the appimage updater library that i forgot the name of,
								// or just nothing, as it would otherwise be handled by an external package manager (ex. apt/pacman/flatpak/etc)
								// and on macOS i genuinely have no idea what would be needed for an auto-updater

				std::string progdir(::progdir.GetChars());

				try
				{
					if(std::filesystem::exists(progdir + "update"))
					{
						if(std::filesystem::is_directory(progdir + "update"))
						{
							std::filesystem::remove_all(progdir + "update");
							std::filesystem::create_directory(progdir + "update");
						}
						else
						{
							std::string dirstr = progdir + "update";
							auto errstr = std::vformat(GStrings.GetString("UPDATER_NOT_A_DIR"), std::make_format_args(dirstr));
							buttonBar->OpenFailedUpdateMenu(errstr, false);
							return;
						}
					}
					else
					{
						std::filesystem::create_directory(progdir + "update");
					}

					if(std::filesystem::exists(progdir + "update_backup"))
					{
						if(std::filesystem::is_directory(progdir + "update_backup"))
						{
							std::filesystem::remove_all(progdir + "update_backup");
							std::filesystem::create_directory(progdir + "update_backup");
						}
						else
						{
							std::string dirstr = progdir + "update_backup";
							auto errstr = std::vformat(GStrings.GetString("UPDATER_NOT_A_DIR"), std::make_format_args(dirstr));
							buttonBar->OpenFailedUpdateMenu(errstr, false);
							return;
						}
					}
					else
					{
						std::filesystem::create_directory(progdir + "update_backup");
					}

				}
				catch(std::exception &e)
				{
					buttonBar->OpenFailedUpdateMenu(e.what(), false);
					return;
				}

				uint32_t n = zip->EntryCountU();

				try
				{
					//extract zip contents into `$PROGDIR/update/`, copy existing files with same name into `$PROGDIR/update_backup/`
					for(uint32_t i = 0; i < n; i++)
					{
						std::string path = zip->getName(i);

						bool isupdaterexe = (path == "updater.exe");

						std::filesystem::path p (path);

						if(p.has_parent_path() && !isupdaterexe)
						{
							std::filesystem::create_directories(progdir + "update/" + p.parent_path().string());
						}

						if(std::filesystem::exists(progdir + path))
						{ // make backup of file, so that a failed update can be reverted
							if(p.has_parent_path())
							{
								std::filesystem::create_directories(progdir + "update_backup/" + p.parent_path().string());
							}
							std::filesystem::copy(progdir + path, progdir + "update_backup/" + path);
						}

						std::string newpath = isupdaterexe ? (progdir + path) : (progdir + "update/" + path); // updater.exe is replaced directly, doesn't go into the update subfolder

						FILE * f = fopen(newpath.c_str(), "wb");
						if(!f)
						{
							std::string err = strerror(errno);
							try
							{
								std::filesystem::remove_all(progdir + "update");
								std::filesystem::remove_all(progdir + "update_backup");
							}
							catch(...) {} // try to remove created files, but if it fails, only show the main error, not the one from the removal

							auto errstr = std::vformat(GStrings.GetString("UPDATER_ZIP_ERROR"), std::make_format_args(err, newpath));
							buttonBar->OpenFailedUpdateMenu(errstr, false);
							return;
						}
						else
						{
							FileSys::FileData data = zip->Read(i);

							if(fwrite(data.data(), 1, data.size(), f) != data.size())
							{
								std::string err = strerror(errno);
								fclose(f);
								try
								{
									std::filesystem::remove_all(progdir + "update");
									std::filesystem::remove_all(progdir + "update_backup");
								}
								catch(...) {} // try to remove created files, but if it fails, only show the main error, not the one from the removal

							auto errstr = std::vformat(GStrings.GetString("UPDATER_ZIP_ERROR"), std::make_format_args(err, newpath));
								buttonBar->OpenFailedUpdateMenu(errstr, false);
								return;
							}
							fclose(f);
						}

					}

				}
				catch(std::exception &e)
				{
					try
					{
						std::filesystem::remove_all(progdir + "update");
						std::filesystem::remove_all(progdir + "update_backup");
					}
					catch(...) {} // try to remove created files, but if it fails, only show the main error, not the one from the removal

					buttonBar->OpenFailedUpdateMenu(e.what(), false);
					return;
				}

				OpenPopup(buttonBar, "TXT_UPDATED", {GStrings.GetString("UPDATER_SUCCESS")},
				{
					{"TXT_CONFIRM", [progdir](auto &self){
						updater_cached_update = "";
						updater_last_update_check = std::to_string(getCurrentDate()).c_str();

						M_SaveDefaultsFinal(); // save settings

						CloseWidgetResources();

						// this code leaks memory but it terminates so it's fiiiiiiiiiiiiine
						int argc;
						LPWSTR * argv = CommandLineToArgvW(GetCommandLineW(), &argc);

						std::string updater_filename(progdir + "updater.exe");

						FString updater_filename_quoted((progdir + "updater.exe").c_str());

						updater_filename_quoted.Substitute("\\", "\\\\");
						updater_filename_quoted.Substitute("\"", "\\\"");
						updater_filename_quoted = "\""+updater_filename_quoted+"\"";
						
						int numchars = MultiByteToWideChar(CP_UTF8, 0, updater_filename.c_str(), updater_filename.length(), NULL, 0);

						WCHAR * updater_filename_w = new WCHAR[numchars + 1];
						MultiByteToWideChar(CP_UTF8, 0, updater_filename.c_str(), updater_filename.length(), updater_filename_w, numchars);
						updater_filename_w[numchars] = 0;

						numchars = MultiByteToWideChar(CP_UTF8, 0, updater_filename_quoted.GetChars(), updater_filename_quoted.Len(), NULL, 0);

						WCHAR * updater_filename_quoted_w = new WCHAR[numchars + 1];
						MultiByteToWideChar(CP_UTF8, 0, updater_filename_quoted.GetChars(), updater_filename_quoted.Len(), updater_filename_quoted_w, numchars);
						updater_filename_quoted_w[numchars] = 0;

						argv[0] = updater_filename_quoted_w;
						_wexecv(updater_filename_w, argv);
					}}
				}, 500.0, POPUPF_DISALLOW_CLOSE);
				#else
					#error "Updater not implemented for non-windows platforms"
				#endif

			}
		}
	}


	virtual void OnClose() override
	{
		if(downloader_thread)
		{
			CancelDownload();
			downloader_thread->join();
			downloader_thread = nullptr;

			OpenPopup(buttonBar, "TXT_CANCELLED", {GStrings.GetString("UPDATER_CANCELLED")},
			{
				{"TXT_BACK", [](auto &self){
					self.Close();
				}}
			});
		}
		else
		{
			ProgressPopup::OnClose();
		}
	}
};

#ifdef _WIN32
#define RELEASE_JSON_PLATFORM_NAME "windows"
#else
#error "Updater not implemented for this platform"
#endif

template<typename T>
std::optional<update_info_t> UpdateButtonBar::ParseRelease(T &&doc, bool &ok, bool &silentfail)
{
	VersionInfo ver;

	std::string downloadName;
	std::string downloadUrl;

	if(!doc.IsObject() || !doc.HasMember("assets") || !doc["assets"].IsArray())
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}

	bool release_json_found = false;
	bool download_link_found = false;
	auto arr = doc["assets"].GetArray();

	rapidjson::Document relinfo;

	for(int i = 0; i < (int)arr.Size(); i++)
	{
		if(!arr[i].HasMember("name") || !arr[i]["name"].IsString())
		{
			ok = false;
			silentfail = false;
			return std::nullopt;
		}
		else if(std::string s = arr[i]["name"].GetString(); s == "_release.json")
		{
			if(!arr[i].HasMember("browser_download_url") || !arr[i]["browser_download_url"].IsString())
			{
				ok = false;
				silentfail = false;
				return std::nullopt;
			}

			auto release_info = (JsonDownloader {}).Perform(this, arr[i]["browser_download_url"].GetString());

			if(!release_info.has_value())
			{
				ok = false;
				silentfail = true;
				return std::nullopt;
			}

			relinfo = std::move(*release_info);

			release_json_found = true;
			break;
		}
	}

	if(!release_json_found)
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}
	else if(!relinfo.HasMember("commit") || !relinfo["commit"].IsObject() || !relinfo["commit"].HasMember("parent") || !relinfo["commit"]["parent"].IsString())
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}

	ver = VersionInfo(relinfo["commit"]["parent"].GetString());

	if constexpr(CURRENT_UPDATE_CHANNEL != UpdateChannel::STABLE)
	{
		if(!relinfo["commit"].HasMember("distance") || !relinfo["commit"]["distance"].IsString())
		{
			ok = false;
			silentfail = false;
			return std::nullopt;
		}

		ver.distance = atoi(relinfo["commit"]["distance"].GetString());
	}
	else
	{
		ver.distance = 0;
	}

	if(!relinfo.HasMember("platforms") || !relinfo["platforms"].IsObject() || !relinfo["platforms"].HasMember(RELEASE_JSON_PLATFORM_NAME) || !relinfo["platforms"][RELEASE_JSON_PLATFORM_NAME].IsString())
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}

	downloadName = relinfo["platforms"][RELEASE_JSON_PLATFORM_NAME].GetString();

	for(int i = 0; i < (int)arr.Size(); i++)
	{
		if(!arr[i].HasMember("name") || !arr[i]["name"].IsString())
		{
			ok = false;
			silentfail = false;
			return std::nullopt;
		}
		else if(std::string s = arr[i]["name"].GetString(); s == downloadName)
		{
			if(!arr[i].HasMember("browser_download_url") || !arr[i]["browser_download_url"].IsString())
			{
				ok = false;
				silentfail = false;
				return std::nullopt;
			}

			downloadUrl = arr[i]["browser_download_url"].GetString();
			download_link_found = true;
			break;
		}
	}

	if(!download_link_found)
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}

	if(!doc.HasMember("body") || !doc["body"].IsString())
	{
		ok = false;
		silentfail = false;
		return std::nullopt;
	}

	ok = true;
	return update_info_t{ver, false, SplitNewLines(doc["body"].GetString(), doc["body"].GetStringLength()), downloadUrl};
}

std::optional<update_info_t> UpdateButtonBar::GetUpdateInfo(bool &ok)
{
	if(InitCurl())
	{
		auto doc = (UpdateChecker {}).Perform(this, CURRENT_UPDATE_CHANNEL);

		if(doc.has_value())
		{
			bool silentfail = false;

			if (CURRENT_UPDATE_CHANNEL == UpdateChannel::RELEASE_CANDIDATE)
			{
				if(!doc->IsArray())
				{
					silentfail = false;
				}
				else
				{
					std::vector<std::optional<update_info_t>> updates;
					auto arr = doc->GetArray();
					bool anyok = false;
					for(int i = 0; i < arr.Size(); i++)
					{
						if(!arr[i].IsObject() || !arr[i].HasMember("tag_name") || !arr[i]["tag_name"].IsString() || arr[i]["tag_name"].GetString()[0] < '0' || arr[i]["tag_name"].GetString()[0] > '9')
						{
							continue;
						}
						bool ok2 = true, silentfail2 = true;

						std::optional<update_info_t> out = ParseRelease(arr[i], ok2, silentfail2);

						if(ok2 && out.has_value() && out->version < GetCurrentVersionForUpdate(CURRENT_UPDATE_CHANNEL))
						{
							break;
						}

						if(ok2 && out.has_value())
						{
							anyok = true;
							updates.push_back(out);
						}
					}

					if(!anyok)
					{
						OpenFailedUpdateMenu("Failed to find any valid updates", true);
						ok = false;
						return std::nullopt;
					}

					return updates[0]; // TODO fully check updates, but otherwise they _should_ be in order so returning the newest one is fine since preview/experimental are excluded by the tag name check
				}
			}
			else
			{
				silentfail = false;
				std::optional<update_info_t> out = ParseRelease(*doc, ok, silentfail);

				if(ok)
				{
					return out;
				}
			}

			if(!silentfail)
			{
				OpenFailedUpdateMenu("Invalid Update JSON", true);
			}
		}
	}

	ok = false;
	return std::nullopt;
}

bool isVersionInvalid(VersionInfo ver)
{
	return ver.major == USHRT_MAX || ver.minor == USHRT_MAX || ver.revision == USHRT_MAX || ver == GetCurrentVersion();
}

void UpdateButtonBar::StartUpdate()
{
	OpenPopup<ProgressDownloader>(this, "UPDATER_UPDATING").Perform(this, GetDownloadURL());
}

void UpdateButtonBar::CheckForUpdate(bool force)
{
	Hide();

	if(!updater_check_updates_initialized)
	{
		OpenUpdateInitChoice();
	}
	else if(updater_check_updates || force)
	{
		bool new_update = true;
		if(updater_cached_update->Length() > 0)
		{
			VersionInfo cachedVer(updater_cached_update);

			if(isVersionInvalid(cachedVer))
			{
				updater_cached_update = "";
				M_SaveDefaults(NULL); // save settings
			}
			else
			{
				new_update = false;
				currentUpdate = update_info_t{cachedVer, true, {}, ""};
			}
		}

		VersionInfo skippedVer;

		if(updater_skipped_update->Length() > 0)
		{
			VersionInfo skippedVerTmp = VersionInfo((const char *)updater_skipped_update);
			if(isVersionInvalid(skippedVerTmp))
			{
				updater_skipped_update = "";
				M_SaveDefaults(NULL); // save settings
			}
			else
			{
				skippedVer = skippedVerTmp;
			}
		}

		uint64_t curTime = getCurrentDate();
		uint64_t nextCheckTime = parseDate((FString)updater_last_update_check) + daysToSeconds(updater_update_interval);

		if(curTime >= nextCheckTime || currentUpdate.has_value() || force)
		{
			if(!currentUpdate.has_value() || curTime >= nextCheckTime || force) // invalidate cache if check time is due
			{
				bool ok;
				bool was_cached = currentUpdate.has_value() && currentUpdate->cached;

				VersionInfo cachedVer;

				if(was_cached)
				{
					cachedVer = currentUpdate->version;
				}

				currentUpdate = GetUpdateInfo(ok);

				if(!ok) return;

				new_update = !was_cached || (currentUpdate->version != cachedVer);

				updater_last_update_check = std::to_string(curTime).c_str();
				if(currentUpdate.has_value())
				{
					updater_cached_update = FString(currentUpdate->version);
				}
				else
				{
					updater_cached_update = "";
				}
				M_SaveDefaults(NULL); // save settings
			}

			if(currentUpdate.has_value())
			{
				bool should_update;
				if(updater_debug_always_update)
				{
					should_update = true;
				}
				else if constexpr(CURRENT_UPDATE_CHANNEL == UpdateChannel::TESTING)
				{
					should_update = (currentUpdate->version != GetCurrentVersionForUpdate(CURRENT_UPDATE_CHANNEL));
				}
				else
				{
					should_update = (currentUpdate->version > GetCurrentVersionForUpdate(CURRENT_UPDATE_CHANNEL));
				}

				if(should_update && (skippedVer != currentUpdate->version))
				{
					if((updater_auto_updates && new_update) || force)
					{
						OpenUpdateMenu(true);
					}
					else
					{
						UpdateLanguage();
						Show();
					}
				}
				else if(force)
				{
					OpenPopup(this, "UPDATER_UP_TO_DATE", {GStrings.GetString("UPDATER_UP_TO_DATE")},
					{
						{"TXT_OK", [](auto &self){
							self.Close();
						}}
					});
				}
			}
			else if(force)
			{
				OpenPopup(this, "UPDATER_UP_TO_DATE", {GStrings.GetString("UPDATER_UP_TO_DATE")},
				{
					{"TXT_OK", [](auto &self){
						self.Close();
					}}
				});
			}
		}
	}
}
