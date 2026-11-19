/*
** profile.cpp
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

#include "profile.h"
#include "const.h"
#include "loader.h"
#include "doomdef.h"

#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <miniz.h>

namespace
{
	bool copyDirectoryRecursively(const QString& srcPath, const QString& dstPath, bool skipRootLogs = false)
	{
		const QDir srcDir(srcPath);
		if (!srcDir.exists())
			return false;
		if (!QDir().mkpath(dstPath))
			return false;

		const QFileInfoList entries = srcDir.entryInfoList(
			QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);
		for (const QFileInfo& entry : entries)
		{
			if (skipRootLogs)
			{
				if (entry.isDir() && entry.fileName().compare(QStringLiteral("logs"), Qt::CaseInsensitive) == 0)
					continue;
				if (entry.isFile() && entry.fileName().compare(QStringLiteral("last_launch.log"), Qt::CaseInsensitive) == 0)
					continue;
			}
			const QString target = QDir(dstPath).filePath(entry.fileName());
			if (entry.isDir())
			{
				if (!copyDirectoryRecursively(entry.absoluteFilePath(), target, false))
					return false;
			}
			else
			{
				if (QFile::exists(target))
					QFile::remove(target);
				if (!QFile::copy(entry.absoluteFilePath(), target))
					return false;
			}
		}
		return true;
	}

	QString normalizeSlash(const QString& p)
	{
		return QDir::cleanPath(p).replace('\\', '/');
	}

	void appendModArg(QStringList& args, const QString& path)
	{
		if (path.isEmpty())
			return;
		const QString lower = path.toLower();
		if (lower.endsWith(".deh"))
			args << "-deh" << path;
		else if (lower.endsWith(".bex"))
			args << "-bex" << path;
		else
			args << "-file" << path;
	}

	QString resolveRelativeToDir(const QString& path, const QString& baseDir)
	{
		if (path.isEmpty())
			return {};
		const QFileInfo fi(path);
		if (fi.isAbsolute() || baseDir.isEmpty())
			return path;
		return QDir(baseDir).filePath(path);
	}

	QString remapSinglePath(const QString& original, const QString& oldBase, const QString& newBase)
	{
		if (original.isEmpty() || newBase.isEmpty())
			return original;
		const QString normOrig = normalizeSlash(original);
		const QString normOld = oldBase.isEmpty() ? QString() : normalizeSlash(oldBase);
		const QString normNew = normalizeSlash(newBase);
		if (!normOld.isEmpty())
		{
			if (normOrig.compare(normOld, Qt::CaseInsensitive) == 0)
				return normNew;
			const QString oldPrefix = normOld.endsWith('/') ? normOld : (normOld + '/');
			if (normOrig.startsWith(oldPrefix, Qt::CaseInsensitive))
			{
				const QString rel = normOrig.mid(oldPrefix.size());
				return normNew.endsWith('/') ? (normNew + rel) : (normNew + '/' + rel);
			}
			return original;
		}

		// Fallback only when oldBase could not be inferred: remap a legacy /profiles/<dir>/<rel>
		// entry only if the corresponding target file/dir actually exists under newBase.
		const QString profilesToken = QStringLiteral("/profiles/");
		const int idx = normOrig.indexOf(profilesToken, 0, Qt::CaseInsensitive);
		if (idx >= 0)
		{
			const int nextSlash = normOrig.indexOf('/', idx + profilesToken.size());
			if (nextSlash >= 0)
			{
				const QString rel = normOrig.mid(nextSlash + 1);
				const QString candidate = normNew.endsWith('/') ? (normNew + rel) : (normNew + '/' + rel);
				if (QFileInfo::exists(candidate))
					return candidate;
			}
		}
		return original;
	}

	QString quoteShellArg(const QString& arg)
	{
		if (!arg.contains(' ') && !arg.contains('\t') && !arg.contains('"'))
			return arg;
		QString escaped = arg;
		escaped.replace('"', "\\\"");
		return '"' + escaped + '"';
	}
}

QString formatPlaytime(uint64_t minutes)
{
	const uint64_t hours = minutes / 60;
	const uint64_t mins = minutes % 60;
	return QString("%1h:%2min").arg(hours).arg(mins);
}

std::string normalizeReleaseDate(const std::string& raw)
{
	QString s = qStr(raw).trimmed();
	if (s.isEmpty() || s.compare("Unknown", Qt::CaseInsensitive) == 0)
		return "Unknown";

	// Strip common trailing qualifiers/parentheticals only when parsing
	QString cleaned = s;
	cleaned.replace(QRegularExpression("\\b(\\d{1,2})(?:st|nd|rd|th)\\b", QRegularExpression::CaseInsensitiveOption), "\\1");
	cleaned.replace(QRegularExpression("\\s+"), " ");

	auto fmt = [](const QDate& d) -> std::string
	{
		return stdStr(d.toString(QStringLiteral("dd-MM-yyyy")));
	};

	// Numeric forms: YYYY[-/.]M[-/.]D or D[-/.]M[-/.]YYYY (or M[-/.]D[-/.]YYYY when month > 12)
	static const QRegularExpression reYmd("^(\\d{4})[.\\-/\\s]+(\\d{1,2})[.\\-/\\s]+(\\d{1,2})(?:\\b.*)?$");
	if (const auto m = reYmd.match(cleaned); m.hasMatch())
	{
		const QDate d(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
		if (d.isValid())
			return fmt(d);
	}

	static const QRegularExpression reDmy("^(\\d{1,2})[.\\-/]+(\\d{1,2})[.\\-/]+(\\d{2,4})(?:\\b.*)?$");
	if (const auto m = reDmy.match(cleaned); m.hasMatch())
	{
		const int a = m.captured(1).toInt();
		const int b = m.captured(2).toInt();
		int y = m.captured(3).toInt();
		if (m.captured(3).size() == 2)
			y += (y >= 70 ? 1900 : 2000);
		QDate d;
		if (a > 12)
			d = QDate(y, b, a);
		else if (b > 12)
			d = QDate(y, a, b);
		else
			d = QDate(y, b, a); // DD-MM-YYYY default
		if (d.isValid())
			return fmt(d);
	}

	// Textual English month dates (common in idgames .txt files: "May 10, 2011", "10 May 2011", "Oct 1 1996")
	const QLocale cLoc = QLocale::c();
	static const char* const kTextFormats[] = {
		"MMMM d, yyyy", "MMM d, yyyy", "MMMM d yyyy", "MMM d yyyy",
		"d MMMM yyyy", "d MMM yyyy", "d MMMM, yyyy", "d MMM, yyyy",
		"d-MMM-yyyy", "d-MMMM-yyyy", "yyyy MMM d", "yyyy MMMM d"
	};
	for (const char* pat : kTextFormats)
	{
		const QDate d = cLoc.toDate(cleaned, QLatin1String(pat));
		if (d.isValid())
			return fmt(d);
	}

	return stdStr(s);
}

std::string sanitizeFolderName(const std::string& name)
{
	QString clean = qStr(name).trimmed();
	clean.replace(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1F]"), "_");
	clean.replace(QRegularExpression("\\s+"), "_");
	while (clean.endsWith('.'))
		clean.chop(1);
	if (clean.isEmpty())
		clean = "profile";
	return stdStr(clean);
}

std::string findBestDefaultIWADPath(const std::vector<Profile>& existingProfiles)
{
	for (const auto& p : existingProfiles)
	{
		if (p.isIWAD && !p.iwadPath.empty() && QFileInfo::exists(qStr(p.iwadPath)))
			return p.iwadPath;
	}
	for (const auto& w : g_discoveredEngineIWads)
	{
		if (!w.path.empty() && QFileInfo::exists(qStr(w.path)))
			return w.path;
	}
	return {};
}

QStringList Profile::buildLaunchArguments(LaunchTargetMode mode, const std::string& launcherLanguage) const
{
	QStringList args;

	const QString normIwad = iwadPath.empty() ? QString() : normalizeSlash(qStr(iwadPath));
	if (!normIwad.isEmpty())
		args << "-iwad" << qStr(iwadPath);

	QStringList seenFiles;
	if (!normIwad.isEmpty())
		seenFiles << normIwad.toLower();

	if (!isIWAD && !pwadPath.empty())
	{
		const QString pwad = qStr(pwadPath);
		const QString key = normalizeSlash(pwad).toLower();
		if (!seenFiles.contains(key))
		{
			appendModArg(args, pwad);
			seenFiles << key;
		}
	}

	QSet<QString> disabledSet;
	for (const auto& dm : disabledMods)
	{
		if (!dm.empty())
			disabledSet.insert(normalizeSlash(qStr(dm)).toLower());
	}

	for (const auto& m : mods)
	{
		if (m.empty())
			continue;
		const QString modQ = qStr(m);
		const QString key = normalizeSlash(modQ).toLower();
		if (disabledSet.contains(key))
			continue;
		if (!seenFiles.contains(key))
		{
			appendModArg(args, modQ);
			seenFiles << key;
		}
	}

	if (!configFilePath.empty())
		args << "-config" << qStr(configFilePath);
	if (!saveDirPath.empty())
		args << "-savedir" << qStr(saveDirPath);
	if (!screenshotDirPath.empty())
		args << "-shotdir" << qStr(screenshotDirPath);

	args << "+set" << "vid_fullscreen" << (fullscreen ? "1" : "0");
	args << (loadSupportWads ? "-supportwad" : "-nosupportwad");
	args << "+set" << "i_loadsupportwad" << (loadSupportWads ? "1" : "0");
	args << "+set" << "r_dynlights" << (enableDynLights ? "1" : "0");

	if (disableAutoload)
		args << "-noautoload";
	else
		args << "+set" << "disableautoload" << "0";

	args << "+set" << "vid_preferbackend" << QString::number(renderingAPI);

	if (extraLights)
		args << "+set" << "autoloadlights" << "1";
	else
		args << "-nolights";

	if (extraBrightmaps)
		args << "+set" << "autoloadbrightmaps" << "1";
	else
		args << "-nobrightmaps";

	if (extraWidescreen)
		args << "+set" << "autoloadwidescreen" << "1";
	else
		args << "-nowidescreen";

	if (launchParameters == 1 && !selectedLaunchMap.empty())
	{
		const QString mapTrimmed = qStr(selectedLaunchMap).trimmed();
		const QStringList warpTokens = mapTrimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
		bool allDigits = !warpTokens.isEmpty();
		for (const auto& tok : warpTokens)
		{
			bool ok = false;
			tok.toInt(&ok);
			if (!ok)
			{
				allDigits = false;
				break;
			}
		}
		if (allDigits)
		{
			args << "-warp";
			args << warpTokens;
		}
		else
		{
			args << "+map" << mapTrimmed;
		}
	}
	else if (launchParameters == 2 && !selectedLaunchSave.empty())
	{
		args << "-loadgame" << resolveRelativeToDir(qStr(selectedLaunchSave), qStr(saveDirPath));
	}
	else if (launchParameters == 3 && !selectedLaunchDemoPlayback.empty())
	{
		args << "-playdemo" << resolveRelativeToDir(qStr(selectedLaunchDemoPlayback), qStr(demoDirPath));
	}

	if (recordDemo && !selectedLaunchDemoRecord.empty())
	{
		args << "-record" << resolveRelativeToDir(qStr(selectedLaunchDemoRecord), qStr(demoDirPath));
	}

	if (difficulty > 0)
		args << "-skill" << QString::number(difficulty);

	if (!complevel.empty())
		args << "-complevel" << qStr(complevel);

	if (mode == LaunchTargetMode::Host)
	{
		const int players = std::clamp(maxPlayers, 1, 64);
		args << "-host" << QString::number(players);
		if (hostPort > 0 && hostPort <= 65535)
			args << "-port" << QString::number(hostPort);
		if (hostTicRate > 0)
			args << "-dup" << QString::number(hostTicRate + 1);
		if (hostExtraTic)
			args << "-extratic";

		switch (hostGamemode)
		{
		case 1:
			args << "-coop";
			break;
		case 2:
			args << "-deathmatch";
			break;
		case 3:
			args << "-deathmatch";
			args << "+set" << "teamplay" << "1";
			args << "+set" << "team" << QString::number(std::clamp(hostTeam, 0, 255));
			break;
		case 4:
			args << "-altdeath";
			break;
		default:
			break;
		}
	}
	else if (mode == LaunchTargetMode::Join)
	{
		QString target = qStr(remoteAddress).trimmed();
		if (!target.isEmpty())
		{
			if (remotePort > 0 && remotePort <= 65535 && !target.contains(':'))
				target += ":" + QString::number(remotePort);
			args << "-join" << target;
		}
		args << "+set" << "team" << QString::number(std::clamp(remoteTeam, 0, 255));
	}

	uint32_t effDmflags = dmflags;
	uint32_t effDmflags2 = dmflags2;
	uint32_t effDmflags3 = dmflags3;
	if (fastMonsters)
		effDmflags |= DF_FAST_MONSTERS;
	if (respawnMonsters)
		effDmflags |= DF_MONSTERS_RESPAWN;
	if (noMonsters)
		effDmflags |= DF_NO_MONSTERS;
	if (allowMultiplayerItems)
		effDmflags2 |= DF2_ALWAYS_SPAWN_MULTI;
	if (pistolStartAllLevels)
		effDmflags3 |= DF3_PISTOL_START;

	if (alwaysApplyDMFlags)
	{
		args << "+set" << "alwaysapplydmflags" << "1";
		args << "+set" << "dmflags" << QString::number(effDmflags);
		args << "+set" << "dmflags2" << QString::number(effDmflags2);
		args << "+set" << "dmflags3" << QString::number(effDmflags3);
	}
	else
	{
		constexpr uint32_t cliDmMask = DF_FAST_MONSTERS | DF_MONSTERS_RESPAWN | DF_NO_MONSTERS;
		if ((effDmflags & ~cliDmMask) != 0)
		{
			args << "+set" << "dmflags" << QString::number(effDmflags);
		}
		else
		{
			if ((effDmflags & DF_FAST_MONSTERS) != 0)
				args << "-fast";
			if ((effDmflags & DF_MONSTERS_RESPAWN) != 0)
				args << "-respawn";
			if ((effDmflags & DF_NO_MONSTERS) != 0)
				args << "-nomonsters";
		}
		if (effDmflags2 != 0)
			args << "+set" << "dmflags2" << QString::number(effDmflags2);
		if (effDmflags3 != 0)
			args << "+set" << "dmflags3" << QString::number(effDmflags3);
	}

	const int matchedCompatMode = findMatchingCompatModeId(compatflags, compatflags2);
	if (matchedCompatMode > 0)
	{
		args << "-compatmode" << QString::number(matchedCompatMode);
	}
	else if (compatflags == 0 && compatflags2 == 0 && compatLevel > 0)
	{
		args << "-compatmode" << QString::number(compatLevel);
	}
	else if (compatflags != 0 || compatflags2 != 0)
	{
		args << "+set" << "compatflags" << QString::number(compatflags);
		args << "+set" << "compatflags2" << QString::number(compatflags2);
	}
	else
	{
		args << "-compatmode" << "0";
	}

	if (!playerName.empty())
		args << "+set" << "name" << qStr(playerName);
	if (!playerClass.empty())
		args << "+playerclass" << qStr(playerClass);
	if (playerGender == 1)
		args << "+set" << "gender" << "male";
	else if (playerGender == 2)
		args << "+set" << "gender" << "female";
	else if (playerGender == 3)
		args << "+set" << "gender" << "neutral";
	std::string effectiveLanguage = wadLanguage;
	if (effectiveLanguage.empty() || effectiveLanguage == "default")
		effectiveLanguage = (!launcherLanguage.empty() && launcherLanguage != "default") ? launcherLanguage : "auto";
	args << "+set" << "language" << qStr(effectiveLanguage);

	if (!appendAdditionalParameters.empty())
		args << QProcess::splitCommand(qStr(appendAdditionalParameters));

	return args;
}

std::string Profile::giveLaunchCommand(LaunchTargetMode mode, const std::string& launcherLanguage) const
{
	const QStringList args = buildLaunchArguments(mode, launcherLanguage);
	QStringList quoted;
	quoted.reserve(args.size());
	for (const auto& a : args)
		quoted << quoteShellArg(a);
	return stdStr(quoted.join(' '));
}

LauncherConfig loadLauncherConfig()
{
	LauncherConfig cfg;
	QFile f(qStr(LauncherConst::CONFIG_FILE));
	if (!f.open(QIODevice::ReadOnly))
		return cfg;

	const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	if (!doc.isObject())
		return cfg;

	const QJsonObject obj = doc.object();
	if (obj.contains("language"))
		cfg.language = stdStr(obj["language"].toString());
	if (obj.contains("theme"))
		cfg.theme = stdStr(obj["theme"].toString());
	if (obj.contains("hideWhilePlaying"))
		cfg.hideWhilePlaying = obj["hideWhilePlaying"].toBool(false);
	if (obj.contains("showReleaseNotes"))
		cfg.showReleaseNotes = obj["showReleaseNotes"].toBool(true);
	if (obj.contains("maximized"))
		cfg.maximized = obj["maximized"].toBool(false);
	if (obj.contains("lastSelectedRow"))
		cfg.lastSelectedRow = obj["lastSelectedRow"].toInt(0);
	if (obj.contains("sortColumn"))
		cfg.sortColumn = obj["sortColumn"].toInt(-1);
	if (obj.contains("sortDescending"))
		cfg.sortDescending = obj["sortDescending"].toBool(false);
	if (obj.contains("splitterTopSize"))
		cfg.splitterTopSize = obj["splitterTopSize"].toInt(0);
	if (obj.contains("splitterBottomSize"))
		cfg.splitterBottomSize = obj["splitterBottomSize"].toInt(0);
	if (obj.contains("defaultModsDir"))
		cfg.defaultModsDir = stdStr(obj["defaultModsDir"].toString());
	return cfg;
}

void saveLauncherConfig(const LauncherConfig& cfg)
{
	QDir().mkpath(qStr(LauncherConst::ROOT_DIR));
	QJsonObject obj;
	obj["language"] = qStr(cfg.language);
	obj["theme"] = qStr(cfg.theme);
	obj["hideWhilePlaying"] = cfg.hideWhilePlaying;
	obj["showReleaseNotes"] = cfg.showReleaseNotes;
	obj["maximized"] = cfg.maximized;
	obj["lastSelectedRow"] = cfg.lastSelectedRow;
	obj["sortColumn"] = cfg.sortColumn;
	obj["sortDescending"] = cfg.sortDescending;
	obj["splitterTopSize"] = cfg.splitterTopSize;
	obj["splitterBottomSize"] = cfg.splitterBottomSize;
	obj["defaultModsDir"] = qStr(cfg.defaultModsDir);

	QFile f(qStr(LauncherConst::CONFIG_FILE));
	if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
		f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void createDefaultDirectoryStructure(Profile& profile)
{
	QDir().mkpath(qStr(LauncherConst::PROFILE_DIR));

	const std::string baseSlug = sanitizeFolderName(profile.title);
	std::string folderName = baseSlug;
	int suffix = 1;
	QString folderPath;
	if (!profile.filePath.empty())
	{
		folderPath = QFileInfo(qStr(profile.filePath)).absolutePath();
	}
	else
	{
		while (true)
		{
			folderPath = QDir(qStr(LauncherConst::PROFILE_DIR)).filePath(qStr(folderName));
			if (!QFileInfo::exists(folderPath))
				break;
			folderName = baseSlug + "_" + std::to_string(++suffix);
		}
	}

	QDir dir(folderPath);
	dir.mkpath(".");
	dir.mkpath("saves");
	dir.mkpath("screenshots");
	dir.mkpath("demos");
	dir.mkpath("mods");

	if (profile.configFilePath.empty())
		profile.configFilePath = stdStr(dir.filePath("uzdoom.ini"));
	if (profile.saveDirPath.empty())
		profile.saveDirPath = stdStr(dir.filePath("saves"));
	if (profile.screenshotDirPath.empty())
		profile.screenshotDirPath = stdStr(dir.filePath("screenshots"));
	if (profile.demoDirPath.empty())
		profile.demoDirPath = stdStr(dir.filePath("demos"));
	if (profile.filePath.empty())
		profile.filePath = stdStr(dir.filePath("profile.Uzdoomprofile"));
}

bool saveProfileToDisk(Profile& profile)
{
	if (profile.filePath.empty())
		createDefaultDirectoryStructure(profile);

	QFileInfo fi(qStr(profile.filePath));
	QDir().mkpath(fi.absolutePath());

	QJsonObject obj;
	obj["title"] = qStr(profile.title);
	obj["author"] = qStr(profile.author);
	obj["releaseDate"] = qStr(profile.releaseDate);
	obj["description"] = qStr(profile.description);
	obj["lastPlayed"] = qStr(profile.lastPlayed);
	obj["playtime"] = static_cast<qint64>(profile.playtime);
	obj["orderIndex"] = profile.orderIndex;
	obj["favorite"] = profile.favorite;
	obj["coverImagePath"] = qStr(profile.coverImagePath);

	obj["isIWAD"] = profile.isIWAD;
	obj["iwadPath"] = qStr(profile.iwadPath);
	obj["pwadPath"] = qStr(profile.pwadPath);
	QJsonArray modsArr;
	for (const auto& m : profile.mods)
		modsArr.append(qStr(m));
	obj["mods"] = modsArr;
	QJsonArray disabledModsArr;
	for (const auto& dm : profile.disabledMods)
		disabledModsArr.append(qStr(dm));
	obj["disabledMods"] = disabledModsArr;
	obj["copyModsToProfile"] = profile.copyModsToProfile;

	obj["launchParameters"] = profile.launchParameters;
	obj["selectedLaunchMap"] = qStr(profile.selectedLaunchMap);
	obj["selectedLaunchSave"] = qStr(profile.selectedLaunchSave);
	obj["selectedLaunchDemoPlayback"] = qStr(profile.selectedLaunchDemoPlayback);
	obj["recordDemo"] = profile.recordDemo;
	obj["selectedLaunchDemoRecord"] = qStr(profile.selectedLaunchDemoRecord);

	obj["difficulty"] = profile.difficulty;
	obj["fastMonsters"] = profile.fastMonsters;
	obj["respawnMonsters"] = profile.respawnMonsters;
	obj["noMonsters"] = profile.noMonsters;
	obj["allowMultiplayerItems"] = profile.allowMultiplayerItems;
	obj["pistolStartAllLevels"] = profile.pistolStartAllLevels;

	obj["playerName"] = qStr(profile.playerName);
	obj["playerClass"] = qStr(profile.playerClass);
	obj["playerGender"] = profile.playerGender;
	obj["wadLanguage"] = qStr(profile.wadLanguage);

	obj["compatLevel"] = profile.compatLevel;
	obj["complevel"] = qStr(profile.complevel);
	obj["alwaysApplyDMFlags"] = profile.alwaysApplyDMFlags;
	obj["dmflags"] = static_cast<qint64>(profile.dmflags);
	obj["dmflags2"] = static_cast<qint64>(profile.dmflags2);
	obj["dmflags3"] = static_cast<qint64>(profile.dmflags3);
	obj["compatflags"] = static_cast<qint64>(profile.compatflags);
	obj["compatflags2"] = static_cast<qint64>(profile.compatflags2);

	obj["remoteAddress"] = qStr(profile.remoteAddress);
	obj["remotePort"] = profile.remotePort;
	obj["remoteTeam"] = profile.remoteTeam;

	obj["hostPort"] = profile.hostPort;
	obj["maxPlayers"] = profile.maxPlayers;
	obj["hostTicRate"] = profile.hostTicRate;
	obj["hostExtraTic"] = profile.hostExtraTic;
	obj["hostGamemode"] = profile.hostGamemode;
	obj["hostTeam"] = profile.hostTeam;

	obj["configFilePath"] = qStr(profile.configFilePath);
	obj["saveDirPath"] = qStr(profile.saveDirPath);
	obj["screenshotDirPath"] = qStr(profile.screenshotDirPath);
	obj["demoDirPath"] = qStr(profile.demoDirPath);

	obj["fullscreen"] = profile.fullscreen;
	obj["loadSupportWads"] = profile.loadSupportWads;
	obj["disableAutoload"] = profile.disableAutoload;
	obj["enableDynLights"] = profile.enableDynLights;
	obj["renderingAPI"] = profile.renderingAPI;
	obj["extraLights"] = profile.extraLights;
	obj["extraBrightmaps"] = profile.extraBrightmaps;
	obj["extraWidescreen"] = profile.extraWidescreen;

	obj["appendAdditionalParameters"] = qStr(profile.appendAdditionalParameters);

	QFile out(qStr(profile.filePath));
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return false;
	return out.write(QJsonDocument(obj).toJson(QJsonDocument::Indented)) >= 0;
}

bool loadProfileFromDisk(const std::string& path, Profile& p)
{
	QFile f(qStr(path));
	if (!f.open(QIODevice::ReadOnly))
		return false;

	const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	if (!doc.isObject())
		return false;
	const QJsonObject obj = doc.object();

	p = Profile{};
	p.filePath = path;

	p.title = stdStr(obj["title"].toString("New Profile"));
	p.author = stdStr(obj["author"].toString("Unknown"));
	p.releaseDate = normalizeReleaseDate(stdStr(obj["releaseDate"].toString("Unknown")));
	p.description = stdStr(obj["description"].toString());
	p.lastPlayed = stdStr(obj["lastPlayed"].toString("Never"));
	p.playtime = static_cast<uint64_t>(std::max<qint64>(0, obj["playtime"].toInteger(0)));
	p.orderIndex = obj["orderIndex"].toInt(-1);
	p.favorite = obj["favorite"].toBool(false);
	p.coverImagePath = stdStr(obj["coverImagePath"].toString());

	p.isIWAD = obj["isIWAD"].toBool(false);
	p.iwadPath = stdStr(obj["iwadPath"].toString());
	p.pwadPath = stdStr(obj["pwadPath"].toString());
	const QJsonArray modsArr = obj["mods"].toArray();
	for (const auto& v : modsArr)
	{
		if (v.isString() && !v.toString().isEmpty())
			p.mods.push_back(stdStr(v.toString()));
	}
	const QJsonArray disabledModsArr = obj["disabledMods"].toArray();
	for (const auto& v : disabledModsArr)
	{
		if (v.isString() && !v.toString().isEmpty())
			p.disabledMods.push_back(stdStr(v.toString()));
	}
	p.copyModsToProfile = obj["copyModsToProfile"].toBool(true);

	p.launchParameters = obj["launchParameters"].toInt(0);
	if (obj["selectedLaunchMap"].isDouble())
	{
		const int legacyMap = obj["selectedLaunchMap"].toInt(0);
		if (legacyMap > 0)
			p.selectedLaunchMap = std::to_string(legacyMap);
	}
	else
	{
		p.selectedLaunchMap = stdStr(obj["selectedLaunchMap"].toString());
	}
	p.selectedLaunchSave = stdStr(obj["selectedLaunchSave"].toString());
	p.selectedLaunchDemoPlayback = stdStr(obj["selectedLaunchDemoPlayback"].toString());
	p.recordDemo = obj["recordDemo"].toBool(false);
	p.selectedLaunchDemoRecord = stdStr(obj["selectedLaunchDemoRecord"].toString());

	p.difficulty = obj["difficulty"].toInt(0);
	p.fastMonsters = obj["fastMonsters"].toBool(false);
	p.respawnMonsters = obj["respawnMonsters"].toBool(false);
	p.noMonsters = obj["noMonsters"].toBool(false);
	p.allowMultiplayerItems = obj["allowMultiplayerItems"].toBool(false);
	p.pistolStartAllLevels = obj["pistolStartAllLevels"].toBool(false);

	p.playerName = stdStr(obj["playerName"].toString());
	p.playerClass = stdStr(obj["playerClass"].toString());
	p.playerGender = obj["playerGender"].toInt(0);
	p.wadLanguage = stdStr(obj["wadLanguage"].toString("default"));

	p.compatLevel = obj["compatLevel"].toInt(0);
	p.complevel = stdStr(obj["complevel"].toString().trimmed().toLower());
	p.alwaysApplyDMFlags = obj["alwaysApplyDMFlags"].toBool(false);
	p.dmflags = static_cast<uint32_t>(obj["dmflags"].toInteger(0));
	p.dmflags2 = static_cast<uint32_t>(obj["dmflags2"].toInteger(0));
	p.dmflags3 = static_cast<uint32_t>(obj["dmflags3"].toInteger(0));
	p.compatflags = static_cast<uint32_t>(obj["compatflags"].toInteger(0));
	p.compatflags2 = static_cast<uint32_t>(obj["compatflags2"].toInteger(0));

	p.remoteAddress = stdStr(obj["remoteAddress"].toString("127.0.0.1"));
	p.remotePort = obj["remotePort"].toInt(5029);
	p.remoteTeam = obj["remoteTeam"].toInt(255);

	p.hostPort = obj["hostPort"].toInt(5029);
	p.maxPlayers = obj["maxPlayers"].toInt(8);
	p.hostTicRate = obj["hostTicRate"].toInt(0);
	p.hostExtraTic = obj["hostExtraTic"].toBool(false);
	p.hostGamemode = obj["hostGamemode"].toInt(0);
	p.hostTeam = obj["hostTeam"].toInt(255);

	p.configFilePath = stdStr(obj["configFilePath"].toString());
	p.saveDirPath = stdStr(obj["saveDirPath"].toString());
	p.screenshotDirPath = stdStr(obj["screenshotDirPath"].toString());
	p.demoDirPath = stdStr(obj["demoDirPath"].toString());

	p.fullscreen = obj["fullscreen"].toBool(true);
	p.loadSupportWads = obj["loadSupportWads"].toBool(true);
	p.disableAutoload = obj["disableAutoload"].toBool(false);
	p.enableDynLights = obj["enableDynLights"].toBool(true);
	p.renderingAPI = obj["renderingAPI"].toInt(1);
	p.extraLights = obj["extraLights"].toBool(false);
	p.extraBrightmaps = obj["extraBrightmaps"].toBool(false);
	p.extraWidescreen = obj["extraWidescreen"].toBool(false);

	p.appendAdditionalParameters = stdStr(obj["appendAdditionalParameters"].toString());
	return true;
}

std::vector<Profile> loadAllProfiles()
{
	std::vector<Profile> profiles;
	const QString profRoot = qStr(LauncherConst::PROFILE_DIR);
	if (!QFileInfo::exists(profRoot))
		return profiles;

	QDir rootDir(profRoot);
	const QFileInfoList subDirs = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const QFileInfo& sub : subDirs)
	{
		QDir subDir(sub.absoluteFilePath());
		QStringList candidates = subDir.entryList({"*.Uzdoomprofile", "*.uzdoomprofile", "*.json"}, QDir::Files);
		if (candidates.isEmpty())
			continue;

		const QString metaFile = subDir.filePath(candidates.first());
		Profile p;
		if (loadProfileFromDisk(stdStr(metaFile), p))
		{
			const QString covPath = qStr(p.coverImagePath);
			bool needsExtract = covPath.isEmpty() || !QFileInfo::exists(covPath);
			if (!needsExtract && QFileInfo(covPath).fileName().compare("cover.png", Qt::CaseInsensitive) == 0)
			{
				const QImage existingImg(covPath);
				if (existingImg.width() == 320 && existingImg.height() == 240)
				{
					p.coverImagePath.clear();
					needsExtract = true;
				}
				else if (!existingImg.isNull() && existingImg.allGray())
				{
					// Re-extract covers that previously fell back to a grayscale ramp palette.
					p.coverImagePath.clear();
					needsExtract = true;
				}
			}
			if (needsExtract && tryExtractCoverForProfile(p))
				saveProfileToDisk(p);
			profiles.push_back(std::move(p));
		}
	}

	std::sort(profiles.begin(), profiles.end(), [](const Profile& a, const Profile& b)
	{
		if (a.orderIndex >= 0 && b.orderIndex >= 0)
		{
			if (a.orderIndex != b.orderIndex)
				return a.orderIndex < b.orderIndex;
		}
		else if (a.orderIndex >= 0)
		{
			return true;
		}
		else if (b.orderIndex >= 0)
		{
			return false;
		}
		if (a.title != b.title)
			return a.title < b.title;
		return a.filePath < b.filePath;
	});

	return profiles;
}

void persistProfileOrdering(std::vector<Profile>& profiles)
{
	for (size_t i = 0; i < profiles.size(); ++i)
	{
		const int desiredIndex = static_cast<int>(i);
		if (profiles[i].orderIndex != desiredIndex || profiles[i].filePath.empty())
		{
			profiles[i].orderIndex = desiredIndex;
			saveProfileToDisk(profiles[i]);
		}
	}
}

void applyProfilePathMapping(Profile& profile, const std::string& oldBaseDir, const std::string& newBaseDir)
{
	const QString oldQ = qStr(oldBaseDir);
	const QString newQ = qStr(newBaseDir);

	auto remapField = [&](std::string& field)
	{
		if (!field.empty())
			field = stdStr(remapSinglePath(qStr(field), oldQ, newQ));
	};

	remapField(profile.iwadPath);
	remapField(profile.pwadPath);
	remapField(profile.coverImagePath);
	remapField(profile.configFilePath);
	remapField(profile.saveDirPath);
	remapField(profile.screenshotDirPath);
	remapField(profile.demoDirPath);
	remapField(profile.selectedLaunchSave);
	remapField(profile.selectedLaunchDemoPlayback);

	for (auto& mod : profile.mods)
		remapField(mod);
	for (auto& dmod : profile.disabledMods)
		remapField(dmod);
}

bool cloneProfile(const Profile& source, const std::string& newTitle, Profile& outClone)
{
	if (source.filePath.empty())
		return false;

	const QString srcFolder = QFileInfo(qStr(source.filePath)).absolutePath();
	outClone = source;
	outClone.title = newTitle.empty() ? (source.title + " (Copy)") : newTitle;
	outClone.filePath.clear();
	outClone.orderIndex = -1;
	outClone.lastPlayed = "Never";
	outClone.playtime = 0;

	const std::string slug = sanitizeFolderName(outClone.title);
	std::string folderName = slug;
	int suffix = 1;
	QString destFolder;
	while (true)
	{
		destFolder = QDir(qStr(LauncherConst::PROFILE_DIR)).filePath(qStr(folderName));
		if (!QFileInfo::exists(destFolder))
			break;
		folderName = slug + "_" + std::to_string(++suffix);
	}

	if (!copyDirectoryRecursively(srcFolder, destFolder, true))
		return false;

	QDir destDir(destFolder);
	for (const QString& oldMeta : destDir.entryList({"*.Uzdoomprofile", "*.uzdoomprofile", "*.json"}, QDir::Files))
		destDir.remove(oldMeta);

	outClone.filePath = stdStr(destDir.filePath("profile.Uzdoomprofile"));
	applyProfilePathMapping(outClone, stdStr(srcFolder), stdStr(destFolder));
	return saveProfileToDisk(outClone);
}

bool deleteProfileDirectory(const Profile& profile)
{
	if (profile.filePath.empty())
		return false;

	const QString folder = QFileInfo(qStr(profile.filePath)).absolutePath();
	const QString normFolder = normalizeSlash(folder);
	const QString normRoot = normalizeSlash(qStr(LauncherConst::PROFILE_DIR));

	if (!normFolder.startsWith(normRoot + "/", Qt::CaseInsensitive) || normFolder == normRoot)
		return QFile::remove(qStr(profile.filePath));

	return QDir(folder).removeRecursively();
}

namespace
{
	size_t qfileZipReadCb(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n)
	{
		auto* f = static_cast<QFile*>(pOpaque);
		if (!f)
			return 0;
		const qint64 ofs = static_cast<qint64>(file_ofs);
		if (f->pos() != ofs && !f->seek(ofs))
			return 0;
		const qint64 r = f->read(static_cast<char*>(pBuf), static_cast<qint64>(n));
		return r > 0 ? static_cast<size_t>(r) : 0;
	}

	size_t qfileZipWriteCb(void* pOpaque, mz_uint64 file_ofs, const void* pBuf, size_t n)
	{
		auto* f = static_cast<QFile*>(pOpaque);
		if (!f)
			return 0;
		const qint64 ofs = static_cast<qint64>(file_ofs);
		if (f->pos() != ofs && !f->seek(ofs))
			return 0;
		const qint64 w = f->write(static_cast<const char*>(pBuf), static_cast<qint64>(n));
		return w > 0 ? static_cast<size_t>(w) : 0;
	}

	size_t qfileStreamWriteCb(void* pOpaque, mz_uint64 /*file_ofs*/, const void* pBuf, size_t n)
	{
		auto* f = static_cast<QFile*>(pOpaque);
		if (!f)
			return 0;
		const qint64 w = f->write(static_cast<const char*>(pBuf), static_cast<qint64>(n));
		return w > 0 ? static_cast<size_t>(w) : 0;
	}
}

