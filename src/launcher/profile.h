/*
** profile.h
**
**
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

#include <QString>
#include <QStringList>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

enum class LaunchTargetMode
{
	Single = 0,
	Host = 1,
	Join = 2
};

struct Profile
{
	std::string title = "New Profile";
	std::string author = "Unknown";
	std::string releaseDate = "Unknown";
	std::string description;
	std::string lastPlayed = "Never";
	uint64_t playtime = 0;
	int orderIndex = -1;
	bool favorite = false;
	std::string coverImagePath;

	bool isIWAD = false;
	std::string iwadPath;
	std::string pwadPath;
	std::vector<std::string> mods;
	std::vector<std::string> disabledMods;
	bool copyModsToProfile = true;

	int launchParameters = 0;
	std::string selectedLaunchMap;
	std::string selectedLaunchSave;
	std::string selectedLaunchDemoPlayback;
	bool recordDemo = false;
	std::string selectedLaunchDemoRecord;

	int difficulty = 0;
	bool fastMonsters = false;
	bool respawnMonsters = false;
	bool noMonsters = false;
	bool allowMultiplayerItems = false;
	bool pistolStartAllLevels = false;

	std::string playerName;
	std::string playerClass;
	int playerGender = 0;
	std::string wadLanguage = "default";

	int compatLevel = 0;
	std::string complevel;
	bool alwaysApplyDMFlags = false;
	uint32_t dmflags = 0;
	uint32_t dmflags2 = 0;
	uint32_t dmflags3 = 0;
	uint32_t compatflags = 0;
	uint32_t compatflags2 = 0;

	std::string remoteAddress = "127.0.0.1";
	int remotePort = 5029;
	int remoteTeam = 255;

	int hostPort = 5029;
	int maxPlayers = 8;
	int hostTicRate = 0;
	bool hostExtraTic = false;
	int hostGamemode = 0;
	int hostTeam = 255;

	std::string configFilePath;
	std::string saveDirPath;
	std::string screenshotDirPath;
	std::string demoDirPath;

	bool fullscreen = true;
	bool loadSupportWads = true;
	bool disableAutoload = false;
	bool enableDynLights = true;
	int renderingAPI = 1;
	bool extraLights = false;
	bool extraBrightmaps = false;
	bool extraWidescreen = false;

	std::string appendAdditionalParameters;

	std::string filePath;

	bool operator==(const Profile&) const = default;

	QStringList buildLaunchArguments(LaunchTargetMode mode, const std::string& launcherLanguage = {}) const;
	std::string giveLaunchCommand(LaunchTargetMode mode, const std::string& launcherLanguage = {}) const;
};

struct LauncherConfig
{
	std::string language = "auto";
	std::string theme = "auto";
	bool hideWhilePlaying = false;
	bool showReleaseNotes = true;
	bool maximized = false;
	int lastSelectedRow = 0;
	int sortColumn = -1;
	bool sortDescending = false;
	int splitterTopSize = 0;
	int splitterBottomSize = 0;
	std::string defaultModsDir;
};

QString formatPlaytime(uint64_t minutes);
std::string sanitizeFolderName(const std::string& name);
std::string findBestDefaultIWADPath(const std::vector<Profile>& existingProfiles);

LauncherConfig loadLauncherConfig();
void saveLauncherConfig(const LauncherConfig& cfg);

void createDefaultDirectoryStructure(Profile& profile);
bool saveProfileToDisk(Profile& profile);
bool loadProfileFromDisk(const std::string& path, Profile& outProfile);
std::vector<Profile> loadAllProfiles();
void persistProfileOrdering(std::vector<Profile>& profiles);

void applyProfilePathMapping(Profile& profile, const std::string& oldBaseDir, const std::string& newBaseDir);
bool cloneProfile(const Profile& source, const std::string& newTitle, Profile& outClone);
bool deleteProfileDirectory(const Profile& profile);
bool exportProfileToZip(const Profile& profile, const std::string& destZipPath, std::string& errOut);
bool importProfileFromZip(const std::string& zipPath, Profile& outProfile, std::string& errOut);
bool extractZipArchiveToDir(const QString& zipPath, const QString& destDir, std::vector<QString>* extractedFiles = nullptr, std::string* errOut = nullptr);
bool extractMatchingEntryFromZip(const QString& zipPath, const std::function<bool(const QString& entryName)>& predicate, const QString& destFolder, const QString& targetBaseName, QString& outExtractedPath);
