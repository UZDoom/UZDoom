/*
** netstartwindow.cpp
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2024-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "launcher/netstartwindow.h"
#include "gstrings.h"
#include "version.h"

#include "starter.h"

#include <algorithm>

struct PlayerData
{
	int         clientID;
	std::string flags;
	std::string name;
	std::string status;
};

// Internal State
std::vector<PlayerData> Players;
std::vector<int>        KickClients;
std::vector<int>        BanClients;

std::string Message           = "";
int         Pos               = 0;
int         MaxPos            = 1;
int         SelectedClientIdx = -1; // For ImGui selection

bool Host        = false;
bool ShouldStart = false;
bool ExitReason  = false;
bool Done        = false;

std::exception_ptr CallbackException = nullptr;

long FindItemByClient(int client)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		if (Players[i].clientID == client)
			return (long)i;
	}
	return -1;
}

std::string GetStatusString(int status)
{
	if (status == 1)
		return GStrings.GetString("NETMENU_STATUS_CONN");
	if (status == 2)
		return GStrings.GetString("NETMENU_STATUS_WAIT");
	if (status == 3)
		return GStrings.GetString("NETMENU_STATUS_READY");
	return "";
}

void NetStartWindow::NetInit(const char *message, bool host)
{
	Host = host;
	Players.clear();
	KickClients.clear();
	BanClients.clear();
	Pos               = 0;
	MaxPos            = 1;
	SelectedClientIdx = -1;
	ShouldStart       = false;
	ExitReason        = false;
	Done              = false;
	CallbackException = nullptr;

	if (message)
		Message = message;
	else
		Message = GStrings.GetString("NETMENU_WAIT");
}

void NetStartWindow::NetMessage(const char *message)
{
	if (message)
		Message = message;
}

void NetStartWindow::NetConnect(int client, const char *name, unsigned flags, int status)
{
	std::string flagStr = "";
	if (flags & 1)
		flagStr += "*";
	if (flags & 2)
		flagStr += "H";

	long index = FindItemByClient(client);
	if (index == -1)
	{
		Players.push_back({client, flagStr, name, GetStatusString(status)});
	}
	else
	{
		Players[index].flags  = flagStr;
		Players[index].name   = name;
		Players[index].status = GetStatusString(status);
	}
}

void NetStartWindow::NetUpdate(int client, int status)
{
	long index = FindItemByClient(client);
	if (index != -1)
	{
		Players[index].status = GetStatusString(status);
	}
}

void NetStartWindow::NetDisconnect(int client)
{
	auto it =
		std::remove_if(Players.begin(), Players.end(), [client](const PlayerData &p) { return p.clientID == client; });
	Players.erase(it, Players.end());

	// Deselect if removed
	if (SelectedClientIdx >= (int)Players.size() ||
	    (SelectedClientIdx >= 0 && Players[SelectedClientIdx].clientID == client))
	{
		SelectedClientIdx = -1;
	}
}

void NetStartWindow::NetProgress(int cur, int limit)
{
	MaxPos = limit;
	Pos    = cur;

	// Ensure list has enough free slots if players haven't connected yet
	for (int i = (int)Players.size(); i < limit; ++i)
	{
		Players.push_back({i, "", "", ""});
	}
}

void NetStartWindow::NetDone()
{
	Done = true;
}

void NetStartWindow::NetClose()
{
	ExitReason = false;
	Done       = true;
}

bool NetStartWindow::ShouldStartNet()
{
	return ShouldStart;
}

int NetStartWindow::GetNetKickClient()
{
	if (KickClients.empty())
		return -1;
	int next = KickClients.back();
	KickClients.pop_back();
	return next;
}

int NetStartWindow::GetNetBanClient()
{
	if (BanClients.empty())
		return -1;
	int next = BanClients.back();
	BanClients.pop_back();
	return next;
}

bool NetStartWindow::NetLoop(bool (*timer_callback)(void *), void *userdata)
{
	Starter::ImGuiContextState context = Starter::SetupContext(GAMENAME, 600, 400, SDL_INIT_VIDEO | SDL_INIT_TIMER);
	if (!context.window)
		return false;

	ImGuiIO &io        = ImGui::GetIO();
	Uint32   last_tick = SDL_GetTicks();

	Starter::RunImGuiLoop(context, [&](bool &done_loop) {
		// If another thread or callback marked Done, immediately exit loop
		if (Done)
		{
			done_loop = true;
			return;
		}

		// 10ms Timer Callback Logic
		Uint32 current_tick = SDL_GetTicks();
		if (current_tick - last_tick >= 10)
		{
			if (timer_callback)
			{
				bool result = false;
				try
				{
					result = timer_callback(userdata);
				}
				catch (...)
				{
					CallbackException = std::current_exception();
					Done              = true;
				}

				if (result)
				{
					ExitReason = true;
					Done       = true;
				}
			}
			last_tick = current_tick;
		}

		// Check again just in case the timer callback set Done to true
		if (Done)
		{
			done_loop = true;
			return;
		}

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);

		// Draw the Main Window
		ImGui::Begin("NetMenu", nullptr,
		             ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		// Status & Progress
		ImGui::SetCursorPosY(15.0f);
		float windowWidth = ImGui::GetWindowSize().x;

		float textWidth = ImGui::CalcTextSize(Message.c_str()).x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text("%s", Message.c_str());

		std::string countStr   = std::to_string(Pos) + "/" + std::to_string(MaxPos);
		float       countWidth = ImGui::CalcTextSize(countStr.c_str()).x;
		ImGui::SetCursorPosX((windowWidth - countWidth) * 0.5f);
		ImGui::Text("%s", countStr.c_str());

		ImGui::Spacing();
		ImGui::Spacing();

		// Player List
		float reserveHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
		if (ImGui::BeginTable("PlayerList", 4,
		                      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
		                      ImVec2(0, -reserveHeight)))
		{
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40.0f);
			ImGui::TableSetupColumn(GStrings.GetString("NETMENU_LIST_INFO"), ImGuiTableColumnFlags_WidthFixed, 55.0f);
			ImGui::TableSetupColumn(GStrings.GetString("NETMENU_LIST_PLAYER"), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(GStrings.GetString("NETMENU_LIST_STATUS"), ImGuiTableColumnFlags_WidthFixed,
			                        160.0f);
			ImGui::TableHeadersRow();

			for (int i = 0; i < (int)Players.size(); ++i)
			{
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				std::string label       = std::to_string(Players[i].clientID) + "##" + std::to_string(i);
				bool        is_selected = (SelectedClientIdx == i);

				if (ImGui::Selectable(label.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns))
				{
					SelectedClientIdx = i;
				}

				ImGui::TableNextColumn();
				ImGui::Text("%s", Players[i].flags.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%s", Players[i].name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%s", Players[i].status.c_str());
			}
			ImGui::EndTable();
		}

		// Buttons
		ImGui::Separator();
		ImGui::Spacing();

		if (Host)
		{
			float btnWidth =
				(windowWidth - ImGui::GetStyle().WindowPadding.x * 2.0f - ImGui::GetStyle().ItemSpacing.x * 3.0f) /
				4.0f;

			if (ImGui::Button(GStrings.GetString("NETMENU_BTN_START"), ImVec2(btnWidth, 0)))
				ShouldStart = true;

			ImGui::SameLine();

			ImGui::BeginDisabled(SelectedClientIdx == -1);
			if (ImGui::Button(GStrings.GetString("NETMENU_BTN_KICK"), ImVec2(btnWidth, 0)))
			{
				int clientID = Players[SelectedClientIdx].clientID;
				if (std::find(KickClients.begin(), KickClients.end(), clientID) == KickClients.end())
					KickClients.push_back(clientID);
			}
			ImGui::SameLine();
			if (ImGui::Button(GStrings.GetString("NETMENU_BTN_BAN"), ImVec2(btnWidth, 0)))
			{
				int clientID = Players[SelectedClientIdx].clientID;
				if (std::find(BanClients.begin(), BanClients.end(), clientID) == BanClients.end())
					BanClients.push_back(clientID);
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button(GStrings.GetString("NETMENU_BTN_ABORT"), ImVec2(btnWidth, 0)))
			{
				ExitReason = false;
				Done       = true;
				done_loop  = true;
			}
		}
		else
		{
			float btnWidth = 100.0f;
			ImGui::SetCursorPosX((windowWidth - btnWidth) * 0.5f);
			if (ImGui::Button(GStrings.GetString("NETMENU_BTN_ABORT"), ImVec2(btnWidth, 0)))
			{
				ExitReason = false;
				Done       = true;
				done_loop  = true;
			}
		}

		ImGui::End();
	});

	Starter::TeardownContext(context);

	if (CallbackException)
		std::rethrow_exception(CallbackException);

	return ExitReason;
}