bool extractZipArchiveToDir(const QString& zipPath, const QString& destDir, std::vector<QString>* extractedFiles, std::string* errOut)
{
	QFile inFile(zipPath);
	if (!inFile.open(QIODevice::ReadOnly))
	{
		if (errOut)
			*errOut = "Failed to open archive.";
		return false;
	}

	mz_zip_archive zip{};
	zip.m_pRead = qfileZipReadCb;
	zip.m_pIO_opaque = &inFile;
	if (!mz_zip_reader_init(&zip, static_cast<mz_uint64>(inFile.size()), 0))
	{
		if (errOut)
			*errOut = "Failed to read archive central directory.";
		return false;
	}

	const mz_uint numFiles = mz_zip_reader_get_num_files(&zip);
	if (numFiles == 0)
	{
		mz_zip_reader_end(&zip);
		if (errOut)
			*errOut = "Archive is empty.";
		return false;
	}

	const QString cleanDest = QDir::cleanPath(destDir);
	QDir().mkpath(cleanDest);
	for (mz_uint i = 0; i < numFiles; ++i)
	{
		mz_zip_archive_file_stat st{};
		if (!mz_zip_reader_file_stat(&zip, i, &st))
			continue;

		QString entryName = QString::fromUtf8(st.m_filename).replace('\\', '/');
		while (entryName.startsWith('/'))
			entryName.remove(0, 1);
		if (entryName.isEmpty() || entryName.contains(".."))
			continue;

		const QString outPath = QDir::cleanPath(QDir(cleanDest).filePath(entryName));
		if (!outPath.startsWith(cleanDest + "/"))
			continue;

		if (mz_zip_reader_is_file_a_directory(&zip, i))
		{
			QDir().mkpath(outPath);
			continue;
		}

		QDir().mkpath(QFileInfo(outPath).absolutePath());
		QFile outFile(outPath);
		if (!outFile.open(QIODevice::WriteOnly)
			|| !mz_zip_reader_extract_to_callback(&zip, i, qfileStreamWriteCb, &outFile, 0))
		{
			mz_zip_reader_end(&zip);
			if (errOut)
				*errOut = "Failed to extract archive entry: " + stdStr(entryName);
			return false;
		}
		if (extractedFiles)
			extractedFiles->push_back(outPath);
	}

	mz_zip_reader_end(&zip);
	return true;
}

