/*
** launcherMainWindow.cpp
**
** Creates the main window for the launcher with the profile list and buttons
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

#include "launcherMainWindow.h"
#include "about.h"
#include "gstrings.h"
#include "loader.h"
#include "profileSettings.h"
#include "starter.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <shellapi.h>
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <limits.h>
#include <unistd.h>
#endif

using json      = nlohmann::json;
using TimePoint = std::chrono::system_clock::time_point;

// returns a string that calculates XXhXXm
static std::string getTimeString(std::time_t totalSeconds)
{
	long long hours   = totalSeconds / 3600;
	long long minutes = (totalSeconds % 3600) / 60;

	std::stringstream ss;
	ss << hours << "h" << minutes << "m";
	return ss.str();
}

// Applies a path transformation lambda to every path in a Profile
void ApplyProfilePathMapping(Profile &profile, const std::function<void(std::string &)> &mappingFunc)
{
	mappingFunc(profile.configFilePath);
	mappingFunc(profile.saveDirPath);
	mappingFunc(profile.screenshotDirPath);
	mappingFunc(profile.demoDirPath);
	mappingFunc(profile.modsDirPath);
	mappingFunc(profile.iwadFilePath);
	mappingFunc(profile.pwadFilePath);

	for (std::string &modPath : profile.modFiles)
	{
		mappingFunc(modPath);
	}
}

LauncherMainWindow::LauncherMainWindow()
{
	// Load config on startup
	std::ifstream configFile(CONFIG_FILE);
	if (configFile.is_open())
	{
		try
		{
			json j;
			configFile >> j;

			if (j.contains("lang") && j["lang"].is_string())
				langVar = j["lang"].get<std::string>();

			if (j.contains("theme") && j["theme"].is_string())
				themeVar = j["theme"].get<std::string>();
		}
		catch (const json::parse_error &e)
		{
			std::cerr << "Config JSON parse error: " << e.what() << std::endl;
		}
		configFile.close();
	}

	UpdateLanguage();
	ApplyTheme();
	RefreshList();
}

void LauncherMainWindow::SaveConfig()
{
	json j;
	j["lang"]     = langVar;
	j["theme"]    = themeVar;
	j["profiles"] = profilePaths;

	std::ofstream file(CONFIG_FILE);
	if (file.is_open())
	{
		file << j.dump(4);
		file.close();
	}
}

void LauncherMainWindow::RefreshList()
{
	profilePaths.clear();
	cachedProfiles.clear();

	std::ifstream file(CONFIG_FILE);
	if (file.is_open())
	{
		try
		{
			json j;
			file >> j;
			if (j.contains("profiles") && j["profiles"].is_array())
			{
				profilePaths = j["profiles"].get<std::vector<std::string>>();
			}
		}
		catch (const json::parse_error &e)
		{
			std::cerr << "Config JSON parse error: " << e.what() << std::endl;
			return;
		}
		file.close();
	}

	for (const auto &filepath : profilePaths)
	{
		Profile tempProfile;
		tempProfile.loadFromFile(filepath);
		cachedProfiles.push_back(tempProfile);
	}

	// Reset selection if it goes out of bounds
	if (selectedProfileIdx >= (int)cachedProfiles.size())
	{
		selectedProfileIdx = -1;
	}
}

void LauncherMainWindow::UpdateLanguage()
{
	GStrings.UpdateLanguage(langVar.c_str());
}

void LauncherMainWindow::ApplyTheme()
{
	// Define available themes
	static std::unordered_map<std::string, LauncherTheme> themes = {
		{"dark", LauncherTheme(LauncherTheme::BaseTheme::ImGuiDark)},
		{"light", LauncherTheme(LauncherTheme::BaseTheme::ImGuiLight)},

		// Doom: Deep hellish reds, dark charcoal, and stark text
		{"doom", LauncherTheme(0x1a1515, // bg
	                           0xdfdfdf, // text
	                           0x2b2222, // inputs
	                           0x5e1313, // interact
	                           0x8a1c1c, // hover
	                           0xc72c2c, // click
	                           0x3d2b2b  // border
	                           )},

		// Plutonia sounds green
		{"plutonia", LauncherTheme(0x1e1f1a, // bg
	                               0xdfd8c8, // text
	                               0x2d3025, // inputs
	                               0x475222, // interact
	                               0x5e6e2d, // hover
	                               0x7a8d3b, // click
	                               0x313626  // border
	                               )},

		// Classic: Retro Windows 95 / Win32 vibe
		{"classic", LauncherTheme(0xc0c0c0, // bg
	                              0x000000, // text
	                              0xcccccc, // inputs
	                              0xa0a0a0, // interact
	                              0x002b80, // hover
	                              0x0000ff, // click
	                              0x808080  // border
	                              )}
    };

	if (themes.find(themeVar) != themes.end())
	{
		themes[themeVar].ApplyTheme();
	}
	else
	{
		std::cerr << "Theme not found: " << themeVar << ". Applying default." << std::endl;
		themes["Dark"].ApplyTheme();
	}
}

void LauncherMainWindow::DrawPopUp()
{
	if (showImportPopup)
	{
		ImGui::OpenPopup("UZDoom");
		showImportPopup = false; // Reset the trigger immediately so it only opens once
	}

	// ImGuiWindowFlags_AlwaysAutoResize makes the popup snap to the text size
	if (ImGui::BeginPopupModal("UZDoom", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// Display a message based on the status enum
		switch (lastImportStatus)
		{
		case IMPORT_IWAD_SUCCESS:
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
			                   GStrings.GetString("LAUNCHER_DETECT_IWAD")); // Green text -> OK
			break;
		case IMPORT_PWAD_SUCCESS:
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
			                   GStrings.GetString("LAUNCHER_DETECT_PWAD")); // Green text -> OK
			break;
		case IMPORT_FAIL:
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
			                   GStrings.GetString("LAUNCHER_DETECT_NOWAD")); // Red text -> ERROR
			break;
		case IMPORT_ARCHIVE_FAIL:
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
			                   GStrings.GetString("LAUNCHER_ERROR_NOWADARCH")); // Red text -> ERROR
			break;
		case IMPORT_DUPLICATE:
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
			                   GStrings.GetString("LAUNCHER_ERROR_DUPLICATE")); // Yellow text -> WARNING
			break;
		case IMPORT_CANCELLED:
			ImGui::Text("Import Cancelled.");
			break;
		}

		ImGui::Separator();
		ImGui::Spacing();

		// Center the OK button
		float btnWidth = ImGui::GetFontSize() * 8.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - btnWidth) * 0.5f);
		if (ImGui::Button("OK", ImVec2(btnWidth, 0)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LauncherMainWindow::Draw()
{
	// Check if background thread signaled a refresh (e.g., game closed and we are coming back to launcher)
	if (needsRefresh && !showSettingsModal)
	{
		RefreshList();
		needsRefresh.store(false);
	}

	// Main Window
	ImGuiIO &io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(io.DisplaySize);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
	                               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if (ImGui::Begin("UZDoom Launcher", nullptr, windowFlags))
	{
		DrawMenuBar();

		// Split the layout in 2 (profile list + description on the left, buttons on the right)
		ImGui::Columns(2, "MainColumns", false);
		ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - (ImGui::GetFontSize() * 10.0f));

		// Top-Left: Profile List
		float reserveHeight = ImGui::GetTextLineHeightWithSpacing() * 10.0f;
		ImGui::BeginChild("ProfileListChild", ImVec2(0, -reserveHeight), true);
		DrawProfileList();
		ImGui::EndChild();

		// Bottom-Left: Description Box
		ImGui::BeginChild("DescriptionBoxChild", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
		DrawDescriptionBox();
		ImGui::EndChild();

		ImGui::NextColumn();

		// Right side: Buttons
		ImGui::BeginChild("ButtonsChild", ImVec2(0, 0));
		DrawButtons();
		ImGui::EndChild();

		ImGui::Columns(1);
	}
	ImGui::End();

	// Render the linked ImGui Modals now
	About::DrawReleaseNotesDialog(&showAboutNotes, langVar);
	About::DrawCreditsDialog(&showAboutCredits, langVar);

	DrawPopUp(); // Draw the import status popup if triggered

	if (showSettingsModal && selectedProfileIdx >= 0 && selectedProfileIdx < cachedProfiles.size())
	{
		bool wasOpen = showSettingsModal;

		settingsModal.Draw(&showSettingsModal, &cachedProfiles[selectedProfileIdx], profilePaths[selectedProfileIdx]);

		// If the modal was just closed
		if (wasOpen && !showSettingsModal)
		{
			// Check if it was deleted inside the profile settings modal
			if (!std::filesystem::exists(profilePaths[selectedProfileIdx]))
			{
				profilePaths.erase(profilePaths.begin() + selectedProfileIdx);
				selectedProfileIdx = -1;
				SaveConfig();
			}

			RefreshList();
		}
	}
}

// draws the top menu bar with File, Preferences, and About
void LauncherMainWindow::DrawMenuBar()
{
	std::filesystem::path defaultPath = std::filesystem::path(PROFILE_DIR);

	if (ImGui::BeginMenuBar())
	{

		if (ImGui::BeginMenu(GStrings.GetString("LAUNCHER_TOPBAR_FILE")))
		{

			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_FILEADDWAD")))
			{
				std::string result =
					ProfileSettings::OpenPathPicker(defaultPath, false,
				                                    {
														{"WAD/PKX Files", "wad,pwd,pk3,pk7,iwad,pwad,ipk3,ipk7"}
                });

				if (!result.empty())
				{
					// Capture status and trigger popup
					lastImportStatus = Loader::ProcessWad(result);
					showImportPopup  = true;
					needsRefresh     = true;
				}
			}
			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_FILEADDARCHIVE")))
			{
				std::string result = ProfileSettings::OpenPathPicker(defaultPath, false,
				                                                     {
																		 {"Zip Archives", "zip"}
                });

				if (!result.empty())
				{
					// Capture status and trigger popup
					lastImportStatus = Loader::ProcessArchive(result);
					showImportPopup  = true;
					needsRefresh     = true;
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_IMPORT")))
			{
				// Allow user to import a .zip profile
				std::string result = ProfileSettings::OpenPathPicker(defaultPath, false,
				                                                     {
																		 {"UZdoom Profiles", "uzdp"}
                });

				// Only allow valid Uzdoom profile import
				std::filesystem::path filePath(result);
				if (!result.empty() && filePath.extension().string() == ".uzdp")
				{
					ImportProfileFromZip(result);
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_INSTFOLDER")))
			{
				OpenInstallDirectory();
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_FILEEXIT")))
			{
				// Shutdown SDL directly
				SDL_Event quit_event;
				quit_event.type = SDL_QUIT;
				SDL_PushEvent(&quit_event);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(GStrings.GetString("LAUNCHER_TOPBAR_PREF")))
		{
			if (ImGui::BeginMenu(GStrings.GetString("LAUNCHER_TOPBAR_PREFLANG")))
			{
				const char *langs[] = {"default", "eng", "cs", "da", "de", "es",  "esm", "eo", "fi", "fr", "hu", "it",
				                       "jp",      "ko",  "nl", "no", "pl", "ptg", "pt",  "ro", "ru", "sr", "tr"};
				const char *names[] = {
					"English (US)",      "English (UK)",      "Česky (Czech)",      "Dansk (Danish)",
					"Deutsch (German)",  "Español (España)",  "Español (Latino)",   "Esperanto",
					"Suomi (Finnish)",   "Français (French)", "Magyar (Hungarian)", "Italiano (Italian)",
					"日本語 (Japanese)", "한국어 (Korean)",   "Nederlands (Dutch)", "Norsk Bokmål",
					"Polski (Polish)",   "Português (EU)",    "Português (BR)",     "Română (Romanian)",
					"Русский (Russian)", "Српски (Serbian)",  "Türkçe (Turkish)"};

				for (int i = 0; i < IM_ARRAYSIZE(langs); i++)
				{
					if (ImGui::MenuItem(names[i], "", langVar == langs[i]))
					{
						langVar = langs[i];
						SaveConfig();
						UpdateLanguage();
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(GStrings.GetString("LAUNCHER_TOPBAR_PREFTHEME")))
			{
				if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_THEME_LIGHT"), "", themeVar == "light"))
				{
					themeVar = "light";
					ApplyTheme();
					SaveConfig();
				}
				if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_THEME_DARK"), "", themeVar == "dark"))
				{
					themeVar = "dark";
					ApplyTheme();
					SaveConfig();
				}

				ImGui::Spacing();
				ImGui::Separator(); // separate "default" themes from more custom ones
				ImGui::Spacing();

				if (ImGui::MenuItem("Doom", "", themeVar == "doom"))
				{
					themeVar = "doom";
					ApplyTheme();
					SaveConfig();
				}

				if (ImGui::MenuItem("Plutonia", "", themeVar == "plutonia"))
				{
					themeVar = "plutonia";
					ApplyTheme();
					SaveConfig();
				}

				if (ImGui::MenuItem("Classic", "", themeVar == "classic"))
				{
					themeVar = "classic";
					ApplyTheme();
					SaveConfig();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(GStrings.GetString("LAUNCHER_TOPBAR_ABOUT")))
		{
			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_ABOUTNOTES")))
			{
				showAboutNotes = true;
			}
			if (ImGui::MenuItem(GStrings.GetString("LAUNCHER_TOPBAR_ABOUTCREDITS")))
			{
				showAboutCredits = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void LauncherMainWindow::DrawProfileList()
{
	static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
	                               ImGuiTableFlags_Resizable | ImGuiTableFlags_HighlightHoveredColumn;

	if (ImGui::BeginTable("Profiles", 6, flags))
	{

		// some columns have fixed width based on their content (e.g dates are always same format), others stretch to
		// fill remaining space
		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_TYPE"), ImGuiTableColumnFlags_WidthFixed,
		                        ImGui::CalcTextSize("XXXX_").x);

		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_TITLE"), ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_AUTHORS"), ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_RELEASEDATE"), ImGuiTableColumnFlags_WidthFixed,
		                        ImGui::CalcTextSize("YYYY-MM-DD").x);

		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_LASTPLAYED"), ImGuiTableColumnFlags_WidthFixed,
		                        ImGui::CalcTextSize("YYYY-MM-DD").x);

		ImGui::TableSetupColumn(GStrings.GetString("LAUNCHER_PROFLIST_PLAYTIME"), ImGuiTableColumnFlags_WidthFixed,
		                        ImGui::CalcTextSize("XXXXhXXm_").x);

		ImGui::TableHeadersRow();

		for (int i = 0; i < cachedProfiles.size(); i++)
		{
			ImGui::PushID(profilePaths[i].c_str());

			ImGui::TableNextRow();
			bool isSelected = (selectedProfileIdx == i);

			ImGui::TableSetColumnIndex(0);
			std::string typeStr = (cachedProfiles[i].isIWAD ? "IWAD" : "PWAD");

			// Use Selectable to allow the row to be clickable across all columns
			if (ImGui::Selectable(typeStr.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
			{
				selectedProfileIdx = i;
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(cachedProfiles[i].title.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(cachedProfiles[i].author.c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(cachedProfiles[i].releaseDate.c_str());
			ImGui::TableSetColumnIndex(4);
			ImGui::TextUnformatted(cachedProfiles[i].lastPlayedDate.c_str());
			ImGui::TableSetColumnIndex(5);
			ImGui::TextUnformatted(getTimeString(cachedProfiles[i].playedTime).c_str());

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void LauncherMainWindow::DrawDescriptionBox()
{
	if (selectedProfileIdx >= 0 && selectedProfileIdx < cachedProfiles.size())
	{
		const std::string &desc = cachedProfiles[selectedProfileIdx].description;
		ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
	}
	else
	{
		ImGui::TextDisabled("%s", GStrings.GetString("LAUNCHER_NO_DSC_SELECT"));
	}
}

void LauncherMainWindow::DrawButtons()
{
	ImVec2 btnSize(-FLT_MIN, ImGui::GetFrameHeight() * 1.2f);
	bool   hasSelection = (selectedProfileIdx != -1);

	// when launched, disable all buttons
	bool disableButtons = (!hasSelection || isAlreadyLaunched);

	if (disableButtons)
		ImGui::BeginDisabled();

	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_START"), btnSize))
		LaunchGame("");
	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_JOIN"), btnSize))
		LaunchGame("join");
	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_HOST"), btnSize))
		LaunchGame("host");

	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_SETTING"), btnSize))
	{
		showSettingsModal = true;
	}

	if (disableButtons)
		ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (disableButtons)
		ImGui::BeginDisabled();

	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_MVUP"), btnSize))
		MoveSelectedEntry(-1);
	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_MVDOWN"), btnSize))
		MoveSelectedEntry(1);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_REFRESH"), btnSize))
	{
		RefreshList();
	}

	if (ImGui::Button(GStrings.GetString("LAUNCHER_PROFBUTTON_CLONE"), btnSize))
		CloneSelectedProfile();

	if (disableButtons)
		ImGui::EndDisabled();

	// Available Status and center the calculated text
	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing());
	std::string baseStr = GStrings.GetString("LAUNCHER_AVAIL_STATUS");

	//because dynamic number
	char buffer[256];
	snprintf(buffer, sizeof(buffer), baseStr.c_str(), cachedProfiles.size());

	std::string statusText = buffer;
	float textWidth  = ImGui::CalcTextSize(statusText.c_str()).x;
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textWidth) * 0.5f);
	ImGui::TextDisabled(statusText.c_str());
}

// when user presses UP or DOWN button, adjust selected entry in table
void LauncherMainWindow::MoveSelectedEntry(int offset)
{
	int newIdx = selectedProfileIdx + offset;
	if (newIdx >= 0 && newIdx < profilePaths.size())
	{
		std::swap(profilePaths[selectedProfileIdx], profilePaths[newIdx]);
		SaveConfig();
		RefreshList();
		selectedProfileIdx = newIdx; // keep focus
	}
}

void LauncherMainWindow::CloneSelectedProfile()
{
	if (selectedProfileIdx < 0 || selectedProfileIdx >= profilePaths.size())
		return;

	std::string           originalJsonPath = profilePaths[selectedProfileIdx];
	std::filesystem::path origDir          = std::filesystem::path(originalJsonPath).parent_path();

	std::string           newFolderName = "clone_" + Loader::GenerateTimestampString();
	std::filesystem::path newDir        = origDir.parent_path() / newFolderName;

	// Copy the directory and its contents
	std::filesystem::copy(origDir, newDir, std::filesystem::copy_options::recursive);

	std::filesystem::path copiedJsonPath = newDir / std::filesystem::path(originalJsonPath).filename();
	std::filesystem::path newJsonPath    = newDir / (newFolderName + ".json");

	// Rename the JSON file to match the new folder name
	std::filesystem::rename(copiedJsonPath, newJsonPath);

	Profile clonedProfile;
	clonedProfile.loadFromFile(newJsonPath.string());
	clonedProfile.title += " (Clone)";

	// path remap lambda
	auto remapPath = [&](std::string &pathRef) {
		if (pathRef.empty())
			return;

		std::string normalizedPath = std::filesystem::path(pathRef).generic_string();
		std::string normalizedOrig = origDir.generic_string();

		// Ensure the path strictly STARTS WITH the original directory path
		if (normalizedPath.find(normalizedOrig) == 0)
		{
			std::string subPath = normalizedPath.substr(normalizedOrig.length());

			if (!subPath.empty() && subPath.front() == '/')
				subPath.erase(0, 1);

			pathRef = (newDir / subPath).generic_string();
		}
	};

	ApplyProfilePathMapping(clonedProfile, remapPath);
	clonedProfile.saveToFile(newJsonPath.string());

	FinalizeProfileAddition(newJsonPath.string(), true);
}

// Finalizes the addition of a new profile
void LauncherMainWindow::FinalizeProfileAddition(const std::string &jsonPath, bool immediateRefresh)
{
	profilePaths.push_back(jsonPath);
	SaveConfig();

	if (immediateRefresh)
		RefreshList();
	else
		needsRefresh = true;
}

// import a file from a .uzdp file (technically a zip)
void LauncherMainWindow::ImportProfileFromZip(const std::string &zipPath)
{
	std::string tempFolderName = "temp_import_" + Loader::GenerateTimestampString();
	std::string tempDir        = (std::filesystem::path(PROFILE_DIR) / tempFolderName).string();

	std::filesystem::create_directories(tempDir);

	if (Loader::ExtractArchive(zipPath, tempDir))
	{
		std::string jsonFilename = "";
		for (const auto &entry : std::filesystem::directory_iterator(tempDir))
		{
			if (entry.path().extension() == ".json")
			{
				jsonFilename = entry.path().filename().string();
				break;
			}
		}

		if (!jsonFilename.empty())
		{
			std::string           originalFolderName = std::filesystem::path(jsonFilename).stem().string();
			std::filesystem::path targetDirPath      = std::filesystem::path(PROFILE_DIR) / originalFolderName;

			std::string targetDir     = targetDirPath.string();
			std::string finalJsonPath = (targetDirPath / jsonFilename).generic_string();

			bool isDuplicate = std::filesystem::exists(targetDir);
			if (!isDuplicate)
			{
				for (const std::string &existingPath : profilePaths)
				{
					if (existingPath == finalJsonPath)
					{
						isDuplicate = true;
						break;
					}
				}
			}

			if (isDuplicate)
			{
				std::filesystem::remove_all(tempDir);
				lastImportStatus = IMPORT_DUPLICATE;
				showImportPopup  = true;
				return;
			}

			std::filesystem::rename(tempDir, targetDir);

			Profile importedProfile;
			importedProfile.loadFromFile(finalJsonPath);

			// path rewrite lambda
			auto rewritePath = [&](std::string &pathRef) {
				if (pathRef.empty())
					return;

				std::string                normalizedPath = std::filesystem::path(pathRef).generic_string();
				constexpr std::string_view marker         = "/launcher/";

				size_t pos = normalizedPath.rfind(marker);
				if (pos != std::string::npos)
				{
					std::string subPath = normalizedPath.substr(pos + marker.length());
					pathRef             = (std::filesystem::path(ROOT_DIR) / subPath).generic_string();
				}
			};

			ApplyProfilePathMapping(importedProfile, rewritePath);
			importedProfile.saveToFile(finalJsonPath);

			FinalizeProfileAddition(finalJsonPath, false);
		}
		else
		{
			std::filesystem::remove_all(tempDir);
		}
	}
	else
	{
		std::filesystem::remove_all(tempDir);
	}
}

// launch selected entry
void LauncherMainWindow::LaunchGame(const std::string &mode)
{
	if (isAlreadyLaunched)
		return;
	if (selectedProfileIdx < 0)
		return;

	std::string selectedRowPath = profilePaths[selectedProfileIdx];
	Profile     tp              = cachedProfiles[selectedProfileIdx];

	std::string dispatchedCmd = tp.giveLaunchCommand(selectedRowPath, mode);
	if (dispatchedCmd == GStrings.GetString("LAUNCHER_PROF_EMPTYWAD"))
		return;

	// get executable path and prepend to command (according to plattform)
	std::string exePath = "";
#ifdef _WIN32
	wchar_t path[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, path, MAX_PATH);
	exePath = std::filesystem::path(path).string();
#elif __APPLE__
	char     path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
	{
		exePath = std::filesystem::path(path).string();
	}
#elif __linux__
	char    result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count != -1)
	{
		exePath = std::filesystem::path(std::string(result, count)).string();
	}
#endif

	dispatchedCmd = exePath + " " + dispatchedCmd;

	isAlreadyLaunched       = true;
	TimePoint startingPoint = std::chrono::system_clock::now();

	// Detach a thread to run the process so it doesnt freeze the ImGui Main Loop
	std::thread([this, dispatchedCmd, selectedRowPath, startingPoint]() {

	// Execute the game based on the operating system
#ifdef _WIN32
		// Windows: Launch the game natively
		STARTUPINFOA        si = {sizeof(STARTUPINFOA)};
		PROCESS_INFORMATION pi;

		// CreateProcessA requires a mutable character array
		std::string cmdCopy = dispatchedCmd;

		if (CreateProcessA(NULL, cmdCopy.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			// Block the background thread until the game exits
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
#else
		int retCode = std::system(dispatchedCmd.c_str());
#endif

		// Process finished!
		TimePoint doneTime      = std::chrono::system_clock::now();
		long long secondsPlayed = std::chrono::duration_cast<std::chrono::seconds>(doneTime - startingPoint).count();

		// Update profile disk storage
		Profile p;
		p.loadFromFile(selectedRowPath);

		auto              now       = std::chrono::system_clock::now();
		auto              in_time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%d-%m-%Y");
		p.lastPlayedDate = ss.str();

		p.playedTime += secondsPlayed;
		p.saveToFile(selectedRowPath);

		// Tell the main thread to refresh the UI list
		needsRefresh.store(true);
		isAlreadyLaunched.store(false);
	}).detach();
}

void LauncherMainWindow::OpenInstallDirectory()
{
#ifdef _WIN32
	// Windows Explorer
	ShellExecuteA(NULL, "open", EXEC_DIR.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
	// macOS and Linux
#ifdef __APPLE__
	std::string cmd = "open \"" + EXEC_DIR + "\"";
#else
	std::string cmd = "xdg-open \"" + EXEC_DIR + "\"";
#endif
	std::thread([cmd]() { std::system(cmd.c_str()); }).detach();
#endif
}
