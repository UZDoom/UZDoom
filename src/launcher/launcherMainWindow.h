/*
** launcherMainWindow.h
**
** Header for launcherMainWindow.cpp
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "imgui.h"
#include "loader.h"
#include "profile.h"
#include "profileSettings.h"
#include <atomic>
#include <string>
#include <vector>

#include <cstdint>


// define the theme class to store the style for the launcher
class LauncherTheme
{
  public:
	enum class BaseTheme
	{
		Custom,
		ImGuiLight,
		ImGuiDark
	};

	BaseTheme type = BaseTheme::ImGuiDark;

	// Custom theme colors
	ImVec4 bgColor;
	ImVec4 textColor;
	ImVec4 inputsColor;
	ImVec4 interactColor;
	ImVec4 hoverColor;
	ImVec4 clickColor;
	ImVec4 borderColor;

	// Default constructor
	LauncherTheme() = default;

	// Construct standard themes
	explicit LauncherTheme(BaseTheme baseType) : type(baseType)
	{
	}

	// Construct custom themes
	LauncherTheme(uint32_t bg, uint32_t text, uint32_t inputs, uint32_t interact, uint32_t hover, uint32_t click,
	              uint32_t border)
		: type(BaseTheme::Custom), bgColor(HexToImVec4(bg)), textColor(HexToImVec4(text)),
		  inputsColor(HexToImVec4(inputs)), interactColor(HexToImVec4(interact)), hoverColor(HexToImVec4(hover)),
		  clickColor(HexToImVec4(click)), borderColor(HexToImVec4(border))
	{
	}

	// Static helper
	static ImVec4 HexToImVec4(uint32_t hex, float alpha = 1.0f)
	{
		float r = ((hex >> 16) & 0xFF) / 255.0f;
		float g = ((hex >> 8) & 0xFF) / 255.0f;
		float b = ((hex >> 0) & 0xFF) / 255.0f;
		return ImVec4(r, g, b, alpha);
	}

	// Applies the theme to the current ImGui context
	void ApplyTheme() const
	{
		ImGuiStyle &style = ImGui::GetStyle();

		// Handle Standard Themes
		if (type == BaseTheme::ImGuiLight)
		{
			ImGui::StyleColorsLight();
			return;
		}

		if (type == BaseTheme::ImGuiDark)
		{
			ImGui::StyleColorsDark();
			return;
		}

		// Setup base dark theme to catch any missing elements safely
		ImGui::StyleColorsDark();
		ImVec4 *colors = style.Colors;

		// Core Backgrounds & Text
		colors[ImGuiCol_WindowBg]       = ImVec4(bgColor.x, bgColor.y, bgColor.z, 0.95f);
		colors[ImGuiCol_ChildBg]        = ImVec4(bgColor.x, bgColor.y, bgColor.z, 0.58f);
		colors[ImGuiCol_PopupBg]        = ImVec4(inputsColor.x, inputsColor.y, inputsColor.z, 0.92f);
		colors[ImGuiCol_Text]           = textColor;
		colors[ImGuiCol_TextDisabled]   = ImVec4(textColor.x, textColor.y, textColor.z, 0.50f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.43f);

		// Borders
		colors[ImGuiCol_Border]       = ImVec4(borderColor.x, borderColor.y, borderColor.z, 0.65f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frame Backgrounds (Inputs, Checkboxes, etc.)
		colors[ImGuiCol_FrameBg]        = inputsColor;
		colors[ImGuiCol_FrameBgHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.78f);
		colors[ImGuiCol_FrameBgActive]  = clickColor;

		// Title Bar & Menus
		colors[ImGuiCol_TitleBg]          = interactColor;
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.75f);
		colors[ImGuiCol_TitleBgActive]    = hoverColor;
		colors[ImGuiCol_MenuBarBg]        = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.47f);

		// Interactive Elements (Buttons)
		colors[ImGuiCol_Button]        = interactColor;
		colors[ImGuiCol_ButtonHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.86f);
		colors[ImGuiCol_ButtonActive]  = clickColor;
		colors[ImGuiCol_CheckMark]     = ImVec4(clickColor.x, clickColor.y, clickColor.z, 0.80f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg]          = bgColor;
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.50f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.78f);
		colors[ImGuiCol_ScrollbarGrabActive]  = clickColor;

		// Sliders & Grips
		colors[ImGuiCol_SliderGrab]        = interactColor;
		colors[ImGuiCol_SliderGrabActive]  = clickColor;
		colors[ImGuiCol_ResizeGrip]        = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.50f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.78f);
		colors[ImGuiCol_ResizeGripActive]  = clickColor;

		// Headers (Selectable rows, Trees)
		colors[ImGuiCol_Header]        = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.76f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.86f);
		colors[ImGuiCol_HeaderActive]  = clickColor;

		// Tables
		colors[ImGuiCol_TableHeaderBg]     = inputsColor;
		colors[ImGuiCol_TableBorderLight]  = borderColor;
		colors[ImGuiCol_TableBorderStrong] = borderColor;

		// Modals
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(bgColor.x, bgColor.y, bgColor.z, 0.73f);

		// Tabs
		colors[ImGuiCol_Tab]                       = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.60f);
		colors[ImGuiCol_TabHovered]                = hoverColor;
		colors[ImGuiCol_TabSelected]               = clickColor;
		colors[ImGuiCol_TabDimmed]                 = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.30f);
		colors[ImGuiCol_TabDimmedSelected]         = ImVec4(interactColor.x, interactColor.y, interactColor.z, 0.60f);
		colors[ImGuiCol_TabSelectedOverline]       = hoverColor;
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.50f);

		// Separators
		colors[ImGuiCol_Separator]        = ImVec4(borderColor.x, borderColor.y, borderColor.z, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(hoverColor.x, hoverColor.y, hoverColor.z, 0.78f);
		colors[ImGuiCol_SeparatorActive]  = clickColor;

		// Table Rows (Backgrounds for alternating table rows)
		colors[ImGuiCol_TableRowBg]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

		// Navigation (Keyboard/Gamepad)
		colors[ImGuiCol_NavCursor]             = hoverColor;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	}
};

class LauncherMainWindow
{
  private:
	ProfileSettings settingsModal;

  public:
	LauncherMainWindow();
	~LauncherMainWindow() = default;

	// main ImGui render loop
	void Draw();

	// Helper to reload from config file
	void RefreshList();
	void SaveConfig();

  private:
	std::vector<std::string> profilePaths;
	std::vector<Profile>     cachedProfiles; // Store actual data so we don't read JSON every frame

	int selectedProfileIdx = -1;

	// State trackers
	std::string langVar  = "default";
	std::string themeVar = "dark";

	// Modal Triggers
	bool showSettingsModal = false;
	bool showAboutNotes    = false;
	bool showAboutCredits  = false;
	bool showImportPopup   = false;

	// keep track of the last import status
	importStatus lastImportStatus = IMPORT_CANCELLED;

	// Async process tracking
	std::atomic<bool> isAlreadyLaunched{false};

	// Set by background thread when game closes and comes back to signal the UI to refresh playtime and last played
	std::atomic<bool> needsRefresh{false};

	// UI Helpers
	void DrawPopUp();
	void DrawMenuBar();
	void DrawProfileList();
	void DrawButtons();
	void DrawDescriptionBox();

	// Action Helpers
	void LaunchGame(const std::string &mode);
	void MoveSelectedEntry(int offset);
	void UpdateLanguage();
	void ApplyTheme();
	void CloneSelectedProfile();
	void ImportProfileFromZip(const std::string &zipPath);
	void OpenInstallDirectory();
	void FinalizeProfileAddition(const std::string &jsonPath, bool immediateRefresh);
};