bool extractMatchingEntryFromZip(const QString& zipPath, const std::function<bool(const QString& entryName)>& predicate, const QString& destFolder, const QString& targetBaseName, QString& outExtractedPath)
{
	QFile inFile(zipPath);
	if (!inFile.open(QIODevice::ReadOnly))
		return false;

	mz_zip_archive zip{};
	zip.m_pRead = qfileZipReadCb;
	zip.m_pIO_opaque = &inFile;
	if (!mz_zip_reader_init(&zip, static_cast<mz_uint64>(inFile.size()), 0))
		return false;

	bool extracted = false;
	const mz_uint count = mz_zip_reader_get_num_files(&zip);
	for (mz_uint i = 0; i < count; ++i)
	{
		mz_zip_archive_file_stat st{};
		if (!mz_zip_reader_file_stat(&zip, i, &st) || mz_zip_reader_is_file_a_directory(&zip, i))
			continue;

		const QString name = QString::fromUtf8(st.m_filename);
		if (!predicate(name))
			continue;

		const QString ext = QFileInfo(name).suffix().toLower();
		const QString outPath = QDir(destFolder).filePath(targetBaseName + "." + ext);
		QDir().mkpath(destFolder);
		QFile outFile(outPath);
		if (outFile.open(QIODevice::WriteOnly)
			&& mz_zip_reader_extract_to_callback(&zip, i, qfileStreamWriteCb, &outFile, 0))
		{
			outExtractedPath = outPath;
			extracted = true;
			break;
		}
	}

	mz_zip_reader_end(&zip);
	return extracted;
}

