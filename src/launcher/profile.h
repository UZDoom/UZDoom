/*
** profile.h
**
** Contains the full definition of a launcher profile
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
#include <chrono>
#include <string>
#include <vector>
#include "gstrings.h"

class Profile
{

  public:

	std::string title{};
	std::string author      = "?";
	std::string releaseDate = "?"; // DD-MM-YYYY
	std::string lastPlayedDate = "-"; // Current DD-MM-YYYY set from a conveterted time to string when any 3 button is pressed to launch
	std::time_t playedTime{}; // stored as a time value to be converted to Xh:Ymin, starts at 0

	std::string description = GStrings.GetString("LAUNCHER_NO_DSC_AVAIL"); // Description of the WAD
	int isIWAD{};                               // Is this an IWAD -> 1 or PWAD -> 0
	std::string iwadFilePath{};                 // Path to the main WAD file for this profile
	std::string pwadFilePath{};                 // Path to the main PWAD file for this profile

	int      launchParameters{}; // Stores the mode. e.g Normal,mapstart etc
	int      selectedLaunchMap = 1;
	std::string selectedLaunchSave{};
	std::string selectedLaunchDemoPlayback{};
	std::string selectedLaunchDemoRecord{};

	int  difficultySkillRating = 2; // Custom difficulty parameters for this profile
	bool difficultyFastMonsters{};
	bool difficultyRespawnMonsters{};
	bool difficultyNoMonsters{};

	int compatLevel = 0; // Custom difficulty parameters for this profile (more can be set in the UI)
	std::string playerName = "Player";   //Player name
	std::string playerClass = "Fighter";  //Player class
	std::string playerGender = "Male"; // Player gender
	std::string wadLanguage = "enu";     // selected language for this wad

	std::string hostPort        = "5029";
	int      hostMaxPlayers  = 8;
	std::string hostTickRate    = "35Hz";
	std::string hostGamemode    = "Cooperative";
	std::string hostNetworkMode = "Packet Server";
	std::string joinAddress{};
	std::string joinPort   = "5029";
	std::string joinTeamNo = "255";
	int mpFragLimit      = 0;
	float mpTimeLimit      = 0;
	float teamDamageFactor = 0;

	int  DMFlags{}, DMFlags2{}, DMFlags3{}; // flags used for deathmatch/mp
	bool alwaysapplydmflags{};
	int  compatflags{}, compatflags2{}; // flags used for compat

	std::string configFilePath{};  // Path to the config file to use with this profile
	std::string saveDirPath{};     // Path to the save directory to use with this profile
	std::string screenshotDirPath{}; // Path to the screenshot directory to use with this profile
	std::string demoDirPath{};       // Path to the demo directory to use with this profile
	std::string modsDirPath{};       // Path to the mods directory to use with this profile

	std::vector<std::string> modFiles{}; // List of mod files to load with this profile, MUST BE ORDERED THIS WAY

	bool enableFullscreen = false; // Fullscreen, Support WADs , etc
	bool enableSupportWAD = false;
	bool disableAutoload  = false;

	int renderingBackend = 0; // Rendering backend to use with this profile 0 - VK / 1 - GL / 2 - GL ES

	bool enableLights     = false; // Extra graphics parameters
	bool enableBrightmaps = false; // Extra graphics parameters
	bool enableWidescreen = false; // Extra graphics parameters

	std::string prependAdditionalParameters{}; // Any additional parameters not covered
	std::string appendAdditionalParameters{};  // Any additional parameters not covered

	// write profile to file
	void saveToFile(const std::string &filepath);

	// load profile from file
	void loadFromFile(const std::string &filepath);

	// build launch command
	std::string giveLaunchCommand(const std::string &filepath, const std::string &mode);
};
