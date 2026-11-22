/*
** updater.cpp
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

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN

#include "updater.h"

#include "basics.h"
#include "c_console.h"
#include "c_cvars.h"
#include "cmdlib.h"
#include "const.h"
#include "curl_loader_internal.h"
#include "gstrings.h"
#include "i_interface.h"
#include "i_net.h"
#include "m_misc.h"
#include "resourcefile.h"
#include "serializer_rapidjson.h"
#include "version.h"
#include "versioninfo.h"
#include "zstring.h"

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>
#include <QTextBrowser>
#include <QThread>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cassert>
#include <charconv>
#include <climits>
#include <cstddef>
#include <ctime>
#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

#ifdef _WIN32
	#include <windows.h>
	#include <shellapi.h>
	#include <process.h>
#endif

#ifdef _WIN32
	#define RELEASE_JSON_PLATFORM_NAME "windows"
#else
	#error "Updater not implemented for this platform"
#endif

void CloseWidgetResources();
void M_SaveDefaultsFinal();

CVAR(String, updater_cached_update, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(String, updater_skipped_update, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(String, updater_last_update_check, "", CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Bool, updater_check_updates_initialized, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Int, updater_update_interval, 7, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG); // by default, check once per week
CVAR(Bool, updater_auto_updates, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG);
CVAR(Bool, updater_check_updates, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG);
CVAR(Bool, updater_debug_always_update, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);
CVAR(Bool, updater_debug_throttle_download, false, CVAR_ARCHIVE | CVAR_CONFIG_ONLY | CVAR_GLOBALCONFIG | CVAR_NOSET | CVAR_HIDDEN);

namespace
{
	uint64_t daysToSeconds(uint64_t days)
	{
		return days * 86400ULL;
	}

	uint64_t getCurrentDate()
	{
		time_t t;
		time(&t);
		return (static_cast<uint64_t>(t) / 86400ULL) * 86400ULL; // round to whole day
	}

	uint64_t parseDate(const FString& str)
	{
		return str.ToULong();
	}

	bool isVersionInvalid(const VersionInfo& ver)
	{
		return ver.major == USHRT_MAX || ver.minor == USHRT_MAX || ver.revision == USHRT_MAX || ver == GetCurrentVersionForUpdater();
	}

	QWidget* makeUpdaterIconHeader(QDialog* dlg, QStyle::StandardPixmap iconType, const QString& text, bool bold = false)
	{
		QWidget* row = new QWidget(dlg);
		QHBoxLayout* h = new QHBoxLayout(row);
		h->setContentsMargins(0, 0, 0, 0);
		h->setSpacing(12);

		QLabel* iconLbl = new QLabel(row);
		const int iconSize = dlg->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, dlg);
		iconLbl->setPixmap(dlg->style()->standardIcon(iconType, nullptr, dlg).pixmap(iconSize, iconSize));
		iconLbl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		iconLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		QLabel* textLbl = new QLabel(text, row);
		textLbl->setWordWrap(true);
		textLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
		if (bold)
		{
			QFont f = textLbl->font();
			f.setBold(true);
			textLbl->setFont(f);
		}

		h->addWidget(iconLbl, 0, Qt::AlignTop);
		h->addWidget(textLbl, 1, Qt::AlignVCenter);
		return row;
	}

	QString shortenByteSize(uint64_t size)
	{
		static const char* const units[4] = {"B", "KB", "MB", "GB"};
		constexpr int maxStep = 3;

		if (size < 1024)
			return QString::fromStdString(std::format("{} {}", size, units[0]));

		double val = static_cast<double>(size);
		int steps = 0;
		while (val >= 1024.0 && steps < maxStep)
		{
			val /= 1024.0;
			++steps;
		}
		if (val >= 100.0)
			return QString::fromStdString(std::format("{:.1f} {}", val, units[steps]));
		return QString::fromStdString(std::format("{:.2f} {}", val, units[steps]));
	}

	QString formatLocalizedWithArgs(const char* key, const char* fallback, const std::string& arg1)
	{
		const std::string fmt = stdStr(LStr(key, fallback));
		try
		{
			return qStr(std::vformat(fmt, std::make_format_args(arg1)));
		}
		catch (...)
		{
			return qStr(fmt);
		}
	}

	QString formatLocalizedWithArgs(const char* key, const char* fallback, const std::string& arg1, const std::string& arg2)
	{
		const std::string fmt = stdStr(LStr(key, fallback));
		try
		{
			return qStr(std::vformat(fmt, std::make_format_args(arg1, arg2)));
		}
		catch (...)
		{
			return qStr(fmt);
		}
	}

	size_t callAcceptData(void* buf, size_t sz, size_t num, void* self);
	int callUpdateProgress(void* self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

	class CurlEasy
	{
		friend size_t callAcceptData(void* buf, size_t sz, size_t num, void* self);
		friend int callUpdateProgress(void* self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

		CURL* curl = nullptr;
		const std::atomic<bool>* externalCancel = nullptr;
		std::atomic<bool> cancelled{false};

	protected:
		virtual int onProgress(curl_off_t dltotal, curl_off_t dlnow)
		{
			(void)dltotal;
			(void)dlnow;
			return isCancelled() ? 1 : 0;
		}
		virtual void acceptData(TArrayView<std::byte> data) = 0;
		virtual void onError(const char* err) { (void)err; }

		CurlEasy(const char* userAgent, bool useProgress, const std::atomic<bool>* extCancel)
			: externalCancel(extCancel)
		{
			curl = curl_easy_init();
			if (!curl)
			{
				onError("curl_easy_init failed");
				return;
			}

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callAcceptData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

			if (useProgress || externalCancel != nullptr)
			{
				curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, callUpdateProgress);
				curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			}

			curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
			curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

			if (updater_debug_throttle_download)
			{
				curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, static_cast<curl_off_t>(1000000));
			}
		}

		bool perform(const std::string& url, bool* outCancelled = nullptr)
		{
			if (!curl)
			{
				if (outCancelled)
					*outCancelled = isCancelled();
				onError("curl_easy_init failed");
				return false;
			}

			DEBUG_LOG("fetching %s", url.c_str());

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			// Empty string enables all compression encodings supported by libcurl.
			curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

			if (const CURLcode ret = curl_easy_perform(curl); ret != CURLE_OK)
			{
				if (outCancelled)
					*outCancelled = isCancelled();
				onError(curl_easy_strerror(ret));
				return false;
			}

			if (outCancelled)
				*outCancelled = isCancelled();
			return true;
		}

	public:
		virtual ~CurlEasy()
		{
			close();
		}

		bool isCancelled() const
		{
			return cancelled.load(std::memory_order_relaxed)
				|| (externalCancel && externalCancel->load(std::memory_order_relaxed));
		}

		void cancel()
		{
			cancelled.store(true, std::memory_order_relaxed);
		}

		void close()
		{
			if (curl)
			{
				curl_easy_cleanup(curl);
				curl = nullptr;
			}
		}
	};

	size_t callAcceptData(void* buf, size_t sz, size_t num, void* self)
	{
		assert(sz == 1);
		(void)sz;
		auto* easy = static_cast<CurlEasy*>(self);
		if (easy->isCancelled())
			return 0;
		easy->acceptData({static_cast<std::byte*>(buf), static_cast<unsigned>(num)});
		return num;
	}

	int callUpdateProgress(void* self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
	{
		(void)ultotal;
		(void)ulnow;
		auto* easy = static_cast<CurlEasy*>(self);
		if (easy->isCancelled())
			return 1;
		return easy->onProgress(dltotal, dlnow);
	}

	class JsonDownloader final : public CurlEasy
	{
		std::string buffer;
		std::string firstErr;

		void acceptData(TArrayView<std::byte> data) override
		{
			buffer.append(reinterpret_cast<const char*>(data.Data()), data.Size());
		}

		void onError(const char* err) override
		{
			if (firstErr.empty() && err)
				firstErr = err;
		}

	public:
		explicit JsonDownloader(const std::atomic<bool>* cancelFlag)
			: CurlEasy(GAMENAME " Updater", false, cancelFlag)
		{
		}

		std::optional<rapidjson::Document> perform(const std::string& url, bool& outTransportErr, std::string& outErrMsg, bool& outCancelled)
		{
			outTransportErr = false;
			outErrMsg.clear();
			outCancelled = false;

			if (!CurlEasy::perform(url, &outCancelled))
			{
				outTransportErr = !outCancelled;
				outErrMsg = firstErr;
				return std::nullopt;
			}
			close();

			rapidjson::Document doc;
			const rapidjson::ParseResult ok = doc.Parse(buffer.c_str(), buffer.length());
			if (!ok)
				return std::nullopt;

			return doc;
		}
	};

	class MemoryDownloader final : public CurlEasy
	{
		std::vector<std::byte> buffer;
		std::string lastErr;
		std::function<void(uint64_t, uint64_t)> progressCb;
		QElapsedTimer throttleTimer;

		void acceptData(TArrayView<std::byte> data) override
		{
			const size_t oldSize = buffer.size();
			buffer.resize(oldSize + data.Size());
			memcpy(buffer.data() + oldSize, data.Data(), data.Size());
		}

		int onProgress(curl_off_t dltotal, curl_off_t dlnow) override
		{
			if (isCancelled())
				return 1;

			if (progressCb)
			{
				const bool done = (dltotal > 0 && dlnow >= dltotal);
				if (!throttleTimer.isValid() || throttleTimer.elapsed() >= 50 || done)
				{
					throttleTimer.restart();
					progressCb(
						dlnow > 0 ? static_cast<uint64_t>(dlnow) : 0ULL,
						dltotal > 0 ? static_cast<uint64_t>(dltotal) : 0ULL);
				}
			}
			return 0;
		}

		void onError(const char* err) override
		{
			if (lastErr.empty() && err)
				lastErr = err;
		}

	public:
		MemoryDownloader(const std::atomic<bool>* cancelFlag, std::function<void(uint64_t, uint64_t)> onProgressCb)
			: CurlEasy(GAMENAME " Updater", true, cancelFlag)
			, progressCb(std::move(onProgressCb))
		{
		}

		bool download(const std::string& url, std::vector<std::byte>& outData, std::string& outErr, bool& outCancelled)
		{
			const bool ok = CurlEasy::perform(url, &outCancelled);
			close();
			if (ok)
			{
				outData = std::move(buffer);
				return true;
			}
			outErr = lastErr;
			return false;
		}
	};

	std::optional<UpdaterReleaseInfo> parseRelease(const rapidjson::Document& doc, bool& ok, bool& silentFail)
	{
		DEBUG_LOG("parsing");

#define HAS_MEMBER(source, id, type) ((source).HasMember(id) && (source)[id].Is##type())
#define FAIL_WITH_ERROR \
	do \
	{ \
		ok = false; \
		silentFail = false; \
		DEBUG_LOG("errored"); \
		return std::nullopt; \
	} while (false)

		if (!HAS_MEMBER(doc, "commit", Object)) FAIL_WITH_ERROR;
		if (!HAS_MEMBER(doc["commit"], "version", String)) FAIL_WITH_ERROR;
		if (!HAS_MEMBER(doc["commit"], "message", String)) FAIL_WITH_ERROR;

		const char* versionStr = doc["commit"]["version"].GetString();
		const char* messageStr = doc["commit"]["message"].GetString();
		const VersionInfo ver{versionStr};

		DEBUG_LOG("%s", FString(ver).GetChars());

		if (!HAS_MEMBER(doc, "platforms", Object)) FAIL_WITH_ERROR;
		if (!HAS_MEMBER(doc["platforms"], RELEASE_JSON_PLATFORM_NAME, String)) FAIL_WITH_ERROR;

		const std::string downloadName = doc["platforms"][RELEASE_JSON_PLATFORM_NAME].GetString();
		DEBUG_LOG("%s", downloadName.c_str());

		std::vector<std::string> notes;
		if (messageStr && *messageStr)
			notes.emplace_back(messageStr);
		if (HAS_MEMBER(doc, "notes", String))
		{
			const char* extra = doc["notes"].GetString();
			if (extra && *extra)
				notes.emplace_back(extra);
		}
		else if (HAS_MEMBER(doc, "body", String))
		{
			const char* extra = doc["body"].GetString();
			if (extra && *extra)
				notes.emplace_back(extra);
		}

		ok = true;
		silentFail = false;
		return UpdaterReleaseInfo{ver, false, std::move(notes), downloadName};

#undef FAIL_WITH_ERROR
#undef HAS_MEMBER
	}

	bool extractAndStageWindowsUpdate(
		const std::string& downloadUrl,
		const std::vector<std::byte>& zipBytes,
		bool& outZipOpenFailed,
		std::string& outError)
	{
		outZipOpenFailed = false;
		outError.clear();

		std::unique_ptr<FileSys::FResourceFile> zip(FileSys::FResourceFile::OpenResourceFileMemory(
			downloadUrl.c_str(),
			const_cast<std::byte*>(zipBytes.data()),
			zipBytes.size(),
			true));
		if (!zip)
		{
			outZipOpenFailed = true;
			return false;
		}

#ifdef _WIN32
		const QString progDirStr = QString::fromUtf8(::progdir.GetChars());
		const QDir rootDir(progDirStr);
		const QString updateDirPath = rootDir.filePath("update");
		const QString backupDirPath = rootDir.filePath("update_backup");

		auto prepareDir = [&](const QString& dirPath) -> bool
		{
			const QFileInfo fi(dirPath);
			if (fi.exists())
			{
				if (fi.isDir())
				{
					if (!QDir(dirPath).removeRecursively() || !QDir().mkpath(dirPath))
					{
						outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Failed to clean staging folder", stdStr(dirPath)));
						return false;
					}
				}
				else
				{
					outError = stdStr(formatLocalizedWithArgs("UPDATER_NOT_A_DIR", "'{}' is not a directory", stdStr(dirPath)));
					return false;
				}
			}
			else if (!QDir().mkpath(dirPath))
			{
				outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Failed to create staging folder", stdStr(dirPath)));
				return false;
			}
			return true;
		};

		if (!prepareDir(updateDirPath) || !prepareDir(backupDirPath))
			return false;

		bool replacedUpdaterExe = false;
		bool hadExistingUpdaterExe = false;
		const QString rootUpdaterPath = rootDir.filePath("updater.exe");
		const QString backupUpdaterPath = QDir(backupDirPath).filePath("updater.exe");

		auto cleanupStaging = [&]()
		{
			if (replacedUpdaterExe)
			{
				if (hadExistingUpdaterExe && QFileInfo::exists(backupUpdaterPath))
				{
					QFile::remove(rootUpdaterPath);
					QFile::copy(backupUpdaterPath, rootUpdaterPath);
				}
				else if (!hadExistingUpdaterExe)
				{
					QFile::remove(rootUpdaterPath);
				}
			}
			QDir(updateDirPath).removeRecursively();
			QDir(backupDirPath).removeRecursively();
		};

		const uint32_t entryCount = zip->EntryCountU();
		for (uint32_t i = 0; i < entryCount; ++i)
		{
			std::string rawPath = zip->getName(i);
			std::replace(rawPath.begin(), rawPath.end(), '\\', '/');
			if (rawPath.empty() || rawPath.back() == '/')
				continue;

			const QString relPath = QDir::cleanPath(QString::fromUtf8(rawPath.c_str()));
			if (relPath.isEmpty() || relPath == "." || relPath.startsWith("..") || relPath.startsWith('/') || relPath.contains(':'))
			{
				cleanupStaging();
				outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Unsafe archive entry path", rawPath));
				return false;
			}

			const bool isUpdaterExe = (relPath.compare("updater.exe", Qt::CaseInsensitive) == 0);
			const QString existingTarget = rootDir.filePath(relPath);
			if (QFileInfo::exists(existingTarget) && QFileInfo(existingTarget).isFile())
			{
				const QString backupDest = QDir(backupDirPath).filePath(relPath);
				if (!QFileInfo(backupDest).dir().mkpath("."))
				{
					cleanupStaging();
					outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Failed to create backup directory", stdStr(backupDest)));
					return false;
				}
				if (QFileInfo::exists(backupDest))
					QFile::remove(backupDest);
				if (!QFile::copy(existingTarget, backupDest))
				{
					cleanupStaging();
					outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Failed to back up existing file", stdStr(backupDest)));
					return false;
				}
				if (isUpdaterExe)
					hadExistingUpdaterExe = true;
			}

			const QString newPath = isUpdaterExe
				? rootDir.filePath(relPath)
				: QDir(updateDirPath).filePath(relPath);

			if (!isUpdaterExe && !QFileInfo(newPath).dir().mkpath("."))
			{
				cleanupStaging();
				outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", "Failed to create directory", stdStr(newPath)));
				return false;
			}

			QFile outFile(newPath);
			if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				const std::string errStr = stdStr(outFile.errorString());
				cleanupStaging();
				outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", errStr, stdStr(newPath)));
				return false;
			}
			if (isUpdaterExe)
				replacedUpdaterExe = true;

			const FileSys::FileData data = zip->Read(i);
			const qint64 totalBytes = static_cast<qint64>(data.size());
			qint64 written = 0;
			while (written < totalBytes)
			{
				const qint64 chunk = outFile.write(reinterpret_cast<const char*>(data.data()) + written, totalBytes - written);
				if (chunk <= 0)
				{
					const std::string errStr = stdStr(outFile.errorString());
					outFile.close();
					cleanupStaging();
					outError = stdStr(formatLocalizedWithArgs("UPDATER_ZIP_ERROR", "Failed to extract zip, error: '{}' while writing file '{}'", errStr, stdStr(newPath)));
					return false;
				}
				written += chunk;
			}
			outFile.close();
		}

		return true;
#else
		return false;
#endif
	}

#ifdef _WIN32
	std::wstring quoteWindowsCommandLineArg(const std::wstring& arg)
	{
		if (!arg.empty() && arg.find_first_of(L" \t\n\v\"") == std::wstring::npos)
			return arg;

		std::wstring out;
		out.push_back(L'"');
		for (size_t i = 0; i < arg.size();)
		{
			size_t backslashes = 0;
			while (i < arg.size() && arg[i] == L'\\')
			{
				++backslashes;
				++i;
			}
			if (i == arg.size())
			{
				out.append(backslashes * 2, L'\\');
			}
			else if (arg[i] == L'"')
			{
				out.append(backslashes * 2 + 1, L'\\');
				out.push_back(L'"');
				++i;
			}
			else
			{
				out.append(backslashes, L'\\');
				out.push_back(arg[i]);
				++i;
			}
		}
		out.push_back(L'"');
		return out;
	}
#endif

	bool launchWindowsUpdaterAndRestart(std::string& outError)
	{
#ifdef _WIN32
		const QString progDirStr = QString::fromUtf8(::progdir.GetChars());
		const QString updaterPathQt = QDir::toNativeSeparators(QDir(progDirStr).filePath("updater.exe"));
		if (!QFileInfo::exists(updaterPathQt))
		{
			outError = "Could not find updater.exe in application directory.";
			return false;
		}

		const std::wstring updaterFilenameW = updaterPathQt.toStdWString();
		std::vector<std::wstring> quotedArgs;
		quotedArgs.push_back(quoteWindowsCommandLineArg(updaterFilenameW));

		int argc = 0;
		if (LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc))
		{
			for (int i = 1; i < argc; ++i)
				quotedArgs.push_back(quoteWindowsCommandLineArg(argv[i]));
			LocalFree(argv);
		}

		std::vector<const wchar_t*> execArgv;
		execArgv.reserve(quotedArgs.size() + 1);
		for (const std::wstring& arg : quotedArgs)
			execArgv.push_back(arg.c_str());
		execArgv.push_back(nullptr);

		updater_cached_update = "";
		updater_last_update_check = std::to_string(getCurrentDate()).c_str();
		M_SaveDefaults(nullptr);
		CloseWidgetResources();

		_wexecv(updaterFilenameW.c_str(), execArgv.data());

		outError = "Failed to launch updater.exe.";
		return false;
#else
		outError = "Updater not implemented for this platform.";
		return false;
#endif
	}
}

LauncherUpdater::LauncherUpdater(QWidget* parent, std::function<void()> settingsChangedCallback)
	: parentWindow(parent)
	, onSettingsChanged(std::move(settingsChangedCallback))
	, destroyed(std::make_shared<std::atomic<bool>>(false))
{
	banner = new QFrame(parent);
	banner->setObjectName("launcherUpdateBanner");
	banner->setFixedHeight(32);
	banner->setStyleSheet(
		"#launcherUpdateBanner { background-color: #0b5394; border: 1px solid #073c6d; border-radius: 4px; }"
		"#launcherUpdateBanner QToolButton { background-color: transparent; border: none; border-right: 1px solid #073c6d; border-top-left-radius: 3px; border-bottom-left-radius: 3px; }"
		"#launcherUpdateBanner QToolButton:hover { background-color: #d94336; }"
		"#launcherUpdateBanner QToolButton:pressed { background-color: #a82d24; }"
		"#launcherUpdateBanner QPushButton { background-color: transparent; color: #ffffff; border: none; font-weight: bold; text-align: center; padding: 0 12px; border-top-right-radius: 3px; border-bottom-right-radius: 3px; }"
		"#launcherUpdateBanner QPushButton:hover { background-color: #1668b2; }"
		"#launcherUpdateBanner QPushButton:pressed { background-color: #073c6d; }"
	);

	auto* layout = new QHBoxLayout(banner);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	btnDismiss = new QToolButton(banner);
	btnDismiss->setFixedSize(32, 30);
	btnDismiss->setIcon(makeLauncherIcon("delete", Qt::white, 13));
	btnDismiss->setCursor(Qt::PointingHandCursor);
	QObject::connect(btnDismiss, &QToolButton::clicked, banner, [this]() {
		onDismissBannerClicked();
	});

	btnOpenUpdate = new QPushButton(banner);
	btnOpenUpdate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	btnOpenUpdate->setCursor(Qt::PointingHandCursor);
	QObject::connect(btnOpenUpdate, &QPushButton::clicked, banner, [this]() {
		onBannerClicked();
	});

	layout->addWidget(btnDismiss);
	layout->addWidget(btnOpenUpdate, 1);

	banner->hide();
	retranslateUi();
}

LauncherUpdater::~LauncherUpdater()
{
	if (destroyed)
		destroyed->store(true, std::memory_order_relaxed);
	cancelPending();
}

QWidget* LauncherUpdater::bannerWidget() const
{
	return banner;
}

void LauncherUpdater::setBannerEnabled(bool enabled)
{
	if (banner)
		banner->setEnabled(enabled);
}

void LauncherUpdater::retranslateUi()
{
	if (btnDismiss)
		btnDismiss->setToolTip(LStr("UPDATER_DISMISS_UPDATE", "Dismiss Update?"));

	if (btnOpenUpdate)
	{
		if (currentUpdate.has_value())
		{
			const QString verText = QString::fromUtf8(FString(currentUpdate->version).GetChars());
			btnOpenUpdate->setText(QString("%1: %2  \u2192").arg(LStr("UPDATER_UPDATE_AVAILABLE", "New Update Available")).arg(verText));
		}
		else
		{
			btnOpenUpdate->setText(QString());
		}
	}
}

void LauncherUpdater::showBanner()
{
	retranslateUi();
	if (banner)
		banner->show();
}

void LauncherUpdater::hideBanner()
{
	if (banner)
		banner->hide();
}

QWidget* LauncherUpdater::resolveDialogParent(QWidget* dialogParent) const
{
	return dialogParent ? dialogParent : parentWindow.data();
}

void LauncherUpdater::notifySettingsChanged()
{
	if (onSettingsChanged)
		onSettingsChanged();
}

UpdaterMode LauncherUpdater::currentMode() const
{
	if (updater_check_updates && updater_auto_updates)
		return UpdaterMode::PromptToInstall;
	if (updater_check_updates)
		return UpdaterMode::Notify;
	return UpdaterMode::Disabled;
}

int LauncherUpdater::currentIntervalDays() const
{
	const int interval = updater_update_interval;
	if (interval < 7)
		return 1;
	if (interval > 7)
		return 30;
	return 7;
}

void LauncherUpdater::setMode(UpdaterMode mode)
{
	switch (mode)
	{
	case UpdaterMode::PromptToInstall:
		updater_auto_updates = true;
		updater_check_updates = true;
		break;
	case UpdaterMode::Notify:
		updater_auto_updates = false;
		updater_check_updates = true;
		break;
	case UpdaterMode::Disabled:
	default:
		updater_auto_updates = false;
		updater_check_updates = false;
		break;
	}
	updater_check_updates_initialized = true;
	M_SaveDefaults(nullptr);
	notifySettingsChanged();
}

void LauncherUpdater::setIntervalDays(int days)
{
	updater_update_interval = days;
	updater_check_updates_initialized = true;
	M_SaveDefaults(nullptr);
	notifySettingsChanged();
}

void LauncherUpdater::cancelPending()
{
	if (checkCancel)
		checkCancel->store(true, std::memory_order_relaxed);
	if (checkThread)
	{
		QThread* t = checkThread.data();
		checkThread = nullptr;
		t->disconnect();
		t->wait();
		delete t;
	}
}

bool LauncherUpdater::initCurl(std::string& errorOut)
{
	static bool s_curlInitialized = false;
	static bool s_curlOk = false;

	if (s_curlInitialized)
		return s_curlOk;

	if (!IsNetworkStartedLean())
		StartNetworkLean();

	s_curlInitialized = true;
	const CURLcode ret = curl_global_init(CURL_GLOBAL_DEFAULT);
	s_curlOk = (ret == CURLE_OK);
	if (!s_curlOk)
		errorOut = std::string("curl_global_init failed: ") + curl_easy_strerror(ret);
	return s_curlOk;
}

LauncherUpdater::FetchOutcome LauncherUpdater::fetchUpdateInfoSync(const std::atomic<bool>* cancelFlag)
{
	FetchOutcome outcome;
	DEBUG_LOG("starting");

	bool primary = false;
	std::string stream = "latest";
	std::string lastTransportErr;
	bool wasCancelled = false;

	auto buildUrl = [&stream](bool usePrimary, const std::string& asset)
	{
		return usePrimary
			? std::format(UPDATER_URL, stream, asset)
			: (stream == "latest")
				? std::format(UPDATER_URL_BACKUP, stream, "download", asset)
				: std::format(UPDATER_URL_BACKUP, "download", stream, asset);
	};

	auto tryGetData = [&]() -> std::optional<rapidjson::Document>
	{
		DEBUG_LOG("Trying '%s'", stream.c_str());

		bool transportErr1 = false;
		std::string errMsg1;
		bool cancelled1 = false;
		auto doc = JsonDownloader(cancelFlag).perform(buildUrl(true, "_release.json"), transportErr1, errMsg1, cancelled1);
		if (cancelled1)
		{
			wasCancelled = true;
			return std::nullopt;
		}
		primary = doc.has_value();
		if (primary)
			return doc;

		bool transportErr2 = false;
		std::string errMsg2;
		bool cancelled2 = false;
		doc = JsonDownloader(cancelFlag).perform(buildUrl(false, "_release.json"), transportErr2, errMsg2, cancelled2);
		if (cancelled2)
		{
			wasCancelled = true;
			return std::nullopt;
		}
		if (!doc.has_value() && transportErr2)
			lastTransportErr = errMsg2;
		else if (!doc.has_value() && transportErr1 && !transportErr2)
			lastTransportErr = errMsg1;

		return doc;
	};

	auto toNum = [](unsigned& v, const std::string& str)
	{
		auto [p, e] = std::from_chars(str.data(), str.data() + str.size(), v);
		return (e == std::errc{} && p == str.data() + str.size());
	};

	auto getReleaseData = [&]() -> std::optional<rapidjson::Document>
	{
		std::optional<rapidjson::Document> doc;
		const std::string current = GetVersionString();
		const std::string tag = GetGitTag();

		if (!current.starts_with(tag))
		{
			stream = tag;
			DEBUG_LOG("Preview build");
			return tryGetData();
		}

		VersionInfo temp = GetCurrentVersionForUpdater();
		const std::string pre = std::string(temp.prerelease);
		temp.prerelease[0] = temp.build[0] = '\0';
		if (pre.empty())
		{
			stream = "latest";
			DEBUG_LOG("Stable build");
			return tryGetData();
		}

		unsigned v = 0;
		const auto base = std::string(temp);
		std::vector<std::string> parts;
		for (auto part : pre | std::views::split('.'))
			parts.emplace_back(part.begin(), part.end());

		if (!parts.empty() && !toNum(v, parts.back()))
			parts.emplace_back("0");

		std::vector<std::string> candidates;
		candidates.emplace_back("latest");
		while (!parts.empty())
		{
			const std::string end = parts.back();
			parts.pop_back();
			std::string release = base + "-";
			for (size_t i = 0; i < parts.size(); ++i)
				release += parts[i] + ".";
			if (toNum(v, end))
			{
				release += std::to_string(v + 1);
				candidates.emplace_back(release);
			}
		}
		candidates.emplace_back(base);

		for (int i = static_cast<int>(candidates.size()) - 1; i >= 0; --i)
		{
			if (cancelFlag && cancelFlag->load(std::memory_order_relaxed))
			{
				wasCancelled = true;
				return std::nullopt;
			}
			stream = candidates[i];
			if ((doc = tryGetData()); doc.has_value() || wasCancelled)
				return doc;
		}
		return doc;
	};

	auto doc = getReleaseData();
	if (wasCancelled)
	{
		outcome.ok = false;
		outcome.cancelled = true;
		return outcome;
	}

	DEBUG_LOG("Using '%s'", stream.c_str());

	if (!doc.has_value())
	{
		if (!lastTransportErr.empty())
		{
			outcome.ok = false;
			outcome.silentFail = false;
			outcome.errorMessage = lastTransportErr;
			return outcome;
		}
		DEBUG_LOG("empty response");
		outcome.ok = true;
		outcome.info = std::nullopt;
		return outcome;
	}

	bool parsedOk = false;
	bool silentFail = false;
	std::optional<UpdaterReleaseInfo> parsed = parseRelease(*doc, parsedOk, silentFail);
	if (parsedOk && parsed.has_value())
	{
		parsed->downloadUrl = buildUrl(primary, parsed->downloadUrl);
		outcome.ok = true;
		outcome.info = std::move(parsed);
		return outcome;
	}

	outcome.ok = false;
	outcome.silentFail = silentFail;
	if (!silentFail)
		outcome.errorMessage = stdStr(LStr("UPDATER_INVALID_JSON", "Invalid Update JSON"));
	return outcome;
}

LauncherUpdater::FetchOutcome LauncherUpdater::fetchUpdateInfoInteractive(QWidget* dialogParent)
{
	QWidget* parent = resolveDialogParent(dialogParent);
	auto cancelFlag = std::make_shared<std::atomic<bool>>(false);
	FetchOutcome outcome;

	QDialog dlg(parent);
	dlg.setWindowTitle(LStr("UPDATER_TITLE", "Update Checker"));
	dlg.setModal(true);
	dlg.setMinimumWidth(360);

	auto* layout = new QVBoxLayout(&dlg);
	auto* lbl = new QLabel(LStr("UPDATER_CHECK_FOR_UPDATES", "Check for Updates") + "...", &dlg);
	lbl->setAlignment(Qt::AlignCenter);
	auto* bar = new QProgressBar(&dlg);
	bar->setRange(0, 0);
	bar->setTextVisible(false);

	auto* btnRow = new QHBoxLayout();
	btnRow->addStretch();
	auto* btnCancel = new QPushButton(LStr("OPTVAL_CANCEL", "Cancel"), &dlg);
	btnRow->addWidget(btnCancel);

	layout->addWidget(lbl);
	layout->addWidget(bar);
	layout->addLayout(btnRow);

	QObject::connect(btnCancel, &QPushButton::clicked, &dlg, [&]() {
		cancelFlag->store(true, std::memory_order_relaxed);
		btnCancel->setEnabled(false);
	});
	QObject::connect(&dlg, &QDialog::rejected, &dlg, [&]() {
		cancelFlag->store(true, std::memory_order_relaxed);
	});

	std::unique_ptr<QThread> thread(QThread::create([cancelFlag, &outcome, &dlg]() {
		outcome = fetchUpdateInfoSync(cancelFlag.get());
		QMetaObject::invokeMethod(&dlg, [&dlg]() {
			dlg.accept();
		}, Qt::QueuedConnection);
	}));

	thread->start();
	dlg.exec();
	cancelFlag->store(true, std::memory_order_relaxed);
	thread->wait();

	return outcome;
}

void LauncherUpdater::startBackgroundCheck(const VersionInfo& cachedVer, bool wasCached, const VersionInfo& skippedVer, uint64_t curTime)
{
	if (!parentWindow)
		return;
	if (checkThread)
	{
		if (checkThread->isRunning())
			return;
		checkThread->wait();
		checkThread = nullptr;
	}

	checkCancel = std::make_shared<std::atomic<bool>>(false);
	auto cancelFlag = checkCancel;
	auto destroyedFlag = destroyed;
	QWidget* targetWindow = parentWindow.data();

	QThread* thread = QThread::create([this, targetWindow, cancelFlag, destroyedFlag, cachedVer, wasCached, skippedVer, curTime]() {
		FetchOutcome outcome = fetchUpdateInfoSync(cancelFlag.get());

		if (destroyedFlag->load(std::memory_order_relaxed) || cancelFlag->load(std::memory_order_relaxed))
			return;

		QMetaObject::invokeMethod(targetWindow, [this, destroyedFlag, cancelFlag, outcome = std::move(outcome), wasCached, cachedVer, skippedVer, curTime]() {
			if (destroyedFlag->load(std::memory_order_relaxed) || cancelFlag->load(std::memory_order_relaxed))
				return;
			handleCheckOutcome(outcome, false, wasCached, cachedVer, skippedVer, curTime, nullptr);
		}, Qt::QueuedConnection);
	});

	checkThread = thread;
	QPointer<QThread> threadGuard(thread);
	QObject::connect(thread, &QThread::finished, targetWindow, [this, destroyedFlag, threadGuard]() {
		if (!destroyedFlag->load(std::memory_order_relaxed) && checkThread == threadGuard)
			checkThread = nullptr;
	});
	QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

void LauncherUpdater::runOnboardingFlow(QWidget* dialogParent)
{
	QWidget* parent = resolveDialogParent(dialogParent);

	enum class Step
	{
		AskAuto,
		AskInterval,
		Done
	};

	Step step = Step::AskAuto;
	bool pendingAutoUpdates = false;

	while (step != Step::Done)
	{
		if (step == Step::AskAuto)
		{
			QDialog dlg(parent);
			dlg.setWindowTitle(LStr("UPDATER_TITLE", "Update Checker"));
			dlg.setModal(true);
			dlg.setMinimumWidth(380);

			auto* layout = new QVBoxLayout(&dlg);
			layout->setSpacing(12);
			layout->addWidget(makeUpdaterIconHeader(&dlg, QStyle::SP_MessageBoxQuestion,
				LStr("UPDATER_ASK_AUTO", "Would you like to automatically check for updates?")));

			auto* btnRow = new QHBoxLayout();
			btnRow->addStretch();
			auto* btnYes = new QPushButton(LStr("TXT_YES", "Yes"), &dlg);
			auto* btnYesPrompt = new QPushButton(LStr("UPDATER_YES_PROMPT", "Yes, and prompt to install updates"), &dlg);
			auto* btnNo = new QPushButton(LStr("TXT_NO", "No"), &dlg);
			btnRow->addWidget(btnYes);
			btnRow->addWidget(btnYesPrompt);
			btnRow->addWidget(btnNo);
			layout->addLayout(btnRow);

			int choice = -1;
			QObject::connect(btnYes, &QPushButton::clicked, &dlg, [&]() { choice = 0; dlg.accept(); });
			QObject::connect(btnYesPrompt, &QPushButton::clicked, &dlg, [&]() { choice = 1; dlg.accept(); });
			QObject::connect(btnNo, &QPushButton::clicked, &dlg, [&]() { choice = 2; dlg.accept(); });

			dlg.exec();
			if (choice < 0)
				return;

			if (choice == 2)
			{
				updater_auto_updates = false;
				updater_check_updates = false;
				updater_check_updates_initialized = true;
				M_SaveDefaults(nullptr);
				notifySettingsChanged();
				step = Step::Done;
			}
			else
			{
				pendingAutoUpdates = (choice == 1);
				step = Step::AskInterval;
			}
		}
		else if (step == Step::AskInterval)
		{
			QDialog dlg(parent);
			dlg.setWindowTitle(LStr("UPDATER_TITLE", "Update Checker"));
			dlg.setModal(true);
			dlg.setMinimumWidth(380);

			auto* layout = new QVBoxLayout(&dlg);
			layout->setSpacing(12);
			layout->addWidget(makeUpdaterIconHeader(&dlg, QStyle::SP_MessageBoxQuestion,
				LStr("UPDATER_ASK_INTERVAL", "How often would you like to check for updates?")));

			auto* btnRow = new QHBoxLayout();
			auto* btnDaily = new QPushButton(LStr("OPTVAL_DAILY", "Daily"), &dlg);
			auto* btnWeekly = new QPushButton(LStr("OPTVAL_WEEKLY", "Weekly"), &dlg);
			auto* btnMonthly = new QPushButton(LStr("OPTVAL_MONTHLY", "Monthly"), &dlg);
			auto* btnBack = new QPushButton(LStr("TXT_BACK", "Back"), &dlg);
			btnRow->addWidget(btnDaily);
			btnRow->addWidget(btnWeekly);
			btnRow->addWidget(btnMonthly);
			btnRow->addStretch();
			btnRow->addWidget(btnBack);
			layout->addLayout(btnRow);

			int choice = -1;
			QObject::connect(btnDaily, &QPushButton::clicked, &dlg, [&]() { choice = 0; dlg.accept(); });
			QObject::connect(btnWeekly, &QPushButton::clicked, &dlg, [&]() { choice = 1; dlg.accept(); });
			QObject::connect(btnMonthly, &QPushButton::clicked, &dlg, [&]() { choice = 2; dlg.accept(); });
			QObject::connect(btnBack, &QPushButton::clicked, &dlg, [&]() { choice = 3; dlg.accept(); });

			dlg.exec();
			if (choice < 0)
				return;

			if (choice == 3)
			{
				step = Step::AskAuto;
			}
			else
			{
				updater_auto_updates = pendingAutoUpdates;
				updater_check_updates = true;
				updater_check_updates_initialized = true;

				const uint64_t now = getCurrentDate();
				if (choice == 0)
				{
					updater_update_interval = 1;
					updater_last_update_check = std::to_string(now).c_str();
				}
				else if (choice == 1)
				{
					updater_update_interval = 7;
					updater_last_update_check = std::to_string(now - daysToSeconds(5)).c_str(); // first check in 2 days
				}
				else
				{
					updater_update_interval = 30;
					updater_last_update_check = std::to_string(now - daysToSeconds(28)).c_str(); // first check in 2 days
				}

				M_SaveDefaults(nullptr);
				notifySettingsChanged();
				step = Step::Done;
			}
		}
	}
}

void LauncherUpdater::openFailedUpdateMenu(const std::string& err, bool checker, QWidget* dialogParent)
{
	QWidget* parent = resolveDialogParent(dialogParent);
	const char* titleKey = checker ? "UPDATER_CHECK_FAILED" : "UPDATER_UPDATE_FAILED";
	const char* titleFallback = checker ? "Checking for Update Failed" : "Update Failed";
	const QString title = LStr(titleKey, titleFallback);

	QDialog dlg(parent);
	dlg.setWindowTitle(title);
	dlg.setModal(true);
	dlg.setMinimumWidth(380);

	auto* layout = new QVBoxLayout(&dlg);
	layout->setSpacing(12);
	const QString detailMsg = err.empty() ? title : QString("%1\n\n%2").arg(title, qStr(err));
	layout->addWidget(makeUpdaterIconHeader(&dlg, QStyle::SP_MessageBoxWarning, detailMsg));

	auto* btnRow = new QHBoxLayout();
	auto* btnDismiss = new QPushButton(LStr("TXT_DISMISS", "Dismiss"), &dlg);
	auto* btnDisable = new QPushButton(LStr("UPDATER_DISABLE", "Disable Update Checker"), &dlg);
	btnRow->addWidget(btnDismiss);
	btnRow->addStretch();
	btnRow->addWidget(btnDisable);
	layout->addLayout(btnRow);

	int action = 0;
	QObject::connect(btnDismiss, &QPushButton::clicked, &dlg, [&]() { action = 0; dlg.accept(); });
	QObject::connect(btnDisable, &QPushButton::clicked, &dlg, [&]() { action = 1; dlg.accept(); });

	dlg.exec();

	if (action == 1)
	{
		updater_check_updates = false;
		M_SaveDefaults(nullptr);
		notifySettingsChanged();
	}
	hideBanner();
}

void LauncherUpdater::openReleaseNotesDialog(QWidget* dialogParent) const
{
	if (!currentUpdate.has_value())
		return;

	QDialog dlg(dialogParent);
	dlg.setWindowTitle(LStr("PICKER_TAB_RELEASE", "Release Notes"));
	dlg.setModal(true);
	dlg.resize(560, 400);

	auto* layout = new QVBoxLayout(&dlg);
	auto* view = new QTextBrowser(&dlg);
	view->setOpenExternalLinks(true);

	const QString versionStr = QString::fromUtf8(FString(currentUpdate->version).GetChars());
	QString notesText;
	for (size_t i = 0; i < currentUpdate->releaseNotes.size(); ++i)
	{
		if (i > 0)
			notesText += "\n\n";
		notesText += qStr(currentUpdate->releaseNotes[i]).trimmed();
	}

	QString md = QString("# %1 %2\n\n").arg(GAMENAME).arg(versionStr);
	if (!notesText.isEmpty())
		md += notesText + "\n\n";
	else
		md += LStr("NOTES_EMPTY", "No description provided.") + "\n\n";

	const QString releaseUrl = QString("https://github.com/UZDoom/UZDoom/releases/tag/%1").arg(versionStr);
	md += "---\n\n";
	md += QString("**GitHub Release Page:** [%1](%1)\n\n").arg(releaseUrl);
	if (currentUpdate->releaseNotes.size() <= 1 && notesText.length() < 120)
		md += "*Note: The update manifest only carries the short release commit summary; open the GitHub link above for the full changelog.*";

	view->setMarkdown(md);
	layout->addWidget(view, 1);

	auto* btnRow = new QHBoxLayout();
	btnRow->addStretch();
	auto* btnBack = new QPushButton(LStr("TXT_BACK", "Back"), &dlg);
	QObject::connect(btnBack, &QPushButton::clicked, &dlg, &QDialog::accept);
	btnRow->addWidget(btnBack);
	layout->addLayout(btnRow);

	dlg.exec();
}

bool LauncherUpdater::ensureFullUpdateInfo(QWidget* dialogParent)
{
	if (!currentUpdate.has_value())
		return false;
	if (!currentUpdate->cached)
		return true;

	std::string curlErr;
	if (!initCurl(curlErr))
	{
		openFailedUpdateMenu(curlErr, true, dialogParent);
		return false;
	}

	const FetchOutcome outcome = fetchUpdateInfoInteractive(dialogParent);
	if (outcome.cancelled)
		return false;

	if (!outcome.ok)
	{
		if (!outcome.silentFail && !outcome.errorMessage.empty())
			openFailedUpdateMenu(outcome.errorMessage, true, dialogParent);
		return false;
	}

	if (!outcome.info.has_value())
	{
		updater_cached_update = "";
		M_SaveDefaults(nullptr);
		currentUpdate.reset();
		hideBanner();
		QMessageBox::information(
			resolveDialogParent(dialogParent),
			LStr("UPDATER_UP_TO_DATE", "Up to Date!"),
			LStr("UPDATER_UP_TO_DATE", "Up to Date!"));
		return false;
	}

	currentUpdate = outcome.info;
	updater_cached_update = FString(currentUpdate->version);
	updater_last_update_check = std::to_string(getCurrentDate()).c_str();
	M_SaveDefaults(nullptr);
	retranslateUi();
	return true;
}

void LauncherUpdater::openUpdateMenu(bool isAutoUpdate, QWidget* dialogParent)
{
	if (!ensureFullUpdateInfo(dialogParent) || !currentUpdate.has_value())
		return;

	QWidget* parent = resolveDialogParent(dialogParent);
	QDialog dlg(parent);
	dlg.setWindowTitle(isAutoUpdate
		? LStr("UPDATER_UPDATE_AVAILABLE", "New Update Available")
		: LStr("TXT_UPDATE", "Update"));
	dlg.setModal(true);
	dlg.setMinimumWidth(360);

	auto* layout = new QVBoxLayout(&dlg);
	layout->setSpacing(12);
	const QString versionLabel = QString("%1 %2 %3")
		.arg(GAMENAME)
		.arg(QString::fromUtf8(FString(currentUpdate->version).GetChars()))
		.arg("is available.");
	layout->addWidget(makeUpdaterIconHeader(&dlg, QStyle::SP_MessageBoxInformation, versionLabel, true));

	auto* btnRow = new QHBoxLayout();
	auto* btnNotes = new QPushButton(LStr("PICKER_SHOWNOTES", "Show Release Notes"), &dlg);
	auto* btnUpdate = new QPushButton(LStr("TXT_UPDATE", "Update"), &dlg);
	btnUpdate->setProperty("launcherRole", "primary");
	if (!isAutoUpdate)
		btnRow->addStretch();
	btnRow->addWidget(btnNotes);
	btnRow->addWidget(btnUpdate);

	enum class Choice
	{
		None,
		Update,
		Skip,
		Dismiss
	};
	Choice choice = Choice::None;

	QObject::connect(btnNotes, &QPushButton::clicked, &dlg, [&]() {
		openReleaseNotesDialog(&dlg);
	});
	QObject::connect(btnUpdate, &QPushButton::clicked, &dlg, [&]() {
		choice = Choice::Update;
		dlg.accept();
	});

	if (isAutoUpdate)
	{
		auto* btnDismiss = new QPushButton(LStr("TXT_DISMISS", "Dismiss"), &dlg);
		auto* btnSkip = new QPushButton(LStr("TXT_SKIP", "Skip"), &dlg);
		btnRow->addWidget(btnDismiss);
		btnRow->addStretch();
		btnRow->addWidget(btnSkip);

		QObject::connect(btnDismiss, &QPushButton::clicked, &dlg, [&]() {
			choice = Choice::Dismiss;
			dlg.accept();
		});
		QObject::connect(btnSkip, &QPushButton::clicked, &dlg, [&]() {
			choice = Choice::Skip;
			dlg.accept();
		});
	}

	layout->addLayout(btnRow);
	dlg.exec();

	if (choice == Choice::Update)
	{
		startUpdate(dialogParent);
	}
	else if (choice == Choice::Skip)
	{
		updater_skipped_update = FString(currentUpdate->version);
		M_SaveDefaults(nullptr);
		hideBanner();
	}
	else if (choice == Choice::Dismiss || (choice == Choice::None && isAutoUpdate))
	{
		showBanner();
	}
}

void LauncherUpdater::onBannerClicked()
{
	openUpdateMenu(false, parentWindow.data());
}

void LauncherUpdater::onDismissBannerClicked()
{
	if (!currentUpdate.has_value())
	{
		hideBanner();
		return;
	}

	QDialog dlg(parentWindow.data());
	dlg.setWindowTitle(LStr("UPDATER_DISMISS_UPDATE", "Dismiss Update?"));
	dlg.setModal(true);
	dlg.setMinimumWidth(360);

	auto* layout = new QVBoxLayout(&dlg);
	layout->setSpacing(12);
	layout->addWidget(makeUpdaterIconHeader(&dlg, QStyle::SP_MessageBoxQuestion,
		LStr("UPDATER_DISMISS_UPDATE", "Dismiss Update?")));
	auto* btnRow = new QHBoxLayout();
	btnRow->addStretch();
	auto* btnDismiss = new QPushButton(LStr("TXT_DISMISS", "Dismiss"), &dlg);
	auto* btnSkip = new QPushButton(LStr("UPDATER_UPDATE_SKIP", "Skip Update"), &dlg);
	auto* btnDisable = new QPushButton(LStr("UPDATER_DISABLE", "Disable Update Checker"), &dlg);
	btnRow->addWidget(btnDismiss);
	btnRow->addWidget(btnSkip);
	btnRow->addWidget(btnDisable);
	layout->addLayout(btnRow);

	int action = -1;
	QObject::connect(btnDismiss, &QPushButton::clicked, &dlg, [&]() { action = 0; dlg.accept(); });
	QObject::connect(btnSkip, &QPushButton::clicked, &dlg, [&]() { action = 1; dlg.accept(); });
	QObject::connect(btnDisable, &QPushButton::clicked, &dlg, [&]() { action = 2; dlg.accept(); });

	dlg.exec();
	if (action < 0)
		return;

	if (action == 1)
	{
		updater_skipped_update = FString(currentUpdate->version);
		M_SaveDefaults(nullptr);
	}
	else if (action == 2)
	{
		updater_check_updates = false;
		M_SaveDefaults(nullptr);
		notifySettingsChanged();
	}
	hideBanner();
}

void LauncherUpdater::startUpdate(QWidget* dialogParent)
{
	if (!currentUpdate.has_value() || currentUpdate->downloadUrl.empty())
		return;

	std::string curlErr;
	if (!initCurl(curlErr))
	{
		openFailedUpdateMenu(curlErr, false, dialogParent);
		return;
	}

	QWidget* parent = resolveDialogParent(dialogParent);
	const std::string downloadUrl = currentUpdate->downloadUrl;

	QDialog dlg(parent);
	dlg.setWindowTitle(LStr("UPDATER_UPDATING", "Updating..."));
	dlg.setModal(true);
	dlg.setMinimumWidth(460);

	auto* layout = new QVBoxLayout(&dlg);
	auto* lblStatus = new QLabel(LStr("UPDATER_STARTING", "Starting Download..."), &dlg);
	lblStatus->setAlignment(Qt::AlignCenter);
	auto* progressBar = new QProgressBar(&dlg);
	progressBar->setRange(0, 0);

	auto* btnRow = new QHBoxLayout();
	btnRow->addStretch();
	auto* btnCancel = new QPushButton(LStr("OPTVAL_CANCEL", "Cancel"), &dlg);
	btnRow->addWidget(btnCancel);

	layout->addWidget(lblStatus);
	layout->addWidget(progressBar);
	layout->addLayout(btnRow);

	auto cancelFlag = std::make_shared<std::atomic<bool>>(false);
	bool downloadOk = false;
	bool wasCancelled = false;
	std::string downloadErr;
	std::vector<std::byte> downloadedData;

	QObject::connect(btnCancel, &QPushButton::clicked, &dlg, [&]() {
		cancelFlag->store(true, std::memory_order_relaxed);
		btnCancel->setEnabled(false);
	});
	QObject::connect(&dlg, &QDialog::rejected, &dlg, [&]() {
		cancelFlag->store(true, std::memory_order_relaxed);
	});

	std::unique_ptr<QThread> thread(QThread::create([&]() {
		MemoryDownloader dl(cancelFlag.get(), [&](uint64_t dlNow, uint64_t dlTotal) {
			QMetaObject::invokeMethod(&dlg, [lblStatus, progressBar, dlNow, dlTotal]() {
				if (dlTotal > 0)
				{
					if (progressBar->maximum() != 1000)
						progressBar->setRange(0, 1000);
					const int val = std::clamp(static_cast<int>((static_cast<double>(dlNow) / static_cast<double>(dlTotal)) * 1000.0), 0, 1000);
					progressBar->setValue(val);
					lblStatus->setText(QString("%1, %2 / %3")
						.arg(LStr("TXT_DOWNLOADING", "Downloading"))
						.arg(shortenByteSize(dlNow))
						.arg(shortenByteSize(dlTotal)));
				}
				else
				{
					lblStatus->setText(QString("%1, %2")
						.arg(LStr("TXT_DOWNLOADING", "Downloading"))
						.arg(shortenByteSize(dlNow)));
				}
			}, Qt::QueuedConnection);
		});

		downloadOk = dl.download(downloadUrl, downloadedData, downloadErr, wasCancelled);
		QMetaObject::invokeMethod(&dlg, [&dlg]() {
			dlg.accept();
		}, Qt::QueuedConnection);
	}));

	thread->start();
	dlg.exec();
	cancelFlag->store(true, std::memory_order_relaxed);
	thread->wait();

	if (wasCancelled)
	{
		QMessageBox::information(
			parent,
			LStr("TXT_CANCELLED", "Cancelled"),
			LStr("UPDATER_CANCELLED", "Update was cancelled"));
		return;
	}

	if (!downloadOk)
	{
		openFailedUpdateMenu(downloadErr, false, dialogParent);
		return;
	}

	bool zipOpenFailed = false;
	std::string stageErr;
	if (!extractAndStageWindowsUpdate(downloadUrl, downloadedData, zipOpenFailed, stageErr))
	{
		if (zipOpenFailed)
		{
			QMessageBox::warning(
				parent,
				LStr("UPDATER_ZIP_FAIL", "Failed to open zip file"),
				LStr("UPDATER_CANCELLED", "Update was cancelled"));
		}
		else
		{
			openFailedUpdateMenu(stageErr, false, dialogParent);
		}
		return;
	}

	QDialog doneDlg(parent);
	doneDlg.setWindowTitle(LStr("TXT_UPDATED", "Updated"));
	doneDlg.setModal(true);
	doneDlg.setMinimumWidth(360);

	auto* doneLayout = new QVBoxLayout(&doneDlg);
	doneLayout->setSpacing(12);
	doneLayout->addWidget(makeUpdaterIconHeader(&doneDlg, QStyle::SP_MessageBoxInformation,
		LStr("UPDATER_SUCCESS", "Update was successful, the launcher will now restart.")));

	auto* doneBtnRow = new QHBoxLayout();
	doneBtnRow->addStretch();
	auto* btnConfirm = new QPushButton(LStr("TXT_CONFIRM", "Confirm"), &doneDlg);
	btnConfirm->setProperty("launcherRole", "primary");
	doneBtnRow->addWidget(btnConfirm);
	doneBtnRow->addStretch();
	doneLayout->addLayout(doneBtnRow);

	QObject::connect(btnConfirm, &QPushButton::clicked, &doneDlg, &QDialog::accept);
	doneDlg.exec();

	std::string launchErr;
	if (!launchWindowsUpdaterAndRestart(launchErr))
	{
		openFailedUpdateMenu(launchErr, false, dialogParent);
	}
}

void LauncherUpdater::handleCheckOutcome(
	const FetchOutcome& outcome,
	bool force,
	bool wasCached,
	const VersionInfo& cachedVer,
	const VersionInfo& skippedVer,
	uint64_t curTime,
	QWidget* dialogParent)
{
	if (outcome.cancelled)
		return;

	if (!outcome.ok)
	{
		if (!outcome.silentFail && !outcome.errorMessage.empty())
			openFailedUpdateMenu(outcome.errorMessage, true, dialogParent);
		return;
	}

	bool newUpdate = true;
	if (outcome.info.has_value())
	{
		currentUpdate = outcome.info;
		newUpdate = !wasCached || (currentUpdate->version != cachedVer);
		updater_last_update_check = std::to_string(curTime).c_str();
		updater_cached_update = FString(currentUpdate->version);
	}
	else
	{
		if (currentUpdate.has_value() && !currentUpdate->cached)
			newUpdate = false;
		currentUpdate.reset();
		updater_last_update_check = std::to_string(curTime).c_str();
		updater_cached_update = "";
	}
	M_SaveDefaults(nullptr);

	if (currentUpdate.has_value())
	{
		const VersionInfo current = GetCurrentVersionForUpdater();
		const bool shouldUpdate = updater_debug_always_update || (currentUpdate->version > current);

		DEBUG_LOG(
			"%s -> %s (%d%d%d)",
			FString(current).GetChars(),
			FString(currentUpdate->version).GetChars(),
			*updater_debug_always_update,
			(currentUpdate->version > current),
			skippedVer != currentUpdate->version);

		if (shouldUpdate && (force || skippedVer != currentUpdate->version))
		{
			if ((updater_auto_updates && newUpdate) || force)
				openUpdateMenu(true, dialogParent);
			else
				showBanner();
			return;
		}
	}

	hideBanner();
	if (force)
	{
		QMessageBox::information(
			resolveDialogParent(dialogParent),
			LStr("UPDATER_UP_TO_DATE", "Up to Date!"),
			LStr("UPDATER_UP_TO_DATE", "Up to Date!"));
	}
}

void LauncherUpdater::checkForUpdate(bool force, QWidget* dialogParent)
{
	DEBUG_LOG("starting");

	if (force)
		cancelPending();

	if (!updater_check_updates_initialized)
	{
		DEBUG_LOG("onboarding");
		runOnboardingFlow(dialogParent);
		if (!force)
			return;
	}

	if (!updater_check_updates && !force)
	{
		DEBUG_LOG("skipping");
		return;
	}

	bool newUpdate = true;
	if (updater_cached_update->Length() > 0)
	{
		const VersionInfo cachedVer(updater_cached_update);
		if (isVersionInvalid(cachedVer))
		{
			DEBUG_LOG("invalidating cache");
			updater_cached_update = "";
			M_SaveDefaults(nullptr);
		}
		else
		{
			DEBUG_LOG("using cache");
			newUpdate = false;
			currentUpdate = UpdaterReleaseInfo{cachedVer, true, {}, ""};
		}
	}

	VersionInfo skippedVer(USHRT_MAX, USHRT_MAX, USHRT_MAX);
	skippedVer.prerelease[0] = skippedVer.build[0] = '\0';

	if (updater_skipped_update->Length() > 0)
	{
		const VersionInfo skippedVerTmp(static_cast<const char*>(updater_skipped_update));
		if (isVersionInvalid(skippedVerTmp))
		{
			DEBUG_LOG("clearing skip");
			updater_skipped_update = "";
			M_SaveDefaults(nullptr);
		}
		else
		{
			DEBUG_LOG("skipping");
			skippedVer = skippedVerTmp;
		}
	}

	const uint64_t curTime = getCurrentDate();
	const uint64_t nextCheckTime = parseDate(static_cast<FString>(updater_last_update_check)) + daysToSeconds(updater_update_interval);
	DEBUG_LOG("%llu -> %llu (%d%d)", static_cast<unsigned long long>(curTime), static_cast<unsigned long long>(nextCheckTime), curTime >= nextCheckTime, force);

	if (curTime < nextCheckTime && !currentUpdate.has_value() && !force)
		return;

	if (!currentUpdate.has_value() || curTime >= nextCheckTime || force)
	{
		std::string curlErr;
		if (!initCurl(curlErr))
		{
			openFailedUpdateMenu(curlErr, true, dialogParent);
			return;
		}

		const bool wasCached = currentUpdate.has_value() && currentUpdate->cached;
		VersionInfo cachedVer{};
		if (wasCached)
			cachedVer = currentUpdate->version;

		if (force)
		{
			const FetchOutcome outcome = fetchUpdateInfoInteractive(dialogParent);
			handleCheckOutcome(outcome, true, wasCached, cachedVer, skippedVer, curTime, dialogParent);
		}
		else
		{
			startBackgroundCheck(cachedVer, wasCached, skippedVer, curTime);
		}
		return;
	}

	if (currentUpdate.has_value())
	{
		const VersionInfo current = GetCurrentVersionForUpdater();
		const bool shouldUpdate = updater_debug_always_update || (currentUpdate->version > current);
		if (shouldUpdate && (skippedVer != currentUpdate->version))
		{
			if ((updater_auto_updates && newUpdate) || force)
				openUpdateMenu(true, dialogParent);
			else
				showBanner();
		}
	}
}

void syncStartupUpdaterInfo(FStartupSelectionInfo& info)
{
	info.DefaultUpdateInterval = updater_update_interval;
	info.bAutoUpdate = updater_auto_updates;
	info.bCheckUpdate = updater_check_updates;
}