bool exportProfileToZip(const Profile& profile, const std::string& destZipPath, std::string& errOut)
{
	if (profile.filePath.empty())
	{
		errOut = "Profile has no directory on disk.";
		return false;
	}

	QFile destFile(qStr(destZipPath));
	if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		errOut = "Could not create export archive.";
		return false;
	}

	const QString profFolder = QFileInfo(qStr(profile.filePath)).absolutePath();
	mz_zip_archive zip{};
	zip.m_pWrite = qfileZipWriteCb;
	zip.m_pIO_opaque = &destFile;
	if (!mz_zip_writer_init(&zip, 0))
	{
		errOut = "Could not initialize export archive.";
		return false;
	}

	const QString destAbs = normalizeSlash(QFileInfo(qStr(destZipPath)).absoluteFilePath());
	const QDir baseDir(profFolder);
	QDirIterator it(profFolder, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	bool ok = true;
	while (it.hasNext())
	{
		const QString absFile = it.next();
		if (normalizeSlash(QFileInfo(absFile).absoluteFilePath()).compare(destAbs, Qt::CaseInsensitive) == 0)
			continue;
		QString relPath = baseDir.relativeFilePath(absFile).replace('\\', '/');
		if (relPath.compare(QStringLiteral("last_launch.log"), Qt::CaseInsensitive) == 0
			|| relPath.startsWith(QStringLiteral("logs/"), Qt::CaseInsensitive))
			continue;
		QFile srcFile(absFile);
		if (!srcFile.open(QIODevice::ReadOnly)
			|| !mz_zip_writer_add_read_buf_callback(
				&zip,
				relPath.toUtf8().constData(),
				qfileZipReadCb,
				&srcFile,
				static_cast<mz_uint64>(srcFile.size()),
				nullptr,
				"",
				0,
				MZ_DEFAULT_COMPRESSION,
				nullptr,
				0,
				nullptr,
				0))
		{
			ok = false;
			errOut = "Failed writing file to export archive: " + stdStr(relPath);
			break;
		}
	}

	if (!mz_zip_writer_finalize_archive(&zip))
	{
		ok = false;
		if (errOut.empty())
			errOut = "Failed to finalize archive.";
	}
	mz_zip_writer_end(&zip);
	return ok;
}

