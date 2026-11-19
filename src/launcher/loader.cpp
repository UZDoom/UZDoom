/*
** loader.cpp
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

#include "loader.h"
#include "const.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSet>
#include <7z.h>
#include <7zAlloc.h>
#include <7zCrc.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <miniz.h>
#include <mutex>
#include <unordered_set>

namespace
{
	struct CachedMd5Entry
	{
		qint64 size = -1;
		qint64 mtimeMs = 0;
		std::string md5;
	};

	QMutex& md5CacheMutex()
	{
		static QMutex mutex;
		return mutex;
	}

	QHash<QString, CachedMd5Entry>& md5Cache()
	{
		static QHash<QString, CachedMd5Entry> cache;
		return cache;
	}

	QString normLower(const std::string& s)
	{
		return QDir::cleanPath(qStr(s)).replace('\\', '/').toLower();
	}

	const std::string& primaryGameFileOf(const Profile& p)
	{
		static const std::string kEmpty;
		if (p.isIWAD)
			return p.iwadPath;
		if (!p.pwadPath.empty())
			return p.pwadPath;
		if (p.mods.size() == 1)
			return p.mods[0];
		return kEmpty;
	}

	bool hasGameFilePath(const std::vector<Profile>& profiles, const std::string& path)
	{
		const QString target = normLower(path);
		for (const auto& p : profiles)
		{
			const std::string& primary = primaryGameFileOf(p);
			if (!primary.empty() && normLower(primary) == target)
				return true;
		}
		return false;
	}

	bool hasGameFilePathOrHash(const std::vector<Profile>& profiles, const std::string& path, const std::string& md5)
	{
		if (hasGameFilePath(profiles, path))
			return true;
		if (md5.empty())
			return false;
		for (const auto& p : profiles)
		{
			const std::string& primary = primaryGameFileOf(p);
			if (!primary.empty() && calculateFileMD5(primary) == md5)
				return true;
		}
		return false;
	}

	bool hasExtension(const QString& path, const QStringList& exts)
	{
		const QString lower = path.toLower();
		for (const auto& ext : exts)
		{
			if (lower.endsWith(ext))
				return true;
		}
		return false;
	}

	void enrichFromTxtSidecar(const QString& filePath, Profile& prof, bool allowAnyFallback)
	{
		const QFileInfo fi(filePath);
		const QDir dir = fi.dir();
		QStringList txtCandidates = dir.entryList({"*.txt", "readme*", "README*"}, QDir::Files);
		if (txtCandidates.isEmpty())
			return;

		std::sort(txtCandidates.begin(), txtCandidates.end(), [](const QString& a, const QString& b)
		{
			return a.compare(b, Qt::CaseInsensitive) < 0;
		});

		const QString stem = fi.completeBaseName().toLower();
		QString chosen;
		for (const QString& c : txtCandidates)
		{
			if (QFileInfo(c).completeBaseName().toLower() == stem)
			{
				chosen = dir.filePath(c);
				break;
			}
		}
		if (chosen.isEmpty())
		{
			if (!allowAnyFallback)
				return;
			chosen = dir.filePath(txtCandidates.first());
		}

		QFile tf(chosen);
		if (!tf.open(QIODevice::ReadOnly | QIODevice::Text))
			return;

		const QString text = QString::fromUtf8(tf.read(16384));
		auto matchField = [&](const QString& pattern) -> QString
		{
			QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption);
			QRegularExpressionMatch m = re.match(text);
			return m.hasMatch() ? m.captured(1).trimmed() : QString();
		};

		const QString title = matchField("^\\s*Title\\s*:\\s*(.+)$");
		const QString author = matchField("^\\s*Authors?\\s*:\\s*(.+)$");
		const QString releaseDate = matchField("^\\s*Release date\\s*:\\s*(.+)$");
		if (!title.isEmpty())
			prof.title = stdStr(title);
		if (!author.isEmpty())
			prof.author = stdStr(author);
		if (!releaseDate.isEmpty())
			prof.releaseDate = normalizeReleaseDate(stdStr(releaseDate));
		if (prof.description.empty() && !text.trimmed().isEmpty())
			prof.description = stdStr(text.left(2000).trimmed());
	}

	struct WadLumpEntry
	{
		uint32_t filePos = 0;
		uint32_t size = 0;
		char name[9] = {};
	};

	uint16_t readLE16(const uint8_t* p)
	{
		return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
	}

	uint32_t readLE32(const uint8_t* p)
	{
		return static_cast<uint32_t>(p[0])
			| (static_cast<uint32_t>(p[1]) << 8)
			| (static_cast<uint32_t>(p[2]) << 16)
			| (static_cast<uint32_t>(p[3]) << 24);
	}

	bool readWadDirectory(QFile& f, std::vector<WadLumpEntry>& outDir)
	{
		if (f.size() < 12 || !f.seek(0))
			return false;

		uint8_t hdr[12] = {};
		if (f.read(reinterpret_cast<char*>(hdr), 12) != 12)
			return false;
		if (std::memcmp(hdr, "IWAD", 4) != 0 && std::memcmp(hdr, "PWAD", 4) != 0)
			return false;

		const uint32_t numLumps = readLE32(hdr + 4);
		const uint32_t dirOfs = readLE32(hdr + 8);
		if (numLumps == 0 || numLumps > 500000)
			return false;

		const uint64_t dirBytes = static_cast<uint64_t>(numLumps) * 16ull;
		if (static_cast<uint64_t>(dirOfs) + dirBytes > static_cast<uint64_t>(f.size()))
			return false;
		if (!f.seek(dirOfs))
			return false;

		QByteArray rawDir = f.read(static_cast<qint64>(dirBytes));
		if (static_cast<uint64_t>(rawDir.size()) != dirBytes)
			return false;

		outDir.resize(numLumps);
		const auto* ptr = reinterpret_cast<const uint8_t*>(rawDir.constData());
		for (uint32_t i = 0; i < numLumps; ++i)
		{
			const uint8_t* entry = ptr + static_cast<size_t>(i) * 16;
			outDir[i].filePos = readLE32(entry);
			outDir[i].size = readLE32(entry + 4);
			std::memcpy(outDir[i].name, entry + 8, 8);
			outDir[i].name[8] = '\0';
		}
		return true;
	}

	QByteArray readWadLumpByName(QFile& f, const std::vector<WadLumpEntry>& dir, const char* targetName, int maxBytes = 16 * 1024 * 1024)
	{
		const uint64_t fileSize = static_cast<uint64_t>(f.size());
		for (int i = static_cast<int>(dir.size()) - 1; i >= 0; --i)
		{
			const WadLumpEntry& e = dir[i];
			if (e.size == 0 || e.size > static_cast<uint32_t>(maxBytes))
				continue;
			if (static_cast<uint64_t>(e.filePos) + e.size > fileSize)
				continue;
			if (qstricmp(e.name, targetName) == 0)
			{
				if (!f.seek(e.filePos))
					return {};
				return f.read(e.size);
			}
		}
		return {};
	}

	size_t zipFileReadCb(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n)
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

	QByteArray readZipEntryBytes(mz_zip_archive& zip, mz_uint index, size_t maxBytes = 16 * 1024 * 1024)
	{
		mz_zip_archive_file_stat st{};
		if (!mz_zip_reader_file_stat(&zip, index, &st) || mz_zip_reader_is_file_a_directory(&zip, index))
			return {};
		if (st.m_uncomp_size == 0 || st.m_uncomp_size > maxBytes)
			return {};

		QByteArray buf(static_cast<int>(st.m_uncomp_size), Qt::Uninitialized);
		if (!mz_zip_reader_extract_to_mem(&zip, index, buf.data(), buf.size(), 0))
			return {};
		return buf;
	}

	struct QFile7zInStream
	{
		ISeekInStream vt{};
		QFile* file = nullptr;

		QFile7zInStream() = default;

		explicit QFile7zInStream(QFile* f)
			: file(f)
		{
			vt.Read = readCb;
			vt.Seek = seekCb;
		}

		static SRes readCb(ISeekInStreamPtr pp, void* buf, size_t* size)
		{
			auto* self = reinterpret_cast<QFile7zInStream*>(const_cast<ISeekInStream*>(pp));
			if (!self || !self->file || !size)
			{
				if (size)
					*size = 0;
				return SZ_ERROR_READ;
			}
			const qint64 want = static_cast<qint64>(*size);
			const qint64 got = self->file->read(static_cast<char*>(buf), want);
			if (got < 0)
			{
				*size = 0;
				return SZ_ERROR_READ;
			}
			*size = static_cast<size_t>(got);
			return SZ_OK;
		}

		static SRes seekCb(ISeekInStreamPtr pp, Int64* pos, ESzSeek origin)
		{
			auto* self = reinterpret_cast<QFile7zInStream*>(const_cast<ISeekInStream*>(pp));
			if (!self || !self->file || !pos)
				return SZ_ERROR_READ;

			qint64 target = *pos;
			if (origin == SZ_SEEK_CUR)
				target = self->file->pos() + *pos;
			else if (origin == SZ_SEEK_END)
				target = self->file->size() + *pos;
			else if (origin != SZ_SEEK_SET)
				return 1;

			if (target < 0 || target > self->file->size() || !self->file->seek(target))
				return 1;

			*pos = self->file->pos();
			return SZ_OK;
		}
	};

	std::string sevenZipErrorString(SRes res)
	{
		switch (res)
		{
		case SZ_ERROR_UNSUPPORTED:
			return "Unsupported 7z compression method or encrypted archive.";
		case SZ_ERROR_CRC:
			return "7z archive CRC check failed.";
		case SZ_ERROR_MEM:
			return "Out of memory while reading 7z archive.";
		case SZ_ERROR_NO_ARCHIVE:
		case SZ_ERROR_ARCHIVE:
			return "Invalid or corrupted 7z archive.";
		default:
			return "Failed to extract 7z archive.";
		}
	}

	class ArchiveReaderView
	{
	public:
		virtual ~ArchiveReaderView() = default;
		virtual uint32_t count() const = 0;
		virtual bool statFile(uint32_t index, QString& outName, uint64_t& outSize) = 0;
		virtual QByteArray readFile(uint32_t index, size_t maxBytes) = 0;
	};

	class ZipArchiveView final : public ArchiveReaderView
	{
	public:
		~ZipArchiveView() override
		{
			if (opened)
				mz_zip_reader_end(&zip);
		}

		bool open(QFile& f)
		{
			if (opened)
			{
				mz_zip_reader_end(&zip);
				opened = false;
			}
			std::memset(&zip, 0, sizeof(zip));
			zip.m_pRead = zipFileReadCb;
			zip.m_pIO_opaque = &f;
			if (!mz_zip_reader_init(&zip, static_cast<mz_uint64>(f.size()), 0))
				return false;
			opened = true;
			return true;
		}

		uint32_t count() const override
		{
			return opened ? static_cast<uint32_t>(mz_zip_reader_get_num_files(const_cast<mz_zip_archive*>(&zip))) : 0;
		}

		bool statFile(uint32_t index, QString& outName, uint64_t& outSize) override
		{
			if (!opened)
				return false;
			mz_zip_archive_file_stat st{};
			if (!mz_zip_reader_file_stat(&zip, index, &st) || mz_zip_reader_is_file_a_directory(&zip, index))
				return false;
			outName = QString::fromUtf8(st.m_filename);
			outSize = st.m_uncomp_size;
			return true;
		}

		QByteArray readFile(uint32_t index, size_t maxBytes) override
		{
			return opened ? readZipEntryBytes(zip, index, maxBytes) : QByteArray();
		}

	private:
		mz_zip_archive zip{};
		bool opened = false;
	};

	class SevenZipReader final : public ArchiveReaderView
	{
	public:
		SevenZipReader() = default;
		~SevenZipReader() override
		{
			close();
		}

		SevenZipReader(const SevenZipReader&) = delete;
		SevenZipReader& operator=(const SevenZipReader&) = delete;

		bool open(QFile& file, SRes* outRes = nullptr)
		{
			close();
			static std::once_flag crcOnce;
			std::call_once(crcOnce, CrcGenerateTable);

			if (!file.seek(0))
			{
				if (outRes)
					*outRes = SZ_ERROR_READ;
				return false;
			}

			inStream = QFile7zInStream(&file);
			LookToRead2_CreateVTable(&lookStream, false);
			lookStream.realStream = &inStream.vt;
			LookToRead2_INIT(&lookStream);
			lookStream.bufSize = sizeof(streamBuf);
			lookStream.buf = streamBuf;

			SzArEx_Init(&db);
			initialized = true;

			const SRes res = SzArEx_Open(&db, &lookStream.vt, &allocMain, &allocTemp);
			if (outRes)
				*outRes = res;
			if (res != SZ_OK)
			{
				close();
				return false;
			}
			return true;
		}

		void close()
		{
			if (outBuffer)
			{
				IAlloc_Free(&allocMain, outBuffer);
				outBuffer = nullptr;
			}
			outBufferSize = 0;
			blockIndex = 0xFFFFFFFFu;
			if (initialized)
			{
				SzArEx_Free(&db, &allocMain);
				initialized = false;
			}
		}

		uint32_t count() const override
		{
			return initialized ? static_cast<uint32_t>(db.NumFiles) : 0;
		}

		bool isDir(uint32_t index) const
		{
			return !initialized || index >= db.NumFiles || (db.IsDirs && SzArEx_IsDir(&db, index));
		}

		QString entryName(uint32_t index) const
		{
			if (!initialized || !db.FileNameOffsets || index >= db.NumFiles)
				return {};
			const size_t len = SzArEx_GetFileNameUtf16(&db, index, nullptr);
			if (len == 0)
				return {};
			std::vector<UInt16> utf16(len);
			SzArEx_GetFileNameUtf16(&db, index, utf16.data());
			while (!utf16.empty() && utf16.back() == 0)
				utf16.pop_back();
			if (utf16.empty())
				return {};
			return QString::fromUtf16(reinterpret_cast<const char16_t*>(utf16.data()), static_cast<qsizetype>(utf16.size()));
		}

		bool statFile(uint32_t index, QString& outName, uint64_t& outSize) override
		{
			if (isDir(index))
				return false;
			outName = entryName(index);
			if (outName.isEmpty())
				return false;
			outSize = SzArEx_GetFileSize(&db, index);
			return true;
		}

		QByteArray readFile(uint32_t index, size_t maxBytes) override
		{
			if (!initialized || isDir(index))
				return {};
			const UInt64 sz = SzArEx_GetFileSize(&db, index);
			if (sz == 0 || sz > maxBytes || sz > static_cast<UInt64>(std::numeric_limits<int>::max()))
				return {};

			size_t offset = 0;
			size_t outSize = 0;
			if (!extractCached(index, offset, outSize))
				return {};
			return QByteArray(reinterpret_cast<const char*>(outBuffer + offset), static_cast<int>(outSize));
		}

		bool extractEntryToFile(uint32_t index, QFile& outFile, SRes* outRes = nullptr)
		{
			if (!initialized || isDir(index))
				return false;

			size_t offset = 0;
			size_t outSize = 0;
			if (!extractCached(index, offset, outSize, outRes))
				return false;

			const char* dataPtr = reinterpret_cast<const char*>(outBuffer + offset);
			size_t writtenTotal = 0;
			while (writtenTotal < outSize)
			{
				const size_t step = std::min<size_t>(outSize - writtenTotal, 64 * 1024 * 1024);
				const qint64 chunk = outFile.write(dataPtr + writtenTotal, static_cast<qint64>(step));
				if (chunk <= 0)
					return false;
				writtenTotal += static_cast<size_t>(chunk);
			}
			return true;
		}

	private:
		bool extractCached(uint32_t index, size_t& offset, size_t& outSize, SRes* outRes = nullptr)
		{
			const SRes res = SzArEx_Extract(
				&db,
				&lookStream.vt,
				index,
				&blockIndex,
				&outBuffer,
				&outBufferSize,
				&offset,
				&outSize,
				&allocMain,
				&allocTemp);
			if (outRes)
				*outRes = res;
			if (res != SZ_OK)
			{
				if (outBuffer)
				{
					IAlloc_Free(&allocMain, outBuffer);
					outBuffer = nullptr;
				}
				outBufferSize = 0;
				blockIndex = 0xFFFFFFFFu;
				return false;
			}
			return true;
		}

		ISzAlloc allocMain{ SzAlloc, SzFree };
		ISzAlloc allocTemp{ SzAllocTemp, SzFreeTemp };
		QFile7zInStream inStream;
		CLookToRead2 lookStream{};
		Byte streamBuf[1 << 14]{};
		CSzArEx db{};
		UInt32 blockIndex = 0xFFFFFFFFu;
		Byte* outBuffer = nullptr;
		size_t outBufferSize = 0;
		bool initialized = false;
	};

	bool findPlaypalInArchive(ArchiveReaderView& arch, uint8_t outPalette[768])
	{
		const uint32_t count = arch.count();
		for (uint32_t i = 0; i < count; ++i)
		{
			QString name;
			uint64_t size = 0;
			if (!arch.statFile(i, name, size) || size < 768)
				continue;
			const QString base = QFileInfo(name.replace('\\', '/')).completeBaseName();
			if (base.compare("playpal", Qt::CaseInsensitive) == 0)
			{
				const QByteArray pal = arch.readFile(i, 65536);
				if (pal.size() >= 768)
				{
					std::memcpy(outPalette, pal.constData(), 768);
					return true;
				}
			}
		}
		return false;
	}

	bool tryLoadPlaypalFromFile(const QString& filePath, uint8_t outPalette[768])
	{
		if (filePath.isEmpty() || !QFileInfo::exists(filePath))
			return false;

		QFile f(filePath);
		if (!f.open(QIODevice::ReadOnly))
			return false;

		std::vector<WadLumpEntry> wadDir;
		if (readWadDirectory(f, wadDir))
		{
			const QByteArray pal = readWadLumpByName(f, wadDir, "PLAYPAL", 65536);
			if (pal.size() >= 768)
			{
				std::memcpy(outPalette, pal.constData(), 768);
				return true;
			}
			return false;
		}

		ZipArchiveView zip;
		if (zip.open(f))
			return findPlaypalInArchive(zip, outPalette);

		SevenZipReader sz;
		if (sz.open(f))
			return findPlaypalInArchive(sz, outPalette);

		return false;
	}

	void resolvePaletteForProfile(const Profile& prof, const QString& primaryFile, uint8_t outPalette[768])
	{
		// Fallback to standard Doom PLAYPAL when neither the mod nor any installed IWAD supplies one.
		static constexpr uint8_t kDefaultDoomPlaypal[768] = {
			  0,   0,   0,  31,  23,  11,  23,  15,   7,  75,  75,  75,
			255, 255, 255,  27,  27,  27,  19,  19,  19,  11,  11,  11,
			  7,   7,   7,  47,  55,  31,  35,  43,  15,  23,  31,   7,
			 15,  23,   0,  79,  59,  43,  71,  51,  35,  63,  43,  27,
			255, 183, 183, 247, 171, 171, 243, 163, 163, 235, 151, 151,
			231, 143, 143, 223, 135, 135, 219, 123, 123, 211, 115, 115,
			203, 107, 107, 199,  99,  99, 191,  91,  91, 187,  87,  87,
			179,  79,  79, 175,  71,  71, 167,  63,  63, 163,  59,  59,
			155,  51,  51, 151,  47,  47, 143,  43,  43, 139,  35,  35,
			131,  31,  31, 127,  27,  27, 119,  23,  23, 115,  19,  19,
			107,  15,  15, 103,  11,  11,  95,   7,   7,  91,   7,   7,
			 83,   7,   7,  79,   0,   0,  71,   0,   0,  67,   0,   0,
			255, 235, 223, 255, 227, 211, 255, 219, 199, 255, 211, 187,
			255, 207, 179, 255, 199, 167, 255, 191, 155, 255, 187, 147,
			255, 179, 131, 247, 171, 123, 239, 163, 115, 231, 155, 107,
			223, 147,  99, 215, 139,  91, 207, 131,  83, 203, 127,  79,
			191, 123,  75, 179, 115,  71, 171, 111,  67, 163, 107,  63,
			155,  99,  59, 143,  95,  55, 135,  87,  51, 127,  83,  47,
			119,  79,  43, 107,  71,  39,  95,  67,  35,  83,  63,  31,
			 75,  55,  27,  63,  47,  23,  51,  43,  19,  43,  35,  15,
			239, 239, 239, 231, 231, 231, 223, 223, 223, 219, 219, 219,
			211, 211, 211, 203, 203, 203, 199, 199, 199, 191, 191, 191,
			183, 183, 183, 179, 179, 179, 171, 171, 171, 167, 167, 167,
			159, 159, 159, 151, 151, 151, 147, 147, 147, 139, 139, 139,
			131, 131, 131, 127, 127, 127, 119, 119, 119, 111, 111, 111,
			107, 107, 107,  99,  99,  99,  91,  91,  91,  87,  87,  87,
			 79,  79,  79,  71,  71,  71,  67,  67,  67,  59,  59,  59,
			 55,  55,  55,  47,  47,  47,  39,  39,  39,  35,  35,  35,
			119, 255, 111, 111, 239, 103, 103, 223,  95,  95, 207,  87,
			 91, 191,  79,  83, 175,  71,  75, 159,  63,  67, 147,  55,
			 63, 131,  47,  55, 115,  43,  47,  99,  35,  39,  83,  27,
			 31,  67,  23,  23,  51,  15,  19,  35,  11,  11,  23,   7,
			191, 167, 143, 183, 159, 135, 175, 151, 127, 167, 143, 119,
			159, 135, 111, 155, 127, 107, 147, 123,  99, 139, 115,  91,
			131, 107,  87, 123,  99,  79, 119,  95,  75, 111,  87,  67,
			103,  83,  63,  95,  75,  55,  87,  67,  51,  83,  63,  47,
			159, 131,  99, 143, 119,  83, 131, 107,  75, 119,  95,  63,
			103,  83,  51,  91,  71,  43,  79,  59,  35,  67,  51,  27,
			123, 127,  99, 111, 115,  87, 103, 107,  79,  91,  99,  71,
			 83,  87,  59,  71,  79,  51,  63,  71,  43,  55,  63,  39,
			255, 255, 115, 235, 219,  87, 215, 187,  67, 195, 155,  47,
			175, 123,  31, 155,  91,  19, 135,  67,   7, 115,  43,   0,
			255, 255, 255, 255, 219, 219, 255, 187, 187, 255, 155, 155,
			255, 123, 123, 255,  95,  95, 255,  63,  63, 255,  31,  31,
			255,   0,   0, 239,   0,   0, 227,   0,   0, 215,   0,   0,
			203,   0,   0, 191,   0,   0, 179,   0,   0, 167,   0,   0,
			155,   0,   0, 139,   0,   0, 127,   0,   0, 115,   0,   0,
			103,   0,   0,  91,   0,   0,  79,   0,   0,  67,   0,   0,
			231, 231, 255, 199, 199, 255, 171, 171, 255, 143, 143, 255,
			115, 115, 255,  83,  83, 255,  55,  55, 255,  27,  27, 255,
			  0,   0, 255,   0,   0, 227,   0,   0, 203,   0,   0, 179,
			  0,   0, 155,   0,   0, 131,   0,   0, 107,   0,   0,  83,
			255, 255, 255, 255, 235, 219, 255, 215, 187, 255, 199, 155,
			255, 179, 123, 255, 163,  91, 255, 143,  59, 255, 127,  27,
			243, 115,  23, 235, 111,  15, 223, 103,  15, 215,  95,  11,
			203,  87,   7, 195,  79,   0, 183,  71,   0, 175,  67,   0,
			255, 255, 255, 255, 255, 215, 255, 255, 179, 255, 255, 143,
			255, 255, 107, 255, 255,  71, 255, 255,  35, 255, 255,   0,
			167,  63,   0, 159,  55,   0, 147,  47,   0, 135,  35,   0,
			 79,  59,  39,  67,  47,  27,  55,  35,  19,  47,  27,  11,
			  0,   0,  83,   0,   0,  71,   0,   0,  59,   0,   0,  47,
			  0,   0,  35,   0,   0,  23,   0,   0,  11,   0,   0,   0,
			255, 159,  67, 255, 231,  75, 255, 123, 255, 255,   0, 255,
			207,   0, 207, 159,   0, 155, 111,   0, 107, 167, 107, 107,
		};
		std::memcpy(outPalette, kDefaultDoomPlaypal, 768);

		if (tryLoadPlaypalFromFile(primaryFile, outPalette))
			return;
		for (int i = static_cast<int>(prof.mods.size()) - 1; i >= 0; --i)
		{
			if (tryLoadPlaypalFromFile(qStr(prof.mods[i]), outPalette))
				return;
		}
		if (!prof.iwadPath.empty() && tryLoadPlaypalFromFile(qStr(prof.iwadPath), outPalette))
			return;
		for (const auto& w : g_discoveredEngineIWads)
		{
			if (tryLoadPlaypalFromFile(qStr(w.path), outPalette))
				return;
		}
	}

	QImage applyDoomAspectCorrection(QImage img)
	{
		if (img.isNull())
			return img;
		if (img.width() == 320 && img.height() == 200)
		{
			const QImage integerUpscaled = img.scaled(1600, 1200, Qt::IgnoreAspectRatio, Qt::FastTransformation);
			return integerUpscaled.scaled(640, 480, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		if (img.width() == 640 && img.height() == 400)
			return img.scaled(640, 480, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		return img;
	}

	QImage decodeDoomPatchOrRaw(const QByteArray& data, const uint8_t palette[768])
	{
		if (data.isEmpty())
			return {};

		QImage stdImg = QImage::fromData(reinterpret_cast<const uchar*>(data.constData()), data.size());
		if (!stdImg.isNull() && stdImg.width() > 0 && stdImg.height() > 0)
			return applyDoomAspectCorrection(stdImg);

		if (data.size() >= 13)
		{
			const auto* raw = reinterpret_cast<const uint8_t*>(data.constData());
			const uint32_t totalLen = static_cast<uint32_t>(data.size());
			const int width = readLE16(raw);
			const int height = readLE16(raw + 2);
			const int leftOffset = static_cast<int16_t>(readLE16(raw + 4));
			const int topOffset = static_cast<int16_t>(readLE16(raw + 6));
			const uint32_t minHeaderSize = static_cast<uint32_t>(width) * 4u + 8u;

			if (width > 0 && width <= 2048 && height > 0 && height <= 2048
				&& std::abs(leftOffset) < 4096 && std::abs(topOffset) < 4096
				&& minHeaderSize < totalLen)
			{
				bool validColumns = true;
				bool hasStartCol = false;
				for (int x = 0; x < width; ++x)
				{
					const uint32_t colOfs = readLE32(raw + 8 + static_cast<size_t>(x) * 4);
					if (colOfs == minHeaderSize)
						hasStartCol = true;
					else if (colOfs >= totalLen)
					{
						validColumns = false;
						break;
					}
				}

				if (validColumns && hasStartCol)
				{
					QImage img(width, height, QImage::Format_ARGB32);
					img.fill(qRgba(0, 0, 0, 255));

					for (int x = 0; x < width; ++x)
					{
						uint32_t colOfs = readLE32(raw + 8 + static_cast<size_t>(x) * 4);
						int top = -1;
						int safety = 0;
						while (colOfs + 3 <= totalLen && safety++ < 4096)
						{
							const uint8_t topdelta = raw[colOfs];
							if (topdelta == 0xFF)
								break;
							const uint8_t length = raw[colOfs + 1];
							if (topdelta <= top)
								top += topdelta;
							else
								top = topdelta;

							const uint32_t pixelStart = colOfs + 3;
							if (pixelStart + length > totalLen)
								break;

							for (int i = 0; i < length; ++i)
							{
								const int y = top + i;
								if (y >= 0 && y < height)
								{
									const uint8_t idx = raw[pixelStart + i];
									reinterpret_cast<QRgb*>(img.scanLine(y))[x] = qRgba(
										palette[idx * 3 + 0],
										palette[idx * 3 + 1],
										palette[idx * 3 + 2],
										255);
								}
							}
							colOfs += static_cast<uint32_t>(length) + 4u;
						}
					}
					return applyDoomAspectCorrection(img);
				}
			}
		}

		if (data.size() == 64000)
		{
			const auto* raw = reinterpret_cast<const uint8_t*>(data.constData());
			QImage img(320, 200, QImage::Format_ARGB32);
			for (int y = 0; y < 200; ++y)
			{
				auto* row = reinterpret_cast<QRgb*>(img.scanLine(y));
				for (int x = 0; x < 320; ++x)
				{
					const uint8_t idx = raw[y * 320 + x];
					row[x] = qRgba(palette[idx * 3 + 0], palette[idx * 3 + 1], palette[idx * 3 + 2], 255);
				}
			}
			return applyDoomAspectCorrection(img);
		}

		return {};
	}

	QImage extractTitleImageFromArchive(ArchiveReaderView& arch, const uint8_t palette[768])
	{
		static const char* const kTitleLumps[] = {
			"TITLEPIC", "DMENUPIC", "TITLE", "INTERPIC", "BOSSBACK", "CREDIT"
		};

		const uint32_t count = arch.count();
		for (const char* lumpName : kTitleLumps)
		{
			for (int i = static_cast<int>(count) - 1; i >= 0; --i)
			{
				QString entryPath;
				uint64_t size = 0;
				if (!arch.statFile(static_cast<uint32_t>(i), entryPath, size))
					continue;

				entryPath.replace('\\', '/');
				const QFileInfo entryFi(entryPath);
				const QString base = entryFi.completeBaseName();
				if (base.compare(QLatin1String(lumpName), Qt::CaseInsensitive) != 0)
					continue;

				const QString dirPrefix = entryFi.path().toLower();
				if (!dirPrefix.isEmpty() && dirPrefix != "." && !dirPrefix.startsWith("graphics") && !dirPrefix.startsWith("hires"))
					continue;

				const QByteArray bytes = arch.readFile(static_cast<uint32_t>(i), 16 * 1024 * 1024);
				QImage img = decodeDoomPatchOrRaw(bytes, palette);
				if (!img.isNull())
					return img;
			}
		}

		for (uint32_t i = 0; i < count; ++i)
		{
			QString entryPath;
			uint64_t size = 0;
			if (!arch.statFile(i, entryPath, size))
				continue;

			const QString lower = entryPath.toLower();
			if ((lower.contains("cover") || lower.contains("banner") || lower.contains("titlepic"))
				&& (lower.endsWith(".png") || lower.endsWith(".jpg") || lower.endsWith(".jpeg")))
			{
				const QByteArray bytes = arch.readFile(i, 16 * 1024 * 1024);
				QImage img = decodeDoomPatchOrRaw(bytes, palette);
				if (!img.isNull())
					return img;
			}
		}

		return {};
	}

	QImage extractTitleImageFromSingleFile(const QString& filePath, const uint8_t palette[768])
	{
		if (filePath.isEmpty() || !QFileInfo::exists(filePath))
			return {};

		QFile f(filePath);
		if (!f.open(QIODevice::ReadOnly))
			return {};

		static const char* const kTitleLumps[] = {
			"TITLEPIC", "DMENUPIC", "TITLE", "INTERPIC", "BOSSBACK", "CREDIT"
		};

		std::vector<WadLumpEntry> wadDir;
		if (readWadDirectory(f, wadDir))
		{
			for (const char* lumpName : kTitleLumps)
			{
				const QByteArray bytes = readWadLumpByName(f, wadDir, lumpName);
				if (bytes.isEmpty())
					continue;
				QImage img = decodeDoomPatchOrRaw(bytes, palette);
				if (!img.isNull())
					return img;
			}
			return {};
		}

		ZipArchiveView zip;
		if (zip.open(f))
			return extractTitleImageFromArchive(zip, palette);

		SevenZipReader sz;
		if (sz.open(f))
			return extractTitleImageFromArchive(sz, palette);

		return {};
	}

	bool extract7zArchiveToDir(const QString& archivePath, const QString& destDir, std::string* errOut)
	{
		QFile inFile(archivePath);
		if (!inFile.open(QIODevice::ReadOnly))
		{
			if (errOut)
				*errOut = "Failed to open archive.";
			return false;
		}

		SevenZipReader sz;
		SRes res = SZ_OK;
		if (!sz.open(inFile, &res))
		{
			if (errOut)
				*errOut = sevenZipErrorString(res);
			return false;
		}

		const uint32_t numFiles = sz.count();
		if (numFiles == 0)
		{
			if (errOut)
				*errOut = "Archive is empty.";
			return false;
		}

		const QString cleanDest = QDir::cleanPath(destDir);
		const QString destPrefix = cleanDest.endsWith('/') ? cleanDest : (cleanDest + '/');
		QDir().mkpath(cleanDest);

		for (uint32_t i = 0; i < numFiles; ++i)
		{
			QString entryName = sz.entryName(i).replace('\\', '/');
			while (entryName.startsWith('/'))
				entryName.remove(0, 1);
			if (entryName.isEmpty() || entryName.contains(':') || QFileInfo(entryName).isAbsolute())
				continue;

			const QStringList parts = entryName.split('/', Qt::SkipEmptyParts);
			if (parts.isEmpty() || parts.contains(".."))
				continue;

			const QString outPath = QDir::cleanPath(QDir(cleanDest).filePath(entryName));
			if (!outPath.startsWith(destPrefix))
				continue;

			if (sz.isDir(i))
			{
				QDir().mkpath(outPath);
				continue;
			}

			QDir().mkpath(QFileInfo(outPath).absolutePath());
			QFile outFile(outPath);
			if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				if (errOut)
					*errOut = "Failed to extract archive entry: " + stdStr(entryName);
				return false;
			}

			if (!sz.extractEntryToFile(i, outFile, &res))
			{
				if (errOut)
					*errOut = sevenZipErrorString(res) + " (" + stdStr(entryName) + ")";
				return false;
			}
		}

		return true;
	}

	bool extractArchiveToDir(const QString& archivePath, const QString& destDir, std::string* errOut)
	{
		const QString lower = archivePath.toLower();
		if (lower.endsWith(".zip"))
			return extractZipArchiveToDir(archivePath, destDir, nullptr, errOut);
		if (lower.endsWith(".7z"))
			return extract7zArchiveToDir(archivePath, destDir, errOut);

		if (errOut)
			*errOut = "Unsupported archive format.";
		return false;
	}

	bool checkIsIWADFile(const std::string& path, const QString& md5)
	{
		const auto& db = getWadDatabase();
		if (auto it = db.find(md5.toStdString()); it != db.end())
			return it->second.isIWAD;

		const QString lower = normLower(path);
		if (lower.endsWith(".ipk3") || lower.endsWith(".ipk7") || lower.endsWith(".iwad"))
			return true;

		for (const auto& w : g_discoveredEngineIWads)
		{
			if (normLower(w.path) == lower)
				return true;
		}

		return isWadHeaderIWAD(path);
	}
}

std::string calculateFileMD5(const std::string& filepath)
{
	const QString qPath = qStr(filepath);
	const QFileInfo fi(qPath);
	if (!fi.exists() || !fi.isFile())
		return {};

	const QString cacheKey = QDir::cleanPath(fi.absoluteFilePath()).replace('\\', '/').toLower();
	const qint64 fileSize = fi.size();
	const qint64 mtimeMs = fi.lastModified().isValid() ? fi.lastModified().toMSecsSinceEpoch() : 0;

	{
		QMutexLocker lock(&md5CacheMutex());
		const auto it = md5Cache().constFind(cacheKey);
		if (it != md5Cache().constEnd() && it->size == fileSize && it->mtimeMs == mtimeMs)
			return it->md5;
	}

	QFile f(qPath);
	if (!f.open(QIODevice::ReadOnly))
		return {};

	QCryptographicHash hash(QCryptographicHash::Md5);
	if (!hash.addData(&f))
		return {};
	const std::string md5 = hash.result().toHex().toLower().toStdString();

	{
		QMutexLocker lock(&md5CacheMutex());
		md5Cache().insert(cacheKey, CachedMd5Entry{fileSize, mtimeMs, md5});
	}
	return md5;
}

bool isWadHeaderIWAD(const std::string& filepath)
{
	QFile f(qStr(filepath));
	if (!f.open(QIODevice::ReadOnly))
		return false;
	char magic[4] = {};
	if (f.read(magic, 4) != 4)
		return false;
	return std::memcmp(magic, "IWAD", 4) == 0;
}

bool tryExtractCoverForProfile(Profile& prof)
{
	if (!prof.coverImagePath.empty() && QFileInfo::exists(qStr(prof.coverImagePath)))
		return true;
	if (prof.filePath.empty())
		return false;

	const QString profFolder = QFileInfo(qStr(prof.filePath)).absolutePath();
	if (profFolder.isEmpty())
		return false;

	const QString primaryPath = qStr(prof.isIWAD ? prof.iwadPath : prof.pwadPath);
	uint8_t palette[768] = {};
	resolvePaletteForProfile(prof, primaryPath, palette);

	QStringList sources;
	if (!prof.isIWAD && !prof.pwadPath.empty())
		sources << qStr(prof.pwadPath);
	for (int i = static_cast<int>(prof.mods.size()) - 1; i >= 0; --i)
	{
		if (!prof.mods[i].empty())
			sources << qStr(prof.mods[i]);
	}
	if (!prof.iwadPath.empty())
		sources << qStr(prof.iwadPath);

	for (const QString& src : sources)
	{
		QImage img = extractTitleImageFromSingleFile(src, palette);
		if (!img.isNull())
		{
			QDir().mkpath(profFolder);
			const QString outPath = QDir(profFolder).filePath("cover.png");
			if (img.save(outPath, "PNG"))
			{
				prof.coverImagePath = stdStr(outPath);
				return true;
			}
		}
	}
	return false;
}

ImportOutcome importGameFileIntoProfiles(const std::string& filePath, const std::vector<Profile>& existingProfiles)
{
	ImportOutcome out;
	const QFileInfo fi(qStr(filePath));
	if (!fi.exists() || !fi.isFile())
	{
		out.status = ImportResult::UnsupportedFile;
		out.message = "Selected file does not exist.";
		return out;
	}

	const QString lower = fi.fileName().toLower();
	if (lower.endsWith(".uzdp"))
	{
		Profile imported;
		std::string err;
		if (!importProfileFromZip(filePath, imported, err))
		{
			out.status = ImportResult::ExtractionFailed;
			out.message = err;
			return out;
		}
		out.status = ImportResult::Success;
		out.createdProfile = std::move(imported);
		return out;
	}

	if (hasExtension(lower, {".zip", ".7z"}))
	{
		Profile prof;
		prof.title = stdStr(fi.completeBaseName());
		createDefaultDirectoryStructure(prof);
		const QString profFolder = QFileInfo(qStr(prof.filePath)).absolutePath();
		const QString extractDir = QDir(profFolder).filePath("content");
		QDir().mkpath(extractDir);

		std::string extractErr;
		if (!extractArchiveToDir(fi.absoluteFilePath(), extractDir, &extractErr))
		{
			QDir(profFolder).removeRecursively();
			out.status = ImportResult::ExtractionFailed;
			out.message = extractErr.empty() ? "Failed to extract archive." : extractErr;
			return out;
		}

		QStringList extractedAll;
		QDirIterator it(extractDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext())
			extractedAll << it.next();

		const QDir extractQDir(extractDir);
		std::sort(extractedAll.begin(), extractedAll.end(), [&](const QString& a, const QString& b)
		{
			const QString relA = extractQDir.relativeFilePath(a).replace('\\', '/');
			const QString relB = extractQDir.relativeFilePath(b).replace('\\', '/');
			const int depthA = relA.count('/');
			const int depthB = relB.count('/');
			if (depthA != depthB)
				return depthA < depthB;
			return relA.compare(relB, Qt::CaseInsensitive) < 0;
		});

		QString primaryFile;
		QStringList extraMods;
		for (const QString& f : extractedAll)
		{
			const QString fl = f.toLower();
			if (hasExtension(fl, {".wad", ".iwad", ".pwad", ".pk3", ".ipk3", ".pk7", ".ipk7"}))
			{
				if (primaryFile.isEmpty())
					primaryFile = f;
				else
					extraMods << f;
			}
			else if (hasExtension(fl, {".deh", ".bex"}))
			{
				extraMods << f;
			}
		}

		if (primaryFile.isEmpty() && extraMods.isEmpty())
		{
			QDir(profFolder).removeRecursively();
			out.status = ImportResult::UnsupportedFile;
			out.message = "Archive does not contain a runnable WAD, PK3, PK7, DEH, or BEX file.";
			return out;
		}

		if (!primaryFile.isEmpty())
		{
			const std::string primaryStd = stdStr(primaryFile);
			const QString md5 = qStr(calculateFileMD5(primaryStd));
			prof.isIWAD = checkIsIWADFile(primaryStd, md5);
			const auto& db = getWadDatabase();
			if (auto dbIt = db.find(md5.toStdString()); dbIt != db.end())
			{
				prof.title = dbIt->second.title;
				prof.author = dbIt->second.author;
				prof.releaseDate = dbIt->second.releaseDate;
				prof.isIWAD = dbIt->second.isIWAD;
			}
			else
			{
				enrichFromTxtSidecar(primaryFile, prof, true);
			}

			if (prof.isIWAD)
			{
				prof.iwadPath = primaryStd;
			}
			else
			{
				prof.pwadPath = primaryStd;
				prof.iwadPath = findBestDefaultIWADPath(existingProfiles);
			}
		}
		else
		{
			prof.isIWAD = false;
			prof.iwadPath = findBestDefaultIWADPath(existingProfiles);
			enrichFromTxtSidecar(extraMods.first(), prof, true);
		}

		for (const auto& m : extraMods)
			prof.mods.push_back(stdStr(m));

		tryExtractCoverForProfile(prof);
		saveProfileToDisk(prof);
		out.status = ImportResult::Success;
		out.createdProfile = std::move(prof);
		return out;
	}

	if (!hasExtension(lower, {".wad", ".iwad", ".pwad", ".pk3", ".ipk3", ".pk7", ".ipk7", ".deh", ".bex"}))
	{
		out.status = ImportResult::UnsupportedFile;
		out.message = "Unsupported file type. Select a WAD, PK3, PK7, DEH, BEX, ZIP, 7Z, or UZDP file.";
		return out;
	}

	const std::string md5 = calculateFileMD5(filePath);
	if (hasGameFilePathOrHash(existingProfiles, filePath, md5))
	{
		out.status = ImportResult::DuplicateProfile;
		out.message = "A profile for this file already exists.";
		return out;
	}

	Profile prof;
	const auto& db = getWadDatabase();
	if (auto dbIt = db.find(md5); dbIt != db.end())
	{
		prof.title = dbIt->second.title;
		prof.author = dbIt->second.author;
		prof.releaseDate = dbIt->second.releaseDate;
		prof.isIWAD = dbIt->second.isIWAD;
	}
	else
	{
		prof.title = stdStr(fi.completeBaseName());
		prof.isIWAD = checkIsIWADFile(filePath, qStr(md5));
		for (const auto& w : g_discoveredEngineIWads)
		{
			if (normLower(w.path) == normLower(filePath) && !w.name.empty())
			{
				prof.title = w.name;
				break;
			}
		}
		if (fi.birthTime().isValid())
			prof.releaseDate = stdStr(fi.birthTime().date().toString("dd-MM-yyyy"));
		else if (fi.lastModified().isValid())
			prof.releaseDate = stdStr(fi.lastModified().date().toString("dd-MM-yyyy"));
		enrichFromTxtSidecar(fi.absoluteFilePath(), prof, false);
	}

	createDefaultDirectoryStructure(prof);
	const QString profFolder = QFileInfo(qStr(prof.filePath)).absolutePath();
	QString storedPath = fi.absoluteFilePath();

	if (!normLower(filePath).startsWith(normLower(LauncherConst::PROFILE_DIR) + "/"))
	{
		const QString copyTarget = QDir(profFolder).filePath(fi.fileName());
		if (QFile::copy(fi.absoluteFilePath(), copyTarget))
			storedPath = copyTarget;
	}

	if (prof.isIWAD)
	{
		prof.iwadPath = stdStr(storedPath);
	}
	else
	{
		if (hasExtension(lower, {".deh", ".bex"}))
			prof.mods.push_back(stdStr(storedPath));
		else
			prof.pwadPath = stdStr(storedPath);
		prof.iwadPath = findBestDefaultIWADPath(existingProfiles);
	}

	tryExtractCoverForProfile(prof);
	saveProfileToDisk(prof);
	out.status = ImportResult::Success;
	out.createdProfile = std::move(prof);
	return out;
}

namespace
{
	std::unordered_set<std::string> buildExistingIwadHashSet(const std::vector<Profile>& profiles)
	{
		std::unordered_set<std::string> hashes;
		hashes.reserve(profiles.size());
		for (const auto& existing : profiles)
		{
			if (!existing.isIWAD || existing.iwadPath.empty())
				continue;
			const std::string h = calculateFileMD5(existing.iwadPath);
			if (!h.empty())
				hashes.insert(h);
		}
		return hashes;
	}
}

std::vector<DiscoveredIWadCandidate> scanUnregisteredIWADs(
	const std::vector<Profile>& existingProfiles,
	const std::vector<DiscoveredIWad>& engineIWads,
	const std::vector<std::string>& extraScanDirs,
	const std::atomic<bool>* cancelFlag)
{
	std::vector<DiscoveredIWadCandidate> discovered;

	QSet<QString> knownPaths;
	knownPaths.reserve(static_cast<int>(existingProfiles.size()));
	for (const auto& p : existingProfiles)
	{
		const std::string& primary = p.isIWAD ? p.iwadPath : p.pwadPath;
		if (!primary.empty())
			knownPaths.insert(normLower(primary));
	}

	QSet<QString> seenCandidatePaths;
	std::unordered_set<std::string> existingHashes;
	bool hashesBuilt = false;
	const auto& db = getWadDatabase();

	auto considerPath = [&](const QString& candidatePath, const std::string& engineName)
	{
		if (cancelFlag && cancelFlag->load(std::memory_order_relaxed))
			return;

		const QFileInfo fi(candidatePath);
		if (!fi.exists() || !fi.isFile())
			return;

		const QString absQ = fi.absoluteFilePath();
		const std::string absStd = stdStr(absQ);
		const QString normAbs = normLower(absStd);

		if (knownPaths.contains(normAbs) || seenCandidatePaths.contains(normAbs))
			return;
		seenCandidatePaths.insert(normAbs);

		const std::string md5 = calculateFileMD5(absStd);
		const auto dbIt = db.find(md5);
		if (dbIt != db.end() && !dbIt->second.isIWAD)
			return;

		const bool isKnownIWad = (dbIt != db.end() && dbIt->second.isIWAD);
		const QString lowerName = fi.fileName().toLower();
		const bool looksLikeIWad = !engineName.empty() || isKnownIWad ||
			lowerName.endsWith(".ipk3") ||
			lowerName.endsWith(".ipk7") ||
			isWadHeaderIWAD(absStd);

		if (!looksLikeIWad)
			return;

		if (!md5.empty())
		{
			if (!hashesBuilt)
			{
				existingHashes = buildExistingIwadHashSet(existingProfiles);
				hashesBuilt = true;
			}
			if (existingHashes.count(md5) > 0)
				return;
			existingHashes.insert(md5);
		}

		DiscoveredIWadCandidate cand;
		cand.absPath = absStd;
		cand.md5 = md5;
		if (dbIt != db.end())
		{
			cand.title = dbIt->second.title;
			cand.author = dbIt->second.author;
			cand.releaseDate = dbIt->second.releaseDate;
		}
		else
		{
			cand.title = !engineName.empty() ? engineName : stdStr(fi.completeBaseName());
			cand.author = "Unknown";
			if (fi.lastModified().isValid())
				cand.releaseDate = stdStr(fi.lastModified().date().toString("dd-MM-yyyy"));
		}
		discovered.push_back(std::move(cand));
	};

	for (const auto& w : engineIWads)
		considerPath(qStr(w.path), w.name);

	for (const auto& dirStd : extraScanDirs)
	{
		if (cancelFlag && cancelFlag->load(std::memory_order_relaxed))
			break;
		const QString dirPath = qStr(dirStd);
		if (dirPath.isEmpty() || !QFileInfo::exists(dirPath))
			continue;
		const QDir dir(dirPath);
		const QStringList entries = dir.entryList({"*.wad", "*.WAD", "*.ipk3", "*.IPK3", "*.ipk7", "*.IPK7", "*.iwad", "*.IWAD"}, QDir::Files);
		for (const QString& file : entries)
			considerPath(dir.filePath(file), {});
	}

	return discovered;
}

int commitDiscoveredIWADs(std::vector<Profile>& profiles, const std::vector<DiscoveredIWadCandidate>& candidates)
{
	if (candidates.empty())
		return 0;

	std::unordered_set<std::string> existingHashes = buildExistingIwadHashSet(profiles);
	int addedCount = 0;

	for (const auto& cand : candidates)
	{
		if (cand.absPath.empty() || hasGameFilePath(profiles, cand.absPath))
			continue;
		if (!cand.md5.empty() && existingHashes.count(cand.md5) > 0)
			continue;

		Profile prof;
		prof.title = cand.title;
		prof.author = cand.author;
		prof.releaseDate = cand.releaseDate;
		prof.isIWAD = true;
		prof.iwadPath = cand.absPath;
		createDefaultDirectoryStructure(prof);
		tryExtractCoverForProfile(prof);
		if (saveProfileToDisk(prof))
		{
			if (!cand.md5.empty())
				existingHashes.insert(cand.md5);
			profiles.push_back(std::move(prof));
			++addedCount;
		}
	}

	return addedCount;
}

int autoDetectAndImportIWADs(std::vector<Profile>& profiles)
{
	const std::vector<DiscoveredIWadCandidate> discovered = scanUnregisteredIWADs(
		profiles,
		g_discoveredEngineIWads,
		{LauncherConst::EXEC_DIR, LauncherConst::ROOT_DIR});
	return commitDiscoveredIWADs(profiles, discovered);
}
