/*
** loader.h
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

#include "const.h"
#include "profile.h"

#include <atomic>
#include <string>
#include <vector>

enum class ImportResult
{
	Success,
	UnsupportedFile,
	DuplicateProfile,
	ExtractionFailed,
	Cancelled
};

struct ImportOutcome
{
	ImportResult status = ImportResult::UnsupportedFile;
	std::string message;
	Profile createdProfile;
};

struct DiscoveredIWadCandidate
{
	std::string absPath;
	std::string md5;
	std::string title;
	std::string author;
	std::string releaseDate;
};

std::string calculateFileMD5(const std::string& filepath);
bool isWadHeaderIWAD(const std::string& filepath);
bool tryExtractCoverForProfile(Profile& prof);

ImportOutcome importGameFileIntoProfiles(const std::string& filePath, const std::vector<Profile>& existingProfiles);

std::vector<DiscoveredIWadCandidate> scanUnregisteredIWADs(
	const std::vector<Profile>& existingProfiles,
	const std::vector<DiscoveredIWad>& engineIWads,
	const std::vector<std::string>& extraScanDirs,
	const std::atomic<bool>* cancelFlag = nullptr);

int commitDiscoveredIWADs(std::vector<Profile>& profiles, const std::vector<DiscoveredIWadCandidate>& candidates);
int autoDetectAndImportIWADs(std::vector<Profile>& profiles);
