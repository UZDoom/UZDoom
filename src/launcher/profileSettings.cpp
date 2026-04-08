/*
** profileSettings.cpp
**
** Contains UI code and logic for the profiles own Settings tab
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

#include "profileSettings.h"
#include "gstrings.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.cpp" // Required to bind std::string directly to ImGui::InputText
#include <miniz.h>

static void ExportProfileToZip(const std::string &profileJsonPath, const std::string &outZipPath)
{
	std::filesystem::path profileDir = std::filesystem::path(profileJsonPath).parent_path();

	mz_zip_archive zip_archive = {};
	if (!mz_zip_writer_init_heap(&zip_archive, 0, 16 * 1024 * 1024))
		return;

	for (const auto &entry : std::filesystem::recursive_directory_iterator(profileDir))
	{
		std::string relPath = std::filesystem::relative(entry.path(), profileDir).string();

		// Replace backslashes with forward slashes for ZIP compatibility
		std::replace(relPath.begin(), relPath.end(), '\\', '/');

		if (entry.is_regular_file())
		{
			std::string   absPath = entry.path().string();
			std::ifstream file(absPath, std::ios::binary | std::ios::ate);
			if (file.is_open())
			{
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);
				std::vector<uint8_t> buffer(size);

				if (file.read(reinterpret_cast<char *>(buffer.data()), size))
				{
					mz_zip_writer_add_mem(&zip_archive, relPath.c_str(), buffer.data(), size, MZ_DEFAULT_COMPRESSION);
				}
			}
		}
		else if (entry.is_directory())
		{
			// ZIPPED EMPTY folders needs to end with a forward slash
			if (!relPath.empty() && relPath.back() != '/')
			{
				relPath += '/';
			}

			// Add the folders as an empty file with a trailing slash
			mz_zip_writer_add_mem(&zip_archive, relPath.c_str(), nullptr, 0, MZ_DEFAULT_COMPRESSION);
		}
	}

	void  *zip_buffer = nullptr;
	size_t zip_size   = 0;
	mz_zip_writer_finalize_heap_archive(&zip_archive, &zip_buffer, &zip_size);
	mz_zip_writer_end(&zip_archive);

	std::ofstream outFile(outZipPath, std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write(static_cast<const char *>(zip_buffer), zip_size);
		outFile.close();
	}

	mz_free(zip_buffer);
}

// Helper Function with NFD-EX (path given must be absolute and preferred, otherwise give empty string)
std::string ProfileSettings::OpenPathPicker(std::filesystem::path defaultPath, bool isFolder,
                                            const std::vector<nfdu8filteritem_t> &filters = {})
{
	// NFD expects a directory. std::filesystem::is_regular_file checks if parent exists.
	if (std::filesystem::is_regular_file(defaultPath))
	{
		defaultPath = defaultPath.parent_path();
	}

	std::string        nfdDefaultPathStr;
	const nfdu8char_t *nfdDefaultPath = nullptr;

	if (!defaultPath.empty() && std::filesystem::exists(defaultPath))
	{
		nfdDefaultPathStr = defaultPath.string();
		nfdDefaultPath    = nfdDefaultPathStr.c_str();
	}

	// Open the Dialog depending on whats asked
	nfdu8char_t *outPath = nullptr;
	nfdresult_t  result;

	if (isFolder)
	{
		result = NFD_PickFolderU8(&outPath, nfdDefaultPath);
	}
	else
	{
		result = NFD_OpenDialogU8(&outPath, filters.empty() ? nullptr : filters.data(),
		                          static_cast<nfdfiltersize_t>(filters.size()), nfdDefaultPath);
	}

	// Handle correct Result
	if (result == NFD_OKAY)
	{
		std::string selectedPath(outPath);
		NFD_FreePathU8(outPath);
		return selectedPath;
	}

	if (result == NFD_ERROR)
	{
		std::cerr << "NFD Error: " << NFD_GetError() << '\n';
	}

	// Return empty string on NFD_CANCEL or NFD_ERROR
	return {};
}

// Main Window Rendering
void ProfileSettings::Draw(bool *p_open, Profile *currEdit, const std::string &profilePath)
{
	const char *popupId = "###ProfileSettingsModal";

	// If the launcher says we should be open, but ImGui hasn't opened it yet, trigger it!
	if (*p_open && !ImGui::IsPopupOpen(popupId))
	{
		ImGui::OpenPopup(popupId);
	}

	if (!*p_open && !ImGui::IsPopupOpen(popupId))
		return;

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_FirstUseEver);

	// Begin main settings window
	std::string title = GStrings.GetString("PROFSET_TITLE");
	title += popupId;

	// Match colors of the popup
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

	bool isModalOpen =
		ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::PopStyleColor(1);

	if (isModalOpen)
	{

		float bottomSpace = ImGui::GetFrameHeightWithSpacing() * 1.5f;
		ImGui::BeginChild("SettingsTabsRegion", ImVec2(0, -bottomSpace), false);

		if (ImGui::BeginTabBar("ProfileSettingsTabs"))
		{
			if (ImGui::BeginTabItem(GStrings.GetString("PROFSET_TAB_GENERAL")))
			{
				DrawGeneralTab(currEdit);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(GStrings.GetString("PROFSET_TAB_LAUNCH")))
			{
				DrawLaunchTab(currEdit);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(GStrings.GetString("PROFSET_TAB_FILES")))
			{
				DrawFilesTab(currEdit);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(GStrings.GetString("PROFSET_TAB_OUTPUT")))
			{
				DrawOutputTab(currEdit);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem(GStrings.GetString("PROFSET_TAB_ADVANCED")))
			{
				DrawAdvancedTab(currEdit);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();

		ImGui::Separator();
		ImGui::Spacing();

		// Bottom Action Buttons
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		if (ImGui::Button(GStrings.GetString("PROFSET_DELPROF")))
		{
			ImGui::OpenPopup("Delete Confirmation");
		}
		ImGui::PopStyleColor(2);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.1f, 1.0f));
		if (ImGui::Button(GStrings.GetString("PROFSET_EXPPROF")))
		{
			nfdchar_t      *outPath       = nullptr;
			nfdfilteritem_t filterItem[1] = {
				{"Uzdoom Profile", "uzdp"}
            };

			// Sanitize profile title for default filename (remove chars that can't be in filenames)
			std::string defaultName = "Export_" + currEdit->title;
			std::replace(defaultName.begin(), defaultName.end(), ':', '_');
			std::replace(defaultName.begin(), defaultName.end(), '/', '_');
			std::replace(defaultName.begin(), defaultName.end(), '\\', '_');

			nfdresult_t result = NFD_SaveDialogU8(&outPath, filterItem, 1, nullptr, defaultName.c_str());
			if (result == NFD_OKAY)
			{
				ExportProfileToZip(profilePath, outPath);
				NFD_FreePathU8(outPath);
			}
		}
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		float buttonWidth =
			ImGui::CalcTextSize(GStrings.GetString("PROFSET_SV_RT")).x + (ImGui::GetStyle().FramePadding.x * 2.0f);
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - buttonWidth);

		if (ImGui::Button(GStrings.GetString("PROFSET_SV_RT")))
		{
			currEdit->saveToFile(profilePath);
			*p_open = false;            // Tell the Launcher to stop drawing
			ImGui::CloseCurrentPopup(); // Tell ImGui to close the modal stack
		}

		// TODO add translation here
		if (ImGui::BeginPopupModal("Delete Confirmation", NULL,
		                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("%s", GStrings.GetString("PROFSET_DELMSG"));
			ImGui::Spacing();

			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				std::filesystem::path fileP(profilePath);
				std::filesystem::path dirToWipe = fileP.parent_path();
				if (!dirToWipe.empty() && std::filesystem::exists(dirToWipe))
				{
					std::filesystem::remove_all(dirToWipe);
				}
				*p_open = false; // Close settings
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (openFlagEditor)
		{
			ImGui::OpenPopup("Flag Editor");
			openFlagEditor = false;
		}

		// Draw flag editors
		DrawFlagEditorModal(currEdit);

		ImGui::EndPopup();
	}
}

// Tab Implementations

void ProfileSettings::DrawGeneralTab(Profile *currEdit)
{
	ImGui::Spacing();

	// Grid-like layout for labels and inputs
	if (ImGui::BeginTable("GeneralTable", 2, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, 0.0f);
		ImGui::TableSetupColumn("Inputs", ImGuiTableColumnFlags_WidthStretch);

		auto DrawRow = [](const char *label, std::string *targetStr) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s:", label);
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText(std::string("##" + std::string(label)).c_str(), targetStr);
		};

		DrawRow(GStrings.GetString("LAUNCHER_PROFLIST_TITLE"), &currEdit->title);
		DrawRow(GStrings.GetString("LAUNCHER_PROFLIST_AUTHORS"), &currEdit->author);
		DrawRow(GStrings.GetString("LAUNCHER_PROFLIST_RELEASEDATE"), &currEdit->releaseDate);

		// Type Combo
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s", GStrings.GetString("PROFSET_GENERAL_TYPE"));
		ImGui::TableNextColumn();
		const char *typeItems[] = {"PWAD", "IWAD"};
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Combo("##TypeCombo", &currEdit->isIWAD, typeItems, IM_ARRAYSIZE(typeItems));

		// IWAD Path
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("IWAD:");
		ImGui::TableNextColumn();
		float iwadBtnWidth = ImGui::GetFrameHeight();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - iwadBtnWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::InputText("##IWADPath", &currEdit->iwadFilePath);
		ImGui::SameLine();
		if (ImGui::Button("...##iwadbtn", ImVec2(iwadBtnWidth, 0)))
		{
			std::filesystem::path startPath = currEdit->iwadFilePath.empty()
			                                      ? std::filesystem::path(PROFILE_DIR)
			                                      : std::filesystem::path(currEdit->iwadFilePath);
			std::string           newPath   = OpenPathPicker(startPath, false,
			                                                 {
                                                     {"WAD Files", "iwad,wad,pk3"}
            });
			if (!newPath.empty())
				currEdit->iwadFilePath = newPath;
		}

		// Conditionally show PWAD Path
		if (currEdit->isIWAD == 0)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("PWAD:");
			ImGui::TableNextColumn();
			float pwadBtnWidth = ImGui::GetFrameHeight();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - pwadBtnWidth - ImGui::GetStyle().ItemSpacing.x);
			ImGui::InputText("##PWADPath", &currEdit->pwadFilePath);
			ImGui::SameLine();
			if (ImGui::Button("...##pwadbtn", ImVec2(pwadBtnWidth, 0)))
			{
				std::filesystem::path startPath = currEdit->iwadFilePath.empty()
				                                      ? std::filesystem::path(PROFILE_DIR)
				                                      : std::filesystem::path(currEdit->pwadFilePath);
				std::string           newPath   = OpenPathPicker(startPath, false,
				                                                 {
                                                         {"WAD Files", "pwad,wad,pk3"}
                });
				if (!newPath.empty())
					currEdit->pwadFilePath = newPath;
			}
		}
		ImGui::EndTable();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Description Area
	ImGui::Text("%s", GStrings.GetString("PROFSET_GENERAL_DSC"));
	ImGui::InputTextMultiline("##DescText", &currEdit->description, ImVec2(-FLT_MIN, -FLT_MIN));
}

void ProfileSettings::DrawOutputTab(Profile *currEdit)
{
	ImGui::Spacing();

	// Uses inner tables to distribute spacing perfectly
	ImGui::SeparatorText(GStrings.GetString("PROFSET_OUTPUT_GENERAL"));
	if (ImGui::BeginTable("OutGenTable", 3))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_FULLSCREEN"), &currEdit->enableFullscreen);
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_SUPPORTWAD"), &currEdit->enableSupportWAD);
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_DISAUTO"), &currEdit->disableAutoload);
		ImGui::EndTable();
	}
	ImGui::Spacing();

	ImGui::SeparatorText(GStrings.GetString("PROFSET_OUTPUT_RENDER"));
	if (ImGui::BeginTable("OutRenTable", 3))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::RadioButton(GStrings.GetString("PROFSET_OUTPUT_VULKAN"), &currEdit->renderingBackend, 0);
		ImGui::TableNextColumn();
		ImGui::RadioButton(GStrings.GetString("PROFSET_OUTPUT_OPENGL"), &currEdit->renderingBackend, 1);
		ImGui::TableNextColumn();
		ImGui::RadioButton(GStrings.GetString("PROFSET_OUTPUT_GLES"), &currEdit->renderingBackend, 2);
		ImGui::EndTable();
	}
	ImGui::Spacing();

	ImGui::SeparatorText(GStrings.GetString("PROFSET_OUTPUT_EXTRA"));
	if (ImGui::BeginTable("OutExtTable", 3))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_LIGHTS"), &currEdit->enableLights);
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_BRIGHT"), &currEdit->enableBrightmaps);
		ImGui::TableNextColumn();
		ImGui::Checkbox(GStrings.GetString("PROFSET_OUTPUT_WIDE"), &currEdit->enableWidescreen);
		ImGui::EndTable();
	}
}

void ProfileSettings::DrawFilesTab(Profile *currEdit)
{
	ImGui::Spacing();

	// Path Pickers Table
	if (ImGui::BeginTable("FilesTable", 2, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Labels", ImGuiTableColumnFlags_WidthFixed, 0.0f);
		ImGui::TableSetupColumn("Inputs", ImGuiTableColumnFlags_WidthStretch);

		auto DrawPathPickerRow = [&](const char *label, std::string *targetVar, bool isFolder,
		                             const std::vector<nfdfilteritem_t> &filters) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label);
			ImGui::TableNextColumn();

			float buttonWidth = ImGui::GetFrameHeight();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
			ImGui::InputText(std::string("##txt" + std::string(label)).c_str(), targetVar);

			ImGui::SameLine();

			if (ImGui::Button(std::string("...##btn" + std::string(label)).c_str(), ImVec2(buttonWidth, 0)))
			{
				std::filesystem::path startPath =
					targetVar->empty() ? std::filesystem::path(PROFILE_DIR) : std::filesystem::path(*targetVar);

				std::string newPath = OpenPathPicker(startPath, isFolder, filters);
				if (!newPath.empty())
					*targetVar = newPath;
			}
		};

		DrawPathPickerRow(GStrings.GetString("PROFSET_FILES_CONFIG"), &currEdit->configFilePath, false,
		                  {
							  {"INI Files", "ini"}
        });
		DrawPathPickerRow(GStrings.GetString("PROFSET_FILES_SAVE"), &currEdit->saveDirPath, true, {});
		DrawPathPickerRow(GStrings.GetString("PROFSET_FILES_SCREEN"), &currEdit->screenshotDirPath, true, {});
		DrawPathPickerRow(GStrings.GetString("PROFSET_FILES_DEMO"), &currEdit->demoDirPath, true, {});

		ImGui::EndTable();
	}

	ImGui::Spacing();

	// Mods Area
	ImGui::SeparatorText(GStrings.GetString("PROFSET_FILES_MODS"));
	if (ImGui::Button(GStrings.GetString("PROFSET_FILES_ADDMOD")))
	{
		const nfdpathset_t *pathSet       = nullptr;
		nfdfilteritem_t     filterItem[1] = {
            {"Mod Files (.wad,.pk3,.pk7,.zip)", "wad,pk3,pk7,zip"}
        };

		// Allow user to select multiple files, then iterate to add them
		nfdresult_t result = NFD_OpenDialogMultipleU8(&pathSet, filterItem, 1, nullptr);

		if (result == NFD_OKAY)
		{
			nfdpathsetsize_t numPaths;
			NFD_PathSet_GetCount(pathSet, &numPaths);

			for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
			{
				nfdchar_t *originalPath;
				NFD_PathSet_GetPathU8(pathSet, i, &originalPath);

				std::filesystem::path src(originalPath);
				std::filesystem::path dest = std::filesystem::path(currEdit->modsDirPath) / src.filename();

				// Copy the file, overwriting if the user is replacing an existing mod of the same name
				std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing);

				currEdit->modFiles.push_back(dest.generic_string());

				NFD_FreePathU8(originalPath); // Free individual path
			}
			NFD_PathSet_Free(pathSet); // Free the entire path set
		}
		else if (result == NFD_ERROR)
		{
			std::cerr << "NFD Multi-Select Error: " << NFD_GetError() << std::endl;
		}
	}

	// Scrollable child window for the mods list
	ImGui::BeginChild("ModList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
	for (size_t i = 0; i < currEdit->modFiles.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));

		std::string fname = std::filesystem::path(currEdit->modFiles[i]).filename().string();

		ImGui::BeginGroup(); // Group items per row

		// Up Arrow
		ImGui::BeginDisabled(i == 0);
		if (ImGui::Button("▲", ImVec2(35, 35)))
		{
			std::swap(currEdit->modFiles[i], currEdit->modFiles[i - 1]);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		// Down Arrow
		ImGui::BeginDisabled(i == currEdit->modFiles.size() - 1);
		if (ImGui::Button("▼", ImVec2(35, 35)))
		{
			std::swap(currEdit->modFiles[i], currEdit->modFiles[i + 1]);
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		// Delete (X)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		if (ImGui::Button("X", ImVec2(35, 35)))
		{
			std::filesystem::remove(currEdit->modFiles[i]);
			currEdit->modFiles.erase(currEdit->modFiles.begin() + i);
			ImGui::PopStyleColor();
			ImGui::EndGroup();
			ImGui::PopID();
			break; // Break and redraw next frame to prevent iterator invalidation errors
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		// Label
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(fname.c_str());

		ImGui::EndGroup();
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void ProfileSettings::DrawAdvancedTab(Profile *currEdit)
{
	ImGui::Spacing();
	ImGui::SeparatorText(GStrings.GetString("PROFSET_ADVANCED_APPEND"));
	ImGui::InputTextMultiline("##AppendParams", &currEdit->appendAdditionalParameters, ImVec2(-FLT_MIN, -FLT_MIN));
}

void ProfileSettings::DrawLaunchTab(Profile *currEdit)
{
	ImGui::Spacing();

	// Use a two-column layout for Left / Right sides
	if (ImGui::BeginTable("LaunchTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
	{
		// ------------------ LEFT COLUMN ------------------
		ImGui::TableNextColumn();

		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_MODE"));

		if (ImGui::BeginTable("LaunchModeInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::RadioButton(GStrings.GetString("PROFSET_LAUNCH_NORMAL"), &currEdit->launchParameters, 0);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::RadioButton(GStrings.GetString("PROFSET_LAUNCH_MAP"), &currEdit->launchParameters, 1);
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragInt("##MapSpinner", &currEdit->selectedLaunchMap, 1.0f, 1, 100000, "%d",
			               ImGuiSliderFlags_AlwaysClamp);

			auto DrawModePath = [&](int modeVal, const char *label, std::string *targetVar,
			                        const std::vector<nfdfilteritem_t> &filters) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::RadioButton(label, &currEdit->launchParameters, modeVal);
				ImGui::TableNextColumn();
				float buttonWidth = ImGui::GetFrameHeight();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttonWidth -
				                        ImGui::GetStyle().ItemSpacing.x);
				ImGui::InputText(std::string("##T" + std::to_string(modeVal)).c_str(), targetVar);
				ImGui::SameLine();
				if (ImGui::Button(std::string("...##B" + std::to_string(modeVal)).c_str(), ImVec2(buttonWidth, 0)))
				{
					std::filesystem::path startPath =
						targetVar->empty() ? std::filesystem::path(PROFILE_DIR) : std::filesystem::path(*targetVar);

					std::string newPath = OpenPathPicker(startPath, false, filters);
					if (!newPath.empty())
						*targetVar = newPath;
				}
			};

			DrawModePath(2, GStrings.GetString("PROFSET_LAUNCH_SAVE"), &currEdit->selectedLaunchSave,
			             {
							 {"ZDS Files", "zds"}
            });
			DrawModePath(3, GStrings.GetString("PROFSET_LAUNCH_PLAYDEM"), &currEdit->selectedLaunchDemoPlayback,
			             {
							 {"LMP Files", "lmp"}
            });
			DrawModePath(4, GStrings.GetString("PROFSET_LAUNCH_RECORD"), &currEdit->selectedLaunchDemoRecord,
			             {
							 {"LMP Files", "lmp"}
            });

			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_GAMEPLAY"));

		if (ImGui::BeginTable("GameplayInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_SKILL"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *skills[] = {
				GStrings.GetString("PROFSET_LAUNCH_SKILL1"), GStrings.GetString("PROFSET_LAUNCH_SKILL2"),
				GStrings.GetString("PROFSET_LAUNCH_SKILL3"), GStrings.GetString("PROFSET_LAUNCH_SKILL4"),
				GStrings.GetString("PROFSET_LAUNCH_SKILL5")};
			ImGui::Combo("##Skill", &currEdit->difficultySkillRating, skills, IM_ARRAYSIZE(skills));

			ImGui::EndTable();
		}

		if (ImGui::BeginTable("MonstersInner", 3, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Checkbox(GStrings.GetString("PROFSET_LAUNCH_FMONSTER"), &currEdit->difficultyFastMonsters);
			ImGui::TableNextColumn();
			ImGui::Checkbox(GStrings.GetString("PROFSET_LAUNCH_RMONSTER"), &currEdit->difficultyRespawnMonsters);
			ImGui::TableNextColumn();
			ImGui::Checkbox(GStrings.GetString("PROFSET_LAUNCH_NMONSTER"), &currEdit->difficultyNoMonsters);
			ImGui::EndTable();
		}

		if (ImGui::Button(GStrings.GetString("PROFSET_LAUNCH_GAMEPLAYMORE"), ImVec2(-FLT_MIN, 0)))
		{
			tempFlags[0]      = currEdit->DMFlags;
			tempFlags[1]      = currEdit->DMFlags2;
			tempFlags[2]      = currEdit->DMFlags3;
			tempFlagsCount    = 3;
			isGameplayFlags   = true;
			forceDmFlags      = currEdit->alwaysapplydmflags;
			currentModalTitle = GStrings.GetString("PROFSET_GAMEPLAY_TITLE");
			openFlagEditor    = true;
		}

		ImGui::Spacing();
		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_MISC"));

		if (ImGui::BeginTable("MiscInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

			auto DrawTextRow = [](const char *label, std::string *targetVar) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", label);
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-FLT_MIN);
				ImGui::InputText(std::string("##" + std::string(label)).c_str(), targetVar);
			};

			DrawTextRow(GStrings.GetString("PROFSET_LAUNCH_NAME"), &currEdit->playerName);
			DrawTextRow(GStrings.GetString("PROFSET_LAUNCH_CLASS"), &currEdit->playerClass);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_GENDER"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *genders[]      = {"Male", "Female", "Neutral", "Object"};
			const char *genderLabels[] = {
				GStrings.GetString("PROFSET_LAUNCH_GENDER0"), GStrings.GetString("PROFSET_LAUNCH_GENDER1"),
				GStrings.GetString("PROFSET_LAUNCH_GENDER2"), GStrings.GetString("PROFSET_LAUNCH_GENDER3")};
			int genderIdx = 0;
			for (int i = 0; i < 4; ++i)
				if (currEdit->playerGender == genders[i])
					genderIdx = i;
			if (ImGui::Combo("##Gender", &genderIdx, genderLabels, IM_ARRAYSIZE(genderLabels)))
				currEdit->playerGender = genders[genderIdx];

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_LANG"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			struct LangPair
			{
				const char *label;
				const char *val;
			};
			LangPair langs[] = {
				{     "enu - English (US)", "enu"},
                {     "eng - English (UK)", "eng"},
                {     "cs - Česky (Czech)",  "cs"},
				{    "da - Dansk (Danish)",  "da"},
                {  "de - Deutsch (German)",  "de"},
                {  "es - Español (España)",  "es"},
				{ "esm - Español (Latino)", "esm"},
                {         "eo - Esperanto",  "eo"},
                {   "fi - Suomi (Finnish)",  "fi"},
				{ "fr - Français (French)",  "fr"},
                {"hu - Magyar (Hungarian)",  "hu"},
                {"it - Italiano (Italian)",  "it"},
				{ "jp - 日本語 (Japanese)",  "jp"},
                {   "ko - 한국어 (Korean)",  "ko"},
                {"nl - Nederlands (Dutch)",  "nl"},
				{      "nb - Norsk Bokmål",  "nb"},
                {   "pl - Polski (Polish)",  "pl"},
                {   "ptg - Português (EU)", "ptg"},
				{    "pt - Português (BR)",  "pt"},
                { "ro - Română (Romanian)",  "ro"},
                { "ru - Русский (Russian)",  "ru"},
				{  "sr - Српски (Serbian)",  "sr"},
                {  "tr - Türkçe (Turkish)",  "tr"}
            };

			int langIdx = 0;
			for (int i = 0; i < IM_ARRAYSIZE(langs); ++i)
				if (currEdit->wadLanguage == langs[i].val)
					langIdx = i;
			if (ImGui::BeginCombo("##Lang", langs[langIdx].label))
			{
				for (int i = 0; i < IM_ARRAYSIZE(langs); ++i)
				{
					bool is_selected = (langIdx == i);
					if (ImGui::Selectable(langs[i].label, is_selected))
						currEdit->wadLanguage = langs[i].val;
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::EndTable();
		}

		// ------------------ RIGHT COLUMN ------------------
		ImGui::TableNextColumn();

		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_REMOTE"));

		auto DrawRightInput = [](const char *label, std::string *targetVar) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label);
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText(std::string("##R" + std::string(label)).c_str(), targetVar);
		};

		if (ImGui::BeginTable("RemoteInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);
			DrawRightInput(GStrings.GetString("PROFSET_LAUNCH_REMADDR"), &currEdit->joinAddress);
			DrawRightInput(GStrings.GetString("PROFSET_LAUNCH_REMPORT"), &currEdit->joinPort);
			DrawRightInput(GStrings.GetString("PROFSET_LAUNCH_TEAMNO"), &currEdit->joinTeamNo);

			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_HOSTMP"));

		if (ImGui::BeginTable("HostInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

			DrawRightInput(GStrings.GetString("PROFSET_LAUNCH_HOPORT"), &currEdit->hostPort);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_HOMAXP"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragInt("##MaxP", &currEdit->hostMaxPlayers, 1.0f, 1, 64, "%d", ImGuiSliderFlags_AlwaysClamp);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_HOTICK"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *ticks[]      = {"35Hz", "17.5Hz", "11.6Hz"};
			const char *tickLabels[] = {GStrings.GetString("PROFSET_LAUNCH_TICK35"),
			                            GStrings.GetString("PROFSET_LAUNCH_TICK17_5"),
			                            GStrings.GetString("PROFSET_LAUNCH_TICK11_6")};
			int         tickIdx      = 0;
			for (int i = 0; i < 3; ++i)
				if (currEdit->hostTickRate == ticks[i])
					tickIdx = i;
			if (ImGui::Combo("##Tick", &tickIdx, tickLabels, IM_ARRAYSIZE(tickLabels)))
				currEdit->hostTickRate = ticks[tickIdx];

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_HOGAME"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *modes[]      = {"Cooperative", "Team Deathmatch", "Alt. Team Deathmatch", "Deathmatch",
			                            "Alt. Deathmatch"};
			const char *modeLabels[] = {
				GStrings.GetString("PROFSET_LAUNCH_COOP"), GStrings.GetString("PROFSET_LAUNCH_TDM"),
				GStrings.GetString("PROFSET_LAUNCH_ATDM"), GStrings.GetString("PROFSET_LAUNCH_DM"),
				GStrings.GetString("PROFSET_LAUNCH_ADM")};
			int modeIdx = 0;
			for (int i = 0; i < 5; ++i)
				if (currEdit->hostGamemode == modes[i])
					modeIdx = i;
			if (ImGui::Combo("##Mode", &modeIdx, modeLabels, IM_ARRAYSIZE(modeLabels)))
				currEdit->hostGamemode = modes[modeIdx];

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_HOMODE"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *nModes[]      = {"Packet Server", "Peer-to-Peer"};
			const char *nModeLabels[] = {GStrings.GetString("PROFSET_LAUNCH_PACKETS"),
			                             GStrings.GetString("PROFSET_LAUNCH_PEER")};
			int         nIdx          = 0;
			for (int i = 0; i < 2; ++i)
				if (currEdit->hostNetworkMode == nModes[i])
					nIdx = i;
			if (ImGui::Combo("##NMode", &nIdx, nModeLabels, IM_ARRAYSIZE(nModeLabels)))
				currEdit->hostNetworkMode = nModes[nIdx];

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_FRAGLIM"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragInt("##MPfrg", &currEdit->mpFragLimit, 1.0f, 0, 100000, "%d", ImGuiSliderFlags_AlwaysClamp);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_TIMELIM"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragFloat("##MPtim", &currEdit->mpTimeLimit, 1.0f, 0.0f, 9999999.0f, "%.5f",
			                 ImGuiSliderFlags_AlwaysClamp);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_TEAMDMG"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragFloat("##Tdmg", &currEdit->teamDamageFactor, 1.0f, 0.0f, 9999999.0f, "%.5f",
			                 ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndTable();
		}

		ImGui::Spacing();
		ImGui::SeparatorText(GStrings.GetString("PROFSET_LAUNCH_COMP"));

		if (ImGui::BeginTable("CompInner", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 0.0f);
			ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", GStrings.GetString("PROFSET_LAUNCH_COMPPRE"));
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			const char *comps[] = {
				GStrings.GetString("PROFSET_LAUNCH_COMP0"), GStrings.GetString("PROFSET_LAUNCH_COMP1"),
				GStrings.GetString("PROFSET_LAUNCH_COMP2"), GStrings.GetString("PROFSET_LAUNCH_COMP3"),
				GStrings.GetString("PROFSET_LAUNCH_COMP4"), GStrings.GetString("PROFSET_LAUNCH_COMP5"),
				GStrings.GetString("PROFSET_LAUNCH_COMP6"), GStrings.GetString("PROFSET_LAUNCH_COMP7"),
				GStrings.GetString("PROFSET_LAUNCH_COMP8"), GStrings.GetString("PROFSET_LAUNCH_COMP9")};
			ImGui::Combo("##Compat", &currEdit->compatLevel, comps, IM_ARRAYSIZE(comps));
			ImGui::EndTable();
		}

		if (ImGui::Button(GStrings.GetString("PROFSET_LAUNCH_COMPCUST"), ImVec2(-FLT_MIN, 0)))
		{
			tempFlags[0]      = currEdit->compatflags;
			tempFlags[1]      = currEdit->compatflags2;
			tempFlagsCount    = 2;
			isGameplayFlags   = false;
			currentModalTitle = GStrings.GetString("PROFSET_COMP_TITLE");
			openFlagEditor    = true;
		}

		ImGui::EndTable();
	}
}

// Modal ShowFlagEditor
void ProfileSettings::DrawFlagEditorModal(Profile *currEdit)
{
	// Need a unified ID string to open and check
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 30.0f, ImGui::GetFontSize() * 20.0f),
	                         ImGuiCond_FirstUseEver);

	if (ImGui::BeginPopupModal("Flag Editor", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar))
	{

		// List of checkboxes inside a scrolling region
		float reservedBottomSpace = ImGui::GetFrameHeightWithSpacing() * (tempFlagsCount + 2.5f);
		ImGui::BeginChild("FlagScrollRegion", ImVec2(0, -reservedBottomSpace), true);

		// Uses reference or creates one efficiently, saves heap allocation overhead slightly vs recreating dynamically
		// inside the loop
		const auto currentList = isGameplayFlags ? getDMFlagsList() : getCompatFlagsList();

		for (size_t i = 0; i < currentList.size(); ++i)
		{
			auto &f = currentList[i];

			// Check bit state using current tempFlags
			bool isSet   = (tempFlags[f.setIdx] & f.bitVal) == f.bitVal;
			bool uiState = f.invert ? !isSet : isSet;

			if (ImGui::Checkbox(std::string(f.label + "##" + std::to_string(i)).c_str(), &uiState))
			{
				// If clicked, flip the bit in the temporary integer array
				bool wantsSet = f.invert ? !uiState : uiState;
				if (wantsSet)
				{
					tempFlags[f.setIdx] |= f.bitVal;
				}
				else
				{
					tempFlags[f.setIdx] &= ~f.bitVal;
				}
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();

				// squish text into a neat box
				ImGui::PushTextWrapPos(400.0f);
				ImGui::TextUnformatted(f.tooltip.c_str());

				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
		ImGui::EndChild();

		// Options below the list
		if (isGameplayFlags)
		{
			const char *forceText = GStrings.GetString("PROFSET_GAMEPLAY_FORCE");

			// Calculate total width of the checkbox (box height + spacing + text width)
			float checkboxWidth =
				ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(forceText).x;

			// Center the cursor
			ImGui::SetCursorPosX((ImGui::GetWindowSize().x - checkboxWidth) * 0.5f);
			ImGui::Checkbox(forceText, &forceDmFlags);
		}
		else
		{
			const char *compInfoText = GStrings.GetString("PROFSET_COMP_INFO");
			float       textWidth    = ImGui::CalcTextSize(compInfoText).x;
			float       windowWidth  = ImGui::GetWindowSize().x;

			// If the text is shorter than the window, center it cleanly.
			if (textWidth < windowWidth)
			{
				ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
				ImGui::Text("%s", compInfoText);
			}
			else
			{
				// Fallback to wrapping if the string is too long to fit on a single centered line (maybe for some
				// translations?)
				ImGui::TextWrapped("%s", compInfoText);
			}
		}

		ImGui::Spacing();

		// Raw Integer Textboxes updating the bits bidirectionally

		float btnWidth = ImGui::GetFontSize() * 8.0f;

		const char *const compatLabels[] = {"compatflags:", "compatflags2:", ""};
		const char *const dmLabels[]     = {"dmflags:", "dmflags2:", "dmflags3:"};

		const char *const *labels = isGameplayFlags ? dmLabels : compatLabels;

		for (int i = 0; i < tempFlagsCount; ++i)
		{
			ImGui::Text("%s", labels[i]);
			ImGui::SameLine(ImGui::GetFontSize() * 15.0f);
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 250.0f - ImGui::GetStyle().WindowPadding.x);
			ImGui::InputInt(std::string("##TFlags" + std::to_string(i)).c_str(), &tempFlags[i], 0, 0);
		}

		ImGui::Separator();

		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - btnWidth) * 0.5f);
		if (ImGui::Button("OK", ImVec2(btnWidth, 0)))
		{
			// Apply temp array back to profile
			if (isGameplayFlags)
			{
				currEdit->DMFlags            = tempFlags[0];
				currEdit->DMFlags2           = tempFlags[1];
				currEdit->DMFlags3           = tempFlags[2];
				currEdit->alwaysapplydmflags = forceDmFlags;
			}
			else
			{
				currEdit->compatflags  = tempFlags[0];
				currEdit->compatflags2 = tempFlags[1];
			}
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		ImGui::EndPopup();
	}
}