bool importProfileFromZip(const std::string& zipPath, Profile& outProfile, std::string& errOut)
{
	const QString baseName = QFileInfo(qStr(zipPath)).completeBaseName();
	std::string folderName = sanitizeFolderName(stdStr(baseName));
	int suffix = 1;
	QString destFolder;
	while (true)
	{
		destFolder = QDir(qStr(LauncherConst::PROFILE_DIR)).filePath(qStr(folderName));
		if (!QFileInfo::exists(destFolder))
			break;
		folderName = sanitizeFolderName(stdStr(baseName)) + "_" + std::to_string(++suffix);
	}

	std::vector<QString> extractedFiles;
	if (!extractZipArchiveToDir(qStr(zipPath), destFolder, &extractedFiles, &errOut))
	{
		QDir(destFolder).removeRecursively();
		return false;
	}

	const QString cleanDestRoot = normalizeSlash(QFileInfo(destFolder).absoluteFilePath());
	QString foundRootProfile;
	QString foundNestedProfile;
	QString foundRootJson;
	for (const QString& outPath : extractedFiles)
	{
		const QString lower = outPath.toLower();
		const QString parentDir = normalizeSlash(QFileInfo(outPath).absolutePath());
		const bool isAtRoot = (parentDir.compare(cleanDestRoot, Qt::CaseInsensitive) == 0);
		if (lower.endsWith(".uzdoomprofile"))
		{
			if (isAtRoot && foundRootProfile.isEmpty())
				foundRootProfile = outPath;
			else if (foundNestedProfile.isEmpty())
				foundNestedProfile = outPath;
		}
		else if (isAtRoot && lower.endsWith(".json") && foundRootJson.isEmpty())
		{
			foundRootJson = outPath;
		}
	}
	const QString foundMeta = !foundRootProfile.isEmpty()
		? foundRootProfile
		: (!foundNestedProfile.isEmpty() ? foundNestedProfile : foundRootJson);

	if (foundMeta.isEmpty())
	{
		QDir(destFolder).removeRecursively();
		errOut = "No profile metadata found inside .uzdp archive.";
		return false;
	}

	if (!loadProfileFromDisk(stdStr(foundMeta), outProfile))
	{
		QDir(destFolder).removeRecursively();
		errOut = "Profile metadata inside archive is invalid.";
		return false;
	}

	QString finalBaseDir = QFileInfo(destFolder).absoluteFilePath();
	const QString metaParent = QFileInfo(foundMeta).absolutePath();
	const QString normMetaParent = normalizeSlash(metaParent);
	if (normMetaParent.compare(cleanDestRoot, Qt::CaseInsensitive) != 0)
	{
		// Hoist nested profile directory to the immediate child folder under PROFILE_DIR so loadAllProfiles() finds it on restart.
		QString tempHoist = destFolder + QStringLiteral("_hoist");
		int hoistSuffix = 1;
		while (QFileInfo::exists(tempHoist))
			tempHoist = destFolder + QStringLiteral("_hoist_") + QString::number(++hoistSuffix);

		bool movedOut = QDir().rename(metaParent, tempHoist);
		if (!movedOut)
			movedOut = copyDirectoryRecursively(metaParent, tempHoist, false);

		if (!movedOut)
		{
			QDir(tempHoist).removeRecursively();
			QDir(destFolder).removeRecursively();
			errOut = "Failed to flatten nested profile directory inside .uzdp archive.";
			return false;
		}

		QDir(destFolder).removeRecursively();
		if (QDir().rename(tempHoist, destFolder))
		{
			finalBaseDir = QFileInfo(destFolder).absoluteFilePath();
		}
		else
		{
			finalBaseDir = QFileInfo(tempHoist).absoluteFilePath();
		}
		outProfile.filePath = stdStr(QDir(finalBaseDir).filePath(QFileInfo(foundMeta).fileName()));
	}

	auto inferOldProfileDir = [&]() -> QString
	{
		auto parentIfNamed = [](const std::string& pathStr, const QString& expectedLeaf) -> QString
		{
			if (pathStr.empty())
				return {};
			const QString norm = normalizeSlash(qStr(pathStr));
			const QFileInfo fi(norm);
			if (!expectedLeaf.isEmpty() && fi.fileName().compare(expectedLeaf, Qt::CaseInsensitive) != 0)
				return {};
			return fi.absolutePath();
		};
		if (const QString d = parentIfNamed(outProfile.configFilePath, QString()); !d.isEmpty())
			return d;
		if (const QString d = parentIfNamed(outProfile.saveDirPath, QStringLiteral("saves")); !d.isEmpty())
			return d;
		if (const QString d = parentIfNamed(outProfile.screenshotDirPath, QStringLiteral("screenshots")); !d.isEmpty())
			return d;
		if (const QString d = parentIfNamed(outProfile.demoDirPath, QStringLiteral("demos")); !d.isEmpty())
			return d;
		if (const QString d = parentIfNamed(outProfile.coverImagePath, QString()); !d.isEmpty())
			return d;
		return {};
	};
	const QString oldDir = inferOldProfileDir();
	applyProfilePathMapping(outProfile, stdStr(oldDir), stdStr(finalBaseDir));
	outProfile.orderIndex = -1;
	if (!saveProfileToDisk(outProfile))
	{
		QDir(finalBaseDir).removeRecursively();
		errOut = "Failed to save imported profile.";
		return false;
	}
	return true;
}
