/*
** launcherMainWindow.cpp
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

#include "launcherMainWindow.h"
#include "about.h"
#include "const.h"
#include "loader.h"
#include "profileSettings.h"

#ifdef HAS_UPDATER
#include "curl_loader.h"
#include "updater.h"
#endif

#include "findfile.h"
#include "gameconfigfile.h"
#include "gstrings.h"
#include "printf.h"
#include "resourcefile.h"
#include "version.h"
#include "versioninfo.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFrame>
#include <QHeaderView>
#include <QImageReader>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSet>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>
#include <chrono>

namespace
{
	const QSet<QString>& acceptedDropExtensions()
	{
		static const QSet<QString> exts = {
			"wad", "iwad", "pwad", "pk3", "ipk3", "pk7", "ipk7", "deh", "bex", "zip", "7z", "uzdp"
		};
		return exts;
	}

	bool hasSupportedLocalFiles(const QMimeData* mime)
	{
		if (!mime || !mime->hasUrls())
			return false;
		for (const QUrl& url : mime->urls())
		{
			if (!url.isLocalFile())
				continue;
			const QString ext = QFileInfo(url.toLocalFile()).suffix().toLower();
			if (acceptedDropExtensions().contains(ext))
				return true;
		}
		return false;
	}

	QStringList extractSupportedLocalFiles(const QMimeData* mime)
	{
		QStringList paths;
		if (!mime || !mime->hasUrls())
			return paths;
		for (const QUrl& url : mime->urls())
		{
			if (!url.isLocalFile())
				continue;
			const QString local = url.toLocalFile();
			if (acceptedDropExtensions().contains(QFileInfo(local).suffix().toLower()))
				paths << local;
		}
		return paths;
	}

	QDate parseProfileReleaseDate(const std::string& raw)
	{
		const QString s = QString::fromStdString(normalizeReleaseDate(raw)).trimmed();
		if (s.isEmpty() || s.compare("Unknown", Qt::CaseInsensitive) == 0)
			return {};
		if (QDate d = QDate::fromString(s, "dd-MM-yyyy"); d.isValid())
			return d;
		static const QRegularExpression yearRe("\\b(19\\d\\d|20\\d\\d)\\b");
		const QRegularExpressionMatch m = yearRe.match(s);
		if (m.hasMatch())
			return QDate(m.captured(1).toInt(), 1, 1);
		return {};
	}

	QDateTime parseProfileLastPlayed(const std::string& raw)
	{
		const QString s = QString::fromStdString(raw).trimmed();
		if (s.isEmpty() || s.compare("Never", Qt::CaseInsensitive) == 0)
			return {};
		return QDateTime::fromString(s, "yyyy-MM-dd HH:mm");
	}

	QString normalizePathKey(const QString& p)
	{
		const QString clean = QDir::cleanPath(p).replace('\\', '/');
#ifdef Q_OS_WIN
		return clean.toLower();
#else
		return clean;
#endif
	}

	struct StripeDef
	{
		int weight;
		const char* hex;
	};

	struct FlagDef
	{
		const char* id;
		const char* label;
		std::initializer_list<StripeDef> stripes;
	};

	const FlagDef kPrideFlags[] = {
		{ "aceflux",      "Aceflux",      { {1, "#C62253"}, {1, "#C12678"}, {1, "#C0279A"}, {1, "#A928AC"}, {1, "#8C26AE"} } },
		{ "agender",      "Agender",      { {1, "#000000"}, {1, "#BCC4C7"}, {1, "#FFFFFF"}, {1, "#B7F684"}, {1, "#FFFFFF"}, {1, "#BCC4C7"}, {1, "#000000"} } },
		{ "aroace",       "Aroace",       { {1, "#E28C00"}, {1, "#ECCD00"}, {1, "#FFFFFF"}, {1, "#62AEDC"}, {1, "#203856"} } },
		{ "aroflux",      "Aroflux",      { {1, "#E7516A"}, {1, "#D86D65"}, {1, "#B7A55D"}, {1, "#A3C95A"}, {1, "#92E454"} } },
		{ "aromantic",    "Aromantic",    { {1, "#3DA542"}, {1, "#A7D379"}, {1, "#FFFFFF"}, {1, "#A9A9A9"}, {1, "#000000"} } },
		{ "asexual",      "Asexual",      { {1, "#000000"}, {1, "#A3A3A3"}, {1, "#FFFFFF"}, {1, "#800080"} } },
		{ "bear",         "Bear",         { {1, "#613704"}, {1, "#D46300"}, {1, "#FDDC62"}, {1, "#FDE5B7"}, {1, "#FFFFFF"}, {1, "#545454"}, {1, "#000000"} } },
		{ "bisexual",     "Bisexual",     { {2, "#D60270"}, {1, "#9B4F96"}, {2, "#0038A8"} } },
		{ "demisexual",   "Demisexual",   { {5, "#FFFFFF"}, {1, "#6E0070"}, {1, "#000000"}, {1, "#6E0070"}, {5, "#D2D2D2"} } },
		{ "diversity",    "Diversity",    { {1, "#CD66FF"}, {1, "#FF6599"}, {1, "#FF0000"}, {1, "#FF9900"}, {1, "#FFFF01"}, {1, "#009A00"}, {1, "#0099CB"}, {1, "#330099"}, {1, "#990099"} } },
		{ "gay",          "Gay",          { {1, "#078D70"}, {1, "#26CEAA"}, {1, "#98E8C1"}, {1, "#FFFFFF"}, {1, "#7BADE2"}, {1, "#5049CC"}, {1, "#3D1A78"} } },
		{ "genderfluid",  "Genderfluid",  { {1, "#FF76A4"}, {1, "#FFFFFF"}, {1, "#C011D7"}, {1, "#000000"}, {1, "#2F3CBE"} } },
		{ "genderqueer",  "Genderqueer",  { {1, "#B57EDC"}, {1, "#FFFFFF"}, {1, "#4A8123"} } },
		{ "intersex",     "Intersex",     { {5, "#FFD800"}, {1, "#7902AA"}, {5, "#FFD800"} } },
		{ "lesbian",      "Lesbian",      { {1, "#D52D00"}, {1, "#EF7627"}, {1, "#FF9A56"}, {1, "#FFFFFF"}, {1, "#D162A4"}, {1, "#B55690"}, {1, "#A30262"} } },
		{ "nonbinary",    "Non-binary",   { {1, "#FCF434"}, {1, "#FFFFFF"}, {1, "#9C59D1"}, {1, "#2C2C2C"} } },
		{ "omnisexual",   "Omnisexual",   { {1, "#FE9ACE"}, {1, "#FF53BF"}, {1, "#200044"}, {1, "#6760FE"}, {1, "#8EA6FF"} } },
		{ "pansexual",    "Pansexual",    { {1, "#FF218C"}, {1, "#FFD800"}, {1, "#21B1FF"} } },
		{ "philadelphia", "Philadelphia", { {1, "#000000"}, {1, "#784F17"}, {1, "#D12229"}, {1, "#F68A1E"}, {1, "#FDE01A"}, {1, "#007940"}, {1, "#24408E"}, {1, "#732982"} } },
		{ "polysexual",   "Polysexual",   { {1, "#F714BA"}, {1, "#01D66A"}, {1, "#1594F6"} } },
		{ "queer",        "Queer",        { {1, "#000000"}, {1, "#99D9EA"}, {1, "#00A2E8"}, {1, "#B5E61D"}, {1, "#FFFFFF"}, {1, "#FFC90E"}, {1, "#FD6666"}, {1, "#FFAEC9"}, {1, "#000000"} } },
		{ "rainbow",      "Rainbow",      { {1, "#E40303"}, {1, "#FF8C00"}, {1, "#FFED00"}, {1, "#008026"}, {1, "#004CFF"}, {1, "#732982"} } },
		{ "transgender",  "Transgender",  { {1, "#5BCEFA"}, {1, "#F5A9B8"}, {1, "#FFFFFF"}, {1, "#F5A9B8"}, {1, "#5BCEFA"} } },
	};

	std::vector<QColor> resolvePrideFlagColors(const std::string& rawId)
	{
		const QString id = QString::fromStdString(rawId).trimmed().toLower();
		if (id.isEmpty())
			return {};

		const size_t flagCount = std::size(kPrideFlags);
		size_t chosen = flagCount;
		if (id == "random" || id == "list")
		{
			const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			chosen = static_cast<size_t>(nowMs) % flagCount;
			if (id == "list")
			{
				QString avail = QStringLiteral("Available flag colors: random");
				for (const auto& f : kPrideFlags)
					avail += QStringLiteral(", ") + QLatin1String(f.id);
				avail += QStringLiteral("\nSelecting: ") + QLatin1String(kPrideFlags[chosen].id);
				Printf("%s\nOpen an issue on our github if your flag is missing\n", avail.toUtf8().constData());
			}
		}
		else
		{
			for (size_t i = 0; i < flagCount; ++i)
			{
				if (id == QLatin1String(kPrideFlags[i].id))
				{
					chosen = i;
					break;
				}
			}
		}

		std::vector<QColor> colors;
		if (chosen < flagCount)
		{
			for (const auto& s : kPrideFlags[chosen].stripes)
			{
				const QColor c(QLatin1String(s.hex));
				for (int k = 0; k < s.weight; ++k)
					colors.push_back(c);
			}
			return colors;
		}

		const QStringList parts = id.split(',', Qt::KeepEmptyParts);
		for (const QString& part : parts)
		{
			bool ok = false;
			if (part.size() != 6)
			{
				colors.clear();
				break;
			}
			const uint32_t val = part.toUInt(&ok, 16);
			if (!ok || val > 0xFFFFFFu)
			{
				colors.clear();
				break;
			}
			colors.emplace_back((val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF);
		}
		if (colors.empty())
			Printf("Unknown flag '%s'\n", id.toUtf8().constData());
		return colors;
	}

	QPixmap loadEngineBannerPixmap(bool lightBanner)
	{
		const char* baseWad = BaseFileSearch(BASEWAD, nullptr, true, GameConfig);
		if (!baseWad)
			return {};
		std::unique_ptr<FileSys::FResourceFile> rf(FileSys::FResourceFile::OpenResourceFile(baseWad));
		if (!rf)
			return {};
		const int lump = rf->FindEntry(lightBanner ? "ui/banner-light.png" : "ui/banner-dark.png");
		if (lump < 0)
			return {};
		const FileSys::FileData data = rf->Read(lump);
		QPixmap pm;
		pm.loadFromData(reinterpret_cast<const uchar*>(data.data()), static_cast<uint>(data.size()));
		return pm;
	}

	QPixmap loadCustomBannerPixmap(const QString& path)
	{
		if (path.isEmpty() || !QFileInfo::exists(path))
			return {};
		QImageReader reader(path);
		reader.setAutoTransform(true);
		const QImage img = reader.read();
		return img.isNull() ? QPixmap() : QPixmap::fromImage(img);
	}

	QString buildBannerImageFilter()
	{
		QStringList patterns;
		for (const QByteArray& fmt : QImageReader::supportedImageFormats())
		{
			const QString ext = QString::fromLatin1(fmt).trimmed().toLower();
			if (!ext.isEmpty())
				patterns << (QStringLiteral("*.") + ext);
		}
		if (patterns.isEmpty())
		{
			patterns = {
				QStringLiteral("*.png"), QStringLiteral("*.jpg"), QStringLiteral("*.jpeg"),
				QStringLiteral("*.webp"), QStringLiteral("*.bmp"), QStringLiteral("*.gif"),
				QStringLiteral("*.svg"), QStringLiteral("*.tif"), QStringLiteral("*.tiff"),
				QStringLiteral("*.ico"), QStringLiteral("*.avif"), QStringLiteral("*.tga")
			};
		}
		patterns.removeDuplicates();
		return QString("%1 (%2);;%3 (*.*)")
			.arg(LStr("LAUNCHER_FILTER_IMAGES", "Image Files"),
				patterns.join(' '),
				LStr("FILEPICKER_ANY", "All Files"));
	}

	bool copyBannerToExecDir(const QString& sourcePath, QString& outDestPath)
	{
		const QFileInfo srcInfo(sourcePath);
		QString ext = srcInfo.suffix().toLower();
		if (ext.isEmpty())
			ext = QString::fromLatin1(QImageReader(sourcePath).format()).toLower();
		if (ext.isEmpty())
			ext = QStringLiteral("png");

		const QDir execDir(QString::fromStdString(LauncherConst::EXEC_DIR));
		execDir.mkpath(QStringLiteral("."));
		const QString destPath = QDir::cleanPath(execDir.filePath(QStringLiteral("banner.") + ext));
		const QString srcKey = normalizePathKey(srcInfo.absoluteFilePath());
		const QString destKey = normalizePathKey(QFileInfo(destPath).absoluteFilePath());

		auto makeWritable = [](const QString& p)
		{
			if (QFileInfo::exists(p))
				QFile::setPermissions(p, QFile::permissions(p) | QFileDevice::WriteOwner | QFileDevice::WriteUser);
		};

		if (srcKey != destKey)
		{
			const QString tmpDest = destPath + QStringLiteral(".uztmp");
			const QString bakDest = destPath + QStringLiteral(".uzbak");
			makeWritable(tmpDest);
			makeWritable(bakDest);
			QFile::remove(tmpDest);
			QFile::remove(bakDest);

			if (!QFile::copy(sourcePath, tmpDest))
				return false;

			const bool hadDest = QFileInfo::exists(destPath);
			if (hadDest)
			{
				makeWritable(destPath);
				if (!QFile::rename(destPath, bakDest))
				{
					makeWritable(tmpDest);
					QFile::remove(tmpDest);
					return false;
				}
			}

			if (!QFile::rename(tmpDest, destPath))
			{
				makeWritable(tmpDest);
				QFile::remove(tmpDest);
				if (hadDest)
					QFile::rename(bakDest, destPath);
				return false;
			}

			if (hadDest)
			{
				makeWritable(bakDest);
				QFile::remove(bakDest);
			}
		}

		QSet<QString> imageExts = {
			QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"),
			QStringLiteral("webp"), QStringLiteral("bmp"), QStringLiteral("gif"),
			QStringLiteral("svg"), QStringLiteral("tif"), QStringLiteral("tiff"),
			QStringLiteral("ico"), QStringLiteral("avif"), QStringLiteral("tga")
		};
		for (const QByteArray& fmt : QImageReader::supportedImageFormats())
			imageExts.insert(QString::fromLatin1(fmt).trimmed().toLower());

		const QFileInfoList existing = execDir.entryInfoList({QStringLiteral("banner.*")}, QDir::Files);
		for (const QFileInfo& fi : existing)
		{
			const QString otherPath = QDir::cleanPath(fi.absoluteFilePath());
			if (normalizePathKey(otherPath) == destKey)
				continue;
			if (imageExts.contains(fi.suffix().toLower()))
			{
				makeWritable(otherPath);
				QFile::remove(otherPath);
			}
		}

		outDestPath = destPath;
		return true;
	}

	class PrideBannerWidget final : public QWidget
	{
		std::vector<QColor> colors;
		float mix = 0.0f;
		bool showPlainLogo = false;
		QPixmap customImage;
		QPixmap logoLight;
		QPixmap logoDark;

	public:
		PrideBannerWidget(std::vector<QColor> stripeColors, float mixVal, bool plainLogo = false, QWidget* parent = nullptr)
			: QWidget(parent)
			, colors(std::move(stripeColors))
			, mix(mixVal)
			, showPlainLogo(plainLogo)
			, logoLight(loadEngineBannerPixmap(true))
			, logoDark(loadEngineBannerPixmap(false))
		{
			setFixedHeight(104);
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		}

		QSize sizeHint() const override { return QSize(240, 104); }
		QSize minimumSizeHint() const override { return QSize(0, 104); }

		void setBanner(std::vector<QColor> stripeColors, bool plainLogo = false, QPixmap customBanner = {})
		{
			colors = std::move(stripeColors);
			showPlainLogo = plainLogo;
			customImage = std::move(customBanner);
			const bool shouldShow = showPlainLogo || !colors.empty() || !customImage.isNull();
			if (isHidden() == shouldShow)
				setVisible(shouldShow);
			updateGeometry();
			update();
		}

	protected:
		void paintEvent(QPaintEvent*) override
		{
			QPainter p(this);
			const int W = width();
			const int H = height();
			if (W <= 0 || H <= 0)
				return;

			if (!customImage.isNull())
			{
				const qreal dpr = std::max<qreal>(1.0, devicePixelRatioF());
				const QSize targetPhys(std::max(1, qRound(W * dpr)), std::max(1, qRound(H * dpr)));
				QPixmap scaled = customImage.scaled(targetPhys, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
				const int sx = std::max(0, (scaled.width() - targetPhys.width()) / 2);
				const int sy = std::max(0, (scaled.height() - targetPhys.height()) / 2);
				QPixmap cropped = scaled.copy(sx, sy, targetPhys.width(), targetPhys.height());
				cropped.setDevicePixelRatio(dpr);
				p.drawPixmap(0, 0, cropped);
				return;
			}

			if (!showPlainLogo && colors.empty())
				return;

			const bool positiveMix = (mix > 0.0f);
			const float m = std::clamp(std::abs(mix), 0.0f, 1.0f);
			const QColor bg = palette().color(QPalette::Window);
			const QColor base = positiveMix ? QColor(255, 255, 255) : bg;
			const bool useLightLogo = (!colors.empty() && positiveMix) || bg.lightness() > 127;
			const QPixmap& logo = useLightLogo ? logoLight : logoDark;

			const int logoW = !logo.isNull()
				? std::min(W, static_cast<int>(std::round(static_cast<double>(logo.width()) * H / std::max(1, logo.height()))))
				: 0;
			const int logoX = (W - logoW) / 2;

			if (!colors.empty())
			{
				const double stripeH = static_cast<double>(H) / static_cast<double>(colors.size());
				for (size_t i = 0; i < colors.size(); ++i)
				{
					const int y0 = static_cast<int>(std::floor(stripeH * static_cast<double>(i)));
					const int y1 = static_cast<int>(std::ceil(stripeH * static_cast<double>(i + 1)));
					const QColor& c = colors[i];
					p.fillRect(0, y0, W, std::max(1, y1 - y0), c);

					if (logoW > 0)
					{
						const QColor mixed(
							static_cast<int>(std::round(base.red() * (1.0f - m) + c.red() * m)),
							static_cast<int>(std::round(base.green() * (1.0f - m) + c.green() * m)),
							static_cast<int>(std::round(base.blue() * (1.0f - m) + c.blue() * m)));
						p.fillRect(logoX, y0, logoW, std::max(1, y1 - y0), mixed);
					}
				}
			}

			if (!logo.isNull() && logoW > 0)
			{
				p.setRenderHint(QPainter::SmoothPixmapTransform, true);
				p.drawPixmap(QRect(logoX, 0, logoW, H), logo);
			}
		}
	};
}

LauncherMainWindow::LauncherMainWindow(
	const LauncherConfig& initialConfig,
	std::vector<Profile> initialProfiles,
	bool showUpcomingNotes,
	const std::string& prideColors,
	float prideMix,
	QWidget* parent)
	: QMainWindow(parent)
	, launcherConfig(initialConfig)
	, profiles(std::move(initialProfiles))
	, showUpcomingReleaseNotes(showUpcomingNotes)
	, prideBannerColors(prideColors)
	, prideBannerMix(prideMix)
{
	setAcceptDrops(true);
	resize(960, 660);
	setMinimumSize(780, 520);

	if (prideBannerColors.empty() && !launcherConfig.banner.empty())
		prideBannerColors = launcherConfig.banner;
	buildUi();
	rebuildMenus();
	retranslateUi();

	const int initialRow = profiles.empty()
		? -1
		: std::clamp(launcherConfig.lastSelectedRow, 0, static_cast<int>(profiles.size()) - 1);
	refreshTable(initialRow);
}

LauncherMainWindow::~LauncherMainWindow()
{
#ifdef HAS_UPDATER
	if (updater)
	{
		updater->cancelPending();
		updater.reset();
	}
#endif

	if (detectCancel)
		detectCancel->store(true, std::memory_order_relaxed);
	if (detectThread)
	{
		QThread* t = detectThread.data();
		detectThread = nullptr;
		t->disconnect();
		t->wait();
		delete t;
	}

	if (gameProcess && gameProcess->state() != QProcess::NotRunning)
	{
		recordRunningSessionPlaytime();
		gameProcess->disconnect(this);
		gameProcess->kill();
		gameProcess->waitForFinished(1500);
		appendLaunchLogFooter(QStringLiteral("=== Process terminated by launcher ==="));
	}
}

void LauncherMainWindow::buildUi()
{
	QMenuBar* mb = menuBar();
	mb->setNativeMenuBar(false);
	addMenu = mb->addMenu(QString());
	prefMenu = mb->addMenu(QString());
	actAbout = mb->addAction(QString(), this, &LauncherMainWindow::onAboutClicked);

	QWidget* searchContainer = new QWidget(mb);
	QHBoxLayout* searchLayout = new QHBoxLayout(searchContainer);
	searchLayout->setContentsMargins(4, 2, 6, 2);
	searchLayout->setSpacing(6);

	filterCategoryCombo = new QComboBox(searchContainer);
	filterCategoryCombo->setMinimumHeight(24);
	filterCategoryCombo->addItem(QString(), 0);
	filterCategoryCombo->addItem(QString(), 1);
	filterCategoryCombo->addItem(QString(), 2);
	filterCategoryCombo->addItem(QString(), 3);
	connect(filterCategoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LauncherMainWindow::onFilterCategoryChanged);

	searchEdit = new QLineEdit(searchContainer);
	searchEdit->setClearButtonEnabled(true);
	searchEdit->setFixedWidth(220);
	searchEdit->setMinimumHeight(24);
	connect(searchEdit, &QLineEdit::textChanged, this, &LauncherMainWindow::onFilterTextChanged);

	searchLayout->addWidget(filterCategoryCombo);
	searchLayout->addWidget(searchEdit);
	mb->setCornerWidget(searchContainer, Qt::TopRightCorner);

	QWidget* central = new QWidget(this);
	QVBoxLayout* rootLayout = new QVBoxLayout(central);
	rootLayout->setContentsMargins(10, 8, 10, 8);
	rootLayout->setSpacing(8);

	prideBannerWidget = new PrideBannerWidget({}, prideBannerMix, false, central);
	rootLayout->addWidget(prideBannerWidget);

	QHBoxLayout* centerRow = new QHBoxLayout();
	centerRow->setContentsMargins(0, 0, 0, 4);
	centerRow->setSpacing(10);

	table = new QTableWidget(0, 7, central);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setAlternatingRowColors(true);
	table->setShowGrid(false);
	table->setContextMenuPolicy(Qt::CustomContextMenu);
	table->verticalHeader()->setVisible(false);
	table->verticalHeader()->setDefaultSectionSize(28);
	table->horizontalHeader()->setStretchLastSection(false);
	table->horizontalHeader()->setMinimumSectionSize(28);
	table->horizontalHeader()->setHighlightSections(false);
	table->horizontalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	table->setColumnWidth(0, 32);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

	connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &LauncherMainWindow::onHeaderSectionClicked);
	connect(table, &QTableWidget::cellClicked, this, &LauncherMainWindow::onTableCellClicked);
	connect(table, &QTableWidget::itemSelectionChanged, this, &LauncherMainWindow::onSelectionChanged);
	connect(table, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem* item) {
		if (item && item->column() == 0)
			return;
		onPlayClicked();
	});
	connect(table, &QTableWidget::customContextMenuRequested, this, &LauncherMainWindow::onTableContextMenu);
	table->installEventFilter(this);
	centerRow->addWidget(table, 1);

	QWidget* rightButtonsWidget = new QWidget(central);
	rightButtonsWidget->setFixedWidth(182);
	QVBoxLayout* rightCol = new QVBoxLayout(rightButtonsWidget);
	rightCol->setContentsMargins(0, 0, 0, 0);
	rightCol->setSpacing(8);

	btnPlay = new QPushButton(rightButtonsWidget);
	btnPlay->setMinimumHeight(44);
	QFont playFont = btnPlay->font();
	playFont.setPointSize(playFont.pointSize() + 1);
	playFont.setBold(true);
	btnPlay->setFont(playFont);
	btnPlay->setProperty("launcherRole", "primary");
	connect(btnPlay, &QPushButton::clicked, this, &LauncherMainWindow::onPlayClicked);

	btnJoin = new QPushButton(rightButtonsWidget);
	btnJoin->setMinimumHeight(34);
	connect(btnJoin, &QPushButton::clicked, this, &LauncherMainWindow::onJoinClicked);

	btnHost = new QPushButton(rightButtonsWidget);
	btnHost->setMinimumHeight(34);
	connect(btnHost, &QPushButton::clicked, this, &LauncherMainWindow::onHostClicked);

	btnSettings = new QPushButton(rightButtonsWidget);
	btnSettings->setMinimumHeight(34);
	connect(btnSettings, &QPushButton::clicked, this, &LauncherMainWindow::onProfileSettingsClicked);

	btnMoveUp = new QPushButton(rightButtonsWidget);
	btnMoveUp->setMinimumHeight(30);
	connect(btnMoveUp, &QPushButton::clicked, this, &LauncherMainWindow::onMoveUpClicked);

	btnMoveDown = new QPushButton(rightButtonsWidget);
	btnMoveDown->setMinimumHeight(30);
	connect(btnMoveDown, &QPushButton::clicked, this, &LauncherMainWindow::onMoveDownClicked);

	btnRefresh = new QPushButton(rightButtonsWidget);
	btnRefresh->setMinimumHeight(30);
	connect(btnRefresh, &QPushButton::clicked, this, &LauncherMainWindow::onRefreshProfiles);

	rightCol->addWidget(btnPlay);
	rightCol->addWidget(btnJoin);
	rightCol->addWidget(btnHost);
	rightCol->addWidget(btnSettings);
	rightCol->addSpacing(8);
	rightCol->addWidget(btnMoveUp);
	rightCol->addWidget(btnMoveDown);
	rightCol->addStretch();
	rightCol->addWidget(btnRefresh);

	centerRow->addWidget(rightButtonsWidget);

	QWidget* bottomRowWidget = new QWidget(central);
	QHBoxLayout* bottomRow = new QHBoxLayout(bottomRowWidget);
	bottomRow->setContentsMargins(0, 4, 0, 0);
	bottomRow->setSpacing(10);

	coverLabel = new QLabel(bottomRowWidget);
	coverLabel->setObjectName("launcherCoverLabel");
	coverLabel->setFixedWidth(240);
	coverLabel->setMinimumHeight(120);
	coverLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
	coverLabel->setContentsMargins(0, 0, 0, 0);
	coverLabel->setFrameShape(QFrame::StyledPanel);
	coverLabel->setFrameShadow(QFrame::Sunken);
	coverLabel->setAlignment(Qt::AlignCenter);
	coverLabel->setWordWrap(true);
	coverLabel->installEventFilter(this);

	descView = new QTextEdit(bottomRowWidget);
	descView->setReadOnly(true);
	descView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	bottomRow->addWidget(coverLabel);
	bottomRow->addWidget(descView, 1);

	QWidget* centerContainer = new QWidget(central);
	centerContainer->setLayout(centerRow);

	verticalSplitter = new QSplitter(Qt::Vertical, central);
	verticalSplitter->setChildrenCollapsible(false);
	verticalSplitter->addWidget(centerContainer);
	verticalSplitter->addWidget(bottomRowWidget);
	verticalSplitter->setStretchFactor(0, 3);
	verticalSplitter->setStretchFactor(1, 1);
	if (launcherConfig.splitterTopSize > 0 && launcherConfig.splitterBottomSize > 0)
		verticalSplitter->setSizes({launcherConfig.splitterTopSize, launcherConfig.splitterBottomSize});
	else
		verticalSplitter->setSizes({360, 165});

	auto* scFind = new QShortcut(QKeySequence::Find, this);
	connect(scFind, &QShortcut::activated, this, [this]() {
		searchEdit->setFocus();
		searchEdit->selectAll();
	});
	auto* scRefresh = new QShortcut(QKeySequence::Refresh, this);
	connect(scRefresh, &QShortcut::activated, this, &LauncherMainWindow::onRefreshProfiles);
	auto* scSettings = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this);
	connect(scSettings, &QShortcut::activated, this, &LauncherMainWindow::onProfileSettingsClicked);

#ifdef HAS_UPDATER
	if (IsCurlLoaded())
	{
		updater = std::make_unique<LauncherUpdater>(this, [this]() {
			updateUpdaterMenuState();
		});
		rootLayout->addWidget(updater->bannerWidget());
	}
#endif

	rootLayout->addWidget(verticalSplitter, 1);

	setCentralWidget(central);
	applyBannerSelection(prideBannerColors);

	statusCountLabel = new QLabel(this);
	statusRunLabel = new QLabel(this);
	statusBar()->addWidget(statusCountLabel, 1);
	statusBar()->addPermanentWidget(statusRunLabel);
}

void LauncherMainWindow::rebuildMenus()
{
	addMenu->clear();
	actAddFile = addMenu->addAction(QString(), this, &LauncherMainWindow::onAddGameFiles);
	actCreateBlank = addMenu->addAction(QString(), this, &LauncherMainWindow::onCreateBlankProfile);
	addMenu->addSeparator();
	actOpenLauncherDir = addMenu->addAction(QString(), this, &LauncherMainWindow::onOpenLauncherFolder);
	actOpenExecDir = addMenu->addAction(QString(), this, &LauncherMainWindow::onOpenExecFolder);
	addMenu->addSeparator();
	actQuit = addMenu->addAction(QString(), this, &QWidget::close);
#ifdef Q_OS_WIN
	actQuit->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_W), QKeySequence(Qt::CTRL | Qt::Key_Q)});
#else
	{
		QKeySequence quitSeq = QKeySequence(QKeySequence::Quit);
		if (quitSeq.isEmpty())
			quitSeq = QKeySequence(Qt::CTRL | Qt::Key_Q);
		actQuit->setShortcut(quitSeq);
	}
#endif
	actQuit->setShortcutContext(Qt::WindowShortcut);

	prefMenu->clear();
	langMenu = prefMenu->addMenu(QString());
	langGroup = new QActionGroup(this);
	langGroup->setExclusive(true);
	connect(langGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onLanguageTriggered);

	for (const LauncherLanguage& lang : getLauncherLanguages())
	{
		QAction* act = langMenu->addAction(lang.name);
		act->setCheckable(true);
		act->setData(lang.code);
		if (lang.code.compare(QString::fromStdString(launcherConfig.language), Qt::CaseInsensitive) == 0)
			act->setChecked(true);
		langGroup->addAction(act);
	}

	themeMenu = prefMenu->addMenu(QString());
	themeGroup = new QActionGroup(this);
	themeGroup->setExclusive(true);
	connect(themeGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onThemeTriggered);

	const struct { const char* id; const char* key; const char* fallback; } themes[] = {
		{"auto", "OPTVAL_AUTO", "Auto"},
		{"dark", "OPTVAL_DARK", "Dark"},
		{"light", "OPTVAL_LIGHT", "Light"},
		{"highcontrast", "PICKER_THEME_HIGHCONTRAST", "High Contrast"},
	};
	const std::string curTheme = themeToName(parseThemeName(launcherConfig.theme));
	for (const auto& t : themes)
	{
		QAction* act = themeMenu->addAction(LStr(t.key, t.fallback));
		act->setCheckable(true);
		act->setData(QString::fromLatin1(t.id));
		if (curTheme == t.id)
			act->setChecked(true);
		themeGroup->addAction(act);
	}

	bannerMenu = prefMenu->addMenu(QString());
	bannerGroup = new QActionGroup(this);
	bannerGroup->setExclusive(true);
	connect(bannerGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onBannerTriggered);

	auto addBannerOption = [&](const QString& label, const QString& id)
	{
		QAction* act = bannerMenu->addAction(label);
		act->setCheckable(true);
		act->setData(id);
		bannerGroup->addAction(act);
		return act;
	};

	addBannerOption(LStr("OPTVAL_NONE", "None"), QString());
	addBannerOption(LStr("OPTVAL_DEFAULT", "Default"), QStringLiteral("default"));
	addBannerOption(LStr("OPTVAL_RANDOM", "Random"), QStringLiteral("random"));
	bannerMenu->addSeparator();

	for (const auto& f : kPrideFlags)
		addBannerOption(QLatin1String(f.label), QLatin1String(f.id));

	bannerMenu->addSeparator();
	QAction* actCustomBanner = addBannerOption(
		LStr("LAUNCHER_BANNER_CUSTOM_IMAGE", "Custom Banner Image") + QStringLiteral("..."),
		QStringLiteral("__custom__"));
	actCustomBanner->setToolTip(
		LStr("LAUNCHER_BANNER_CUSTOM_TIP", "Recommended aspect ratio: ~9:1 (e.g. 940x104 px at default width; wider when maximized)."));

	syncBannerMenuState();

	prefMenu->addSeparator();
	actSetDefaultModsDir = prefMenu->addAction(QString(), this, &LauncherMainWindow::onSetDefaultModsDir);
	actClearDefaultModsDir = prefMenu->addAction(QString(), this, &LauncherMainWindow::onClearDefaultModsDir);
	updateDefaultModsMenuState();

	prefMenu->addSeparator();
	actHideWhilePlaying = prefMenu->addAction(QString());
	actHideWhilePlaying->setCheckable(true);
	actHideWhilePlaying->setChecked(launcherConfig.hideWhilePlaying);
	connect(actHideWhilePlaying, &QAction::toggled, this, &LauncherMainWindow::onToggleHideWhilePlaying);

	actDontAskAgain = prefMenu->addAction(QString());
	actDontAskAgain->setCheckable(true);
	actDontAskAgain->setChecked(launcherConfig.dontAskAgain);
	connect(actDontAskAgain, &QAction::toggled, this, &LauncherMainWindow::onToggleDontAskAgain);

#ifdef HAS_UPDATER
	if (IsCurlLoaded())
	{
		prefMenu->addSeparator();

		updaterModeMenu = prefMenu->addMenu(QString());
		updaterModeGroup = new QActionGroup(this);
		updaterModeGroup->setExclusive(true);
		connect(updaterModeGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onUpdaterModeTriggered);

		const int modes[] = {
			static_cast<int>(UpdaterMode::Disabled),
			static_cast<int>(UpdaterMode::Notify),
			static_cast<int>(UpdaterMode::PromptToInstall)
		};
		for (int modeVal : modes)
		{
			QAction* act = updaterModeMenu->addAction(QString());
			act->setCheckable(true);
			act->setData(modeVal);
			updaterModeGroup->addAction(act);
		}

		updaterIntervalMenu = prefMenu->addMenu(QString());
		updaterIntervalGroup = new QActionGroup(this);
		updaterIntervalGroup->setExclusive(true);
		connect(updaterIntervalGroup, &QActionGroup::triggered, this, &LauncherMainWindow::onUpdaterIntervalTriggered);

		const int intervals[] = {1, 7, 30};
		for (int days : intervals)
		{
			QAction* act = updaterIntervalMenu->addAction(QString());
			act->setCheckable(true);
			act->setData(days);
			updaterIntervalGroup->addAction(act);
		}

		updateUpdaterMenuState();
	}
#endif
}

void LauncherMainWindow::retranslateUi()
{
	const QString rawGitTime = QString::fromUtf8(GetGitTime());
	const QDateTime parsedDt = QDateTime::fromString(rawGitTime, Qt::ISODate);
	const QString formattedDate = parsedDt.isValid() ? parsedDt.date().toString("dd-MM-yyyy") : rawGitTime;
	setWindowTitle(LStr("LAUNCHER_WINDOW_TITLE", "%1 (Version %2) [Build Date: %3]").arg(GAMENAME).arg(GetVersionString()).arg(formattedDate));

	addMenu->setTitle(LStr("LAUNCHER_ADD_ENTRY", "New Entry"));
	prefMenu->setTitle(LStr("OPTMNU_TITLE", "Options"));
	actAbout->setText(LStr("MNU_INFO", "Info"));
	searchEdit->setPlaceholderText(LStr("LAUNCHER_SEARCH_PLACEHOLDER", "Filter profiles..."));
	searchEdit->setToolTip(LStr("LAUNCHER_SEARCH_TIP", "Search by title, author, description, IWAD, or mod filenames (space-separated terms)."));
	if (filterCategoryCombo && filterCategoryCombo->count() >= 4)
	{
		filterCategoryCombo->setItemText(0, LStr("OPTVAL_ALL", "All"));
		filterCategoryCombo->setItemText(1, QString::fromUtf8("\u2605 ") + LStr("LAUNCHER_FILTER_FAVORITES", "Favorites"));
		filterCategoryCombo->setItemText(2, QStringLiteral("IWADs"));
		filterCategoryCombo->setItemText(3, QStringLiteral("(P)WADs"));
	}

	actAddFile->setText(LStr("LAUNCHER_MENU_ADD_FILE", "Add WAD / Mod / Archive") + QStringLiteral("..."));
	actCreateBlank->setText(LStr("LAUNCHER_MENU_NEW_BLANK", "Create Blank Profile") + QStringLiteral("..."));
	actOpenLauncherDir->setText(LStr("LAUNCHER_MENU_OPEN_LAUNCHER_DIR", "Open Launcher Data Folder"));
	actOpenExecDir->setText(LStr("LAUNCHER_MENU_OPEN_APP_DIR", "Open Application Folder"));
	actQuit->setText(LStr("PICKER_EXIT", "Exit"));

	langMenu->setTitle(LStr("OPTMNU_LANGUAGE", "Language"));
	themeMenu->setTitle(LStr("PICKER_THEME", "Theme"));
	if (actSetDefaultModsDir)
		actSetDefaultModsDir->setText(LStr("LAUNCHER_PREF_SET_DEFAULT_MODS", "Set Default Mods Folder") + QStringLiteral("..."));
	if (actClearDefaultModsDir)
		actClearDefaultModsDir->setText(LStr("LAUNCHER_PREF_CLEAR_DEFAULT_MODS", "Clear Default Mods Folder"));
	updateDefaultModsMenuState();
	actHideWhilePlaying->setText(LStr("LAUNCHER_PREF_HIDE_ON_PLAY", "Hide Launcher While Playing"));
	if (actDontAskAgain)
	{
		actDontAskAgain->setText(LStr("PICKER_DONTASK", "Don't ask me this again"));
		actDontAskAgain->setToolTip(LStr("LAUNCHER_DONTASK_TIP", "Skip the launcher on startup and launch directly. Hold Shift or pass -showlauncher to open the launcher again."));
	}

	const QList<QAction*> themeActions = themeGroup->actions();
	const char* const themeKeys[] = {"OPTVAL_AUTO", "OPTVAL_DARK", "OPTVAL_LIGHT", "PICKER_THEME_HIGHCONTRAST"};
	const char* const themeFallbacks[] = {"Auto", "Dark", "Light", "High Contrast"};
	for (int i = 0; i < themeActions.size() && i < 4; ++i)
		themeActions[i]->setText(LStr(themeKeys[i], themeFallbacks[i]));

	if (bannerMenu && bannerGroup)
	{
		bannerMenu->setTitle(LStr("LAUNCHER_BANNER", "Banner"));
		for (QAction* act : bannerGroup->actions())
		{
			const QString id = act->data().toString();
			if (id.isEmpty())
				act->setText(LStr("OPTVAL_NONE", "None"));
			else if (id == QStringLiteral("default"))
				act->setText(LStr("OPTVAL_DEFAULT", "Default"));
			else if (id == QStringLiteral("random"))
				act->setText(LStr("OPTVAL_RANDOM", "Random"));
			else if (id == QStringLiteral("__custom__"))
			{
				act->setText(LStr("LAUNCHER_BANNER_CUSTOM_IMAGE", "Custom Banner Image") + QStringLiteral("..."));
				act->setToolTip(LStr("LAUNCHER_BANNER_CUSTOM_TIP", "Recommended aspect ratio: ~9:1 (e.g. 940x104 px at default width; wider when maximized)."));
			}
		}
	}

#ifdef HAS_UPDATER
	if (updaterModeMenu && updaterModeGroup)
	{
		updaterModeMenu->setTitle(LStr("UPDATER_SETTINGS", "Updater Settings"));
		const QList<QAction*> modeActions = updaterModeGroup->actions();
		const char* const modeKeys[] = {"OPTVAL_DISABLE", "OPTVAL_NOTIFY", "UPDATER_PROMPT_TO_INSTALL"};
		const char* const modeFallbacks[] = {"Disable", "Notify", "Prompt to install"};
		for (int i = 0; i < modeActions.size() && i < 3; ++i)
			modeActions[i]->setText(LStr(modeKeys[i], modeFallbacks[i]));
	}
	if (updaterIntervalMenu && updaterIntervalGroup)
	{
		updaterIntervalMenu->setTitle(stripTrailingColon(LStr("UPDATER_INTERVAL", "Check every:")));
		const QList<QAction*> intervalActions = updaterIntervalGroup->actions();
		const char* const intervalKeys[] = {"OPTVAL_DAILY", "OPTVAL_WEEKLY", "OPTVAL_MONTHLY"};
		const char* const intervalFallbacks[] = {"Daily", "Weekly", "Monthly"};
		for (int i = 0; i < intervalActions.size() && i < 3; ++i)
			intervalActions[i]->setText(LStr(intervalKeys[i], intervalFallbacks[i]));
	}
	if (updater)
		updater->retranslateUi();
#endif

	table->setHorizontalHeaderLabels({
		QString::fromUtf8("\u2605"),
		LStr("LAUNCHER_COL_TYPE", "Type"),
		LStr("LAUNCHER_COL_TITLE", "Title"),
		LStr("LAUNCHER_COL_AUTHOR", "Author(s)"),
		LStr("LAUNCHER_COL_RELEASE", "Release Date"),
		LStr("LAUNCHER_COL_LAST_PLAYED", "Last Played"),
		LStr("LAUNCHER_COL_PLAYTIME", "Playtime")
	});
	for (int col = 0; col < 7; ++col)
	{
		if (QTableWidgetItem* hItem = table->horizontalHeaderItem(col))
		{
			hItem->setTextAlignment((col == 2 || col == 3) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
			if (col == 0)
				hItem->setToolTip(LStr("LAUNCHER_SORT_RESET_TIP", "Pinned favorites stay at the top. Click to reset column sort to custom order."));
		}
	}
	syncSortIndicator();

	btnPlay->setText(LStr("PICKER_PLAY", "Play Game"));
	btnJoin->setText(LStr("PICKER_PLAYJOIN", "Join Game"));
	btnHost->setText(LStr("PICKER_PLAYHOST", "Host Game"));
	btnSettings->setText(LStr("LAUNCHER_PROFILE_SETTINGS", "Profile Settings") + QStringLiteral("..."));
	btnMoveUp->setText(LStr("LAUNCHER_MOVE_UP", "Move Up"));
	btnMoveDown->setText(LStr("LAUNCHER_MOVE_DOWN", "Move Down"));
	btnRefresh->setText(LStr("LAUNCHER_MENU_REFRESH", "Refresh Profiles"));

	for (int r = 0; r < table->rowCount(); ++r)
	{
		const int idx = profileIndexForTableRow(r);
		if (idx >= 0)
			updateTableRow(r, idx);
	}
	applyFilter();
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::syncSortIndicator()
{
	const bool sorted = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6);
	table->horizontalHeader()->setSortIndicatorShown(sorted);
	if (sorted)
	{
		table->horizontalHeader()->setSortIndicator(
			launcherConfig.sortColumn,
			launcherConfig.sortDescending ? Qt::DescendingOrder : Qt::AscendingOrder);
	}
}

std::vector<int> LauncherMainWindow::buildDisplayOrder() const
{
	std::vector<int> order(profiles.size());
	for (size_t i = 0; i < profiles.size(); ++i)
		order[i] = static_cast<int>(i);

	const int sortCol = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6)
		? launcherConfig.sortColumn
		: -1;
	const bool desc = launcherConfig.sortDescending;

	std::sort(order.begin(), order.end(), [&](int idxA, int idxB)
	{
		const Profile& a = profiles[idxA];
		const Profile& b = profiles[idxB];

		if (a.favorite != b.favorite)
			return a.favorite;

		if (sortCol < 0)
			return idxA < idxB;

		auto cmpTitle = [&]() -> int
		{
			return QString::fromStdString(a.title).compare(QString::fromStdString(b.title), Qt::CaseInsensitive);
		};

		int cmp = 0;
		switch (sortCol)
		{
		case 1: // Type
			if (a.isIWAD != b.isIWAD)
				cmp = a.isIWAD ? -1 : 1;
			else
				cmp = cmpTitle();
			break;
		case 2: // Title
			cmp = cmpTitle();
			break;
		case 3: // Author
			cmp = QString::fromStdString(a.author).compare(QString::fromStdString(b.author), Qt::CaseInsensitive);
			if (cmp == 0)
				cmp = cmpTitle();
			break;
		case 4: // Release Date
		{
			const QDate da = parseProfileReleaseDate(a.releaseDate);
			const QDate db = parseProfileReleaseDate(b.releaseDate);
			if (da.isValid() != db.isValid())
				return da.isValid();
			if (da.isValid() && db.isValid())
				cmp = (da < db) ? -1 : ((da > db) ? 1 : cmpTitle());
			else
				cmp = cmpTitle();
			break;
		}
		case 5: // Last Played
		{
			const QDateTime ta = parseProfileLastPlayed(a.lastPlayed);
			const QDateTime tb = parseProfileLastPlayed(b.lastPlayed);
			if (ta.isValid() != tb.isValid())
				return ta.isValid();
			if (ta.isValid() && tb.isValid())
				cmp = (ta < tb) ? -1 : ((ta > tb) ? 1 : cmpTitle());
			else
				cmp = cmpTitle();
			break;
		}
		case 6: // Playtime
			if (a.playtime != b.playtime)
				cmp = (a.playtime < b.playtime) ? -1 : 1;
			else
				cmp = cmpTitle();
			break;
		default:
			break;
		}

		if (cmp == 0)
			return idxA < idxB;
		return desc ? (cmp > 0) : (cmp < 0);
	});

	return order;
}

int LauncherMainWindow::profileIndexForTableRow(int tableRow) const
{
	if (tableRow < 0 || tableRow >= table->rowCount())
		return -1;
	const QTableWidgetItem* item = table->item(tableRow, 0);
	if (!item)
		return -1;
	const int idx = item->data(Qt::UserRole).toInt();
	if (idx < 0 || idx >= static_cast<int>(profiles.size()))
		return -1;
	return idx;
}

int LauncherMainWindow::tableRowForProfileIndex(int profileIdx) const
{
	if (profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return -1;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		if (profileIndexForTableRow(r) == profileIdx)
			return r;
	}
	return -1;
}

bool LauncherMainWindow::canMoveSelectedBy(int delta) const
{
	if (gameProcess != nullptr)
		return false;
	if (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6)
		return false;
	if (!searchEdit->text().trimmed().isEmpty())
		return false;
	if (filterCategoryCombo && filterCategoryCombo->currentData().toInt() != 0)
		return false;

	const int curRow = tableRowForProfileIndex(selectedProfileIndex());
	if (curRow < 0 || table->isRowHidden(curRow))
		return false;
	const int targetRow = curRow + delta;
	if (targetRow < 0 || targetRow >= table->rowCount() || table->isRowHidden(targetRow))
		return false;

	const int idxA = profileIndexForTableRow(curRow);
	const int idxB = profileIndexForTableRow(targetRow);
	if (idxA < 0 || idxB < 0)
		return false;
	return profiles[idxA].favorite == profiles[idxB].favorite;
}

void LauncherMainWindow::refreshTable(int selectProfileIndexValue)
{
	const std::vector<int> order = buildDisplayOrder();
	table->blockSignals(true);
	table->setRowCount(static_cast<int>(order.size()));
	for (int r = 0; r < static_cast<int>(order.size()); ++r)
		updateTableRow(r, order[r]);
	table->blockSignals(false);
	syncSortIndicator();

	applyFilter();

	if (selectProfileIndexValue >= 0 && selectProfileIndexValue < static_cast<int>(profiles.size()))
		selectProfileIndex(selectProfileIndexValue);
	else if (!order.empty())
		selectProfileIndex(order.front());
	else
	{
		updateDetailsPane();
		updateButtonStates();
	}
}

void LauncherMainWindow::updateTableRow(int tableRow, int profileIdx)
{
	if (tableRow < 0 || tableRow >= table->rowCount() || profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return;

	const Profile& p = profiles[profileIdx];
	auto makeItem = [&](const QString& text, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter)
	{
		auto* item = new QTableWidgetItem(text);
		item->setData(Qt::UserRole, profileIdx);
		item->setTextAlignment(align);
		return item;
	};

	auto* starItem = makeItem(p.favorite ? QString::fromUtf8("\u2605") : QString::fromUtf8("\u2606"), Qt::AlignCenter);
	const LauncherTheme theme = parseThemeName(launcherConfig.theme);
	if (p.favorite)
	{
		const QColor starColor = (theme == LauncherTheme::HighContrast)
			? QColor(255, 255, 0)
			: (isDarkThemeActive(theme) ? QColor(245, 197, 66) : QColor(214, 148, 0));
		starItem->setForeground(starColor);
		starItem->setToolTip(LStr("LAUNCHER_UNPIN_FAVORITE", "Unpin Favorite"));
	}
	else
	{
		starItem->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
		starItem->setToolTip(LStr("LAUNCHER_PIN_FAVORITE", "Pin to Top (Favorite)"));
	}

	table->setItem(tableRow, 0, starItem);
	table->setItem(tableRow, 1, makeItem(p.isIWAD ? "IWAD" : "(P)WAD", Qt::AlignCenter));
	table->setItem(tableRow, 2, makeItem(QString::fromStdString(p.title)));
	const QString authorText = (p.author.empty() || QString::fromStdString(p.author).compare("Unknown", Qt::CaseInsensitive) == 0)
		? LStr("TXT_UNKNOWN", "Unknown")
		: QString::fromStdString(p.author);
	table->setItem(tableRow, 3, makeItem(authorText));
	const QString releaseText = (p.releaseDate.empty() || QString::fromStdString(p.releaseDate).compare("Unknown", Qt::CaseInsensitive) == 0)
		? LStr("TXT_UNKNOWN", "Unknown")
		: QString::fromStdString(p.releaseDate);
	table->setItem(tableRow, 4, makeItem(releaseText, Qt::AlignCenter));
	const QString lastPlayedText = (p.lastPlayed.empty() || p.lastPlayed == "Never")
		? LStr("OPTVAL_NEVER", "Never")
		: QString::fromStdString(p.lastPlayed);
	table->setItem(tableRow, 5, makeItem(lastPlayedText, Qt::AlignCenter));
	table->setItem(tableRow, 6, makeItem(formatPlaytime(p.playtime), Qt::AlignCenter));
}

int LauncherMainWindow::selectedProfileIndex() const
{
	const QModelIndexList sel = table->selectionModel() ? table->selectionModel()->selectedRows() : QModelIndexList{};
	if (sel.isEmpty())
		return -1;
	const int row = sel.first().row();
	if (row < 0 || table->isRowHidden(row))
		return -1;
	return profileIndexForTableRow(row);
}

void LauncherMainWindow::selectProfileIndex(int profileIdx)
{
	if (table->rowCount() <= 0)
		return;

	int targetRow = tableRowForProfileIndex(profileIdx);
	if (targetRow < 0 || table->isRowHidden(targetRow))
	{
		targetRow = -1;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			if (!table->isRowHidden(r))
			{
				targetRow = r;
				break;
			}
		}
	}

	if (targetRow >= 0)
	{
		table->selectRow(targetRow);
		if (const auto* item = table->item(targetRow, 0))
			table->scrollToItem(item);
	}
	else
	{
		table->clearSelection();
		table->setCurrentCell(-1, -1);
	}
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::applyFilter()
{
	const QString filter = searchEdit->text().trimmed();
	const QStringList terms = filter.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	const int category = filterCategoryCombo ? filterCategoryCombo->currentData().toInt() : 0;
	int visibleCount = 0;

	for (int r = 0; r < table->rowCount(); ++r)
	{
		const int idx = profileIndexForTableRow(r);
		if (idx < 0)
		{
			table->setRowHidden(r, true);
			continue;
		}
		const Profile& p = profiles[idx];

		bool match = true;
		if (category == 1)
			match = p.favorite;
		else if (category == 2)
			match = p.isIWAD;
		else if (category == 3)
			match = !p.isIWAD;

		if (match && !terms.isEmpty())
		{
			const QString title = QString::fromStdString(p.title);
			const QString author = QString::fromStdString(p.author);
			const QString desc = QString::fromStdString(p.description);
			const QString type = p.isIWAD ? QStringLiteral("IWAD") : QStringLiteral("(P)WAD PWAD");
			const QString iwadFile = QFileInfo(QString::fromStdString(p.iwadPath)).fileName();
			const QString pwadFile = QFileInfo(QString::fromStdString(p.pwadPath)).fileName();

			for (const QString& term : terms)
			{
				bool termHit = title.contains(term, Qt::CaseInsensitive)
					|| author.contains(term, Qt::CaseInsensitive)
					|| desc.contains(term, Qt::CaseInsensitive)
					|| type.contains(term, Qt::CaseInsensitive)
					|| iwadFile.contains(term, Qt::CaseInsensitive)
					|| pwadFile.contains(term, Qt::CaseInsensitive);
				if (!termHit)
				{
					for (const std::string& modPath : p.mods)
					{
						if (QFileInfo(QString::fromStdString(modPath)).fileName().contains(term, Qt::CaseInsensitive))
						{
							termHit = true;
							break;
						}
					}
				}
				if (!termHit)
				{
					match = false;
					break;
				}
			}
		}

		table->setRowHidden(r, !match);
		if (match)
			++visibleCount;
	}

	if (terms.isEmpty() && category == 0)
		statusCountLabel->setText(LStr("LAUNCHER_AVAILABLE_PROFILES", "Available Profiles:") + QString(" %1").arg(profiles.size()));
	else
		statusCountLabel->setText(LStr("LAUNCHER_SHOWING_PROFILES", "Showing %1 of %2 profiles").arg(visibleCount).arg(profiles.size()));
}

void LauncherMainWindow::updateCoverPixmapScale()
{
	if (cachedCoverPixmap.isNull() && !usingFallbackCover)
		return;

	coverLabel->setFrameShape(QFrame::StyledPanel);
	coverLabel->setFrameShadow(QFrame::Sunken);
	const int fw = coverLabel->frameWidth();
	const int h = std::max(64, coverLabel->height());
	const int innerH = std::max(64, h - 2 * fw);
	const double aspect = (!usingFallbackCover && cachedCoverPixmap.height() > 0)
		? (static_cast<double>(cachedCoverPixmap.width()) / static_cast<double>(cachedCoverPixmap.height()))
		: (4.0 / 3.0);
	const int innerW = std::clamp(qRound(innerH * aspect), 160, 320);
	const int targetW = innerW + 2 * fw;
	if (coverLabel->width() != targetW)
		coverLabel->setFixedWidth(targetW);

	const qreal dpr = std::max<qreal>(1.0, coverLabel->devicePixelRatioF());
	if (usingFallbackCover)
	{
		coverLabel->setPixmap(loadUZDoomGenericThumbnailPixmap(innerW, innerH, dpr));
		coverLabel->setText(QString());
		return;
	}

	const QSize targetPhys(
		std::max(64, qRound(innerW * dpr)),
		std::max(64, qRound(innerH * dpr)));
	QPixmap scaled = cachedCoverPixmap.scaled(
		targetPhys,
		Qt::IgnoreAspectRatio,
		Qt::SmoothTransformation);
	scaled.setDevicePixelRatio(dpr);
	coverLabel->setPixmap(scaled);
	coverLabel->setText(QString());
}

void LauncherMainWindow::updateDetailsPane()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
	{
		cachedCoverPath.clear();
		cachedCoverPixmap = QPixmap();
		usingFallbackCover = false;
		coverLabel->setFixedWidth(240);
		coverLabel->setFrameShape(QFrame::StyledPanel);
		coverLabel->setPixmap(QPixmap());
		coverLabel->setText(profiles.empty() ? "UZDoom" : LStr("LAUNCHER_NO_SELECTION", "No Profile Selected"));
		descView->setPlainText(profiles.empty()
			? LStr("LAUNCHER_EMPTY_HINT", "No profiles found yet.\nDrop a .wad, .pk3, .ipk3, .zip, or .uzdp file here, or click 'New Entry' above.")
			: QString());
		return;
	}

	const Profile& p = profiles[idx];
	if (p.coverImagePath != cachedCoverPath)
	{
		cachedCoverPath = p.coverImagePath;
		cachedCoverPixmap = (!p.coverImagePath.empty() && QFileInfo::exists(QString::fromStdString(p.coverImagePath)))
			? QPixmap(QString::fromStdString(p.coverImagePath))
			: QPixmap();
	}

	usingFallbackCover = cachedCoverPixmap.isNull();
	updateCoverPixmapScale();

	if (!p.description.empty())
		descView->setPlainText(QString::fromStdString(p.description));
	else
		descView->setPlainText(LStr("LAUNCHER_NO_DESCRIPTION", "No description provided for this profile."));
}

void LauncherMainWindow::updateButtonStates()
{
	const bool isRunning = (gameProcess != nullptr);
	const int idx = selectedProfileIndex();
	const bool hasSel = (idx >= 0);
	const bool isSorted = (launcherConfig.sortColumn >= 1 && launcherConfig.sortColumn <= 6);
	const QString moveTip = isSorted
		? LStr("LAUNCHER_MOVE_DISABLED_SORTED", "Reset column sort (click sorted header or \u2605) to reorder profiles manually.")
		: QString();

	addMenu->menuAction()->setEnabled(!isRunning);
	prefMenu->menuAction()->setEnabled(!isRunning);
	btnPlay->setEnabled(!isRunning && hasSel);
	btnJoin->setEnabled(!isRunning && hasSel);
	btnHost->setEnabled(!isRunning && hasSel);
	btnSettings->setEnabled(!isRunning && hasSel);
	btnMoveUp->setEnabled(canMoveSelectedBy(-1));
	btnMoveDown->setEnabled(canMoveSelectedBy(1));
	btnMoveUp->setToolTip(moveTip);
	btnMoveDown->setToolTip(moveTip);
	btnRefresh->setEnabled(!isRunning);
#ifdef HAS_UPDATER
	if (updater)
		updater->setBannerEnabled(!isRunning);
#endif
}

void LauncherMainWindow::setRunningState(bool running, const QString& title)
{
	statusRunLabel->setText(running ? (LStr("LAUNCHER_RUNNING", "Running:") + " " + title) : QString());
	updateButtonStates();
}

void LauncherMainWindow::onSelectionChanged()
{
	const int idx = selectedProfileIndex();
	if (idx >= 0)
		launcherConfig.lastSelectedRow = idx;
	updateDetailsPane();
	updateButtonStates();
}

void LauncherMainWindow::onFilterTextChanged(const QString&)
{
	applyFilter();
	if (selectedProfileIndex() < 0 && !profiles.empty())
		selectProfileIndex(std::clamp(launcherConfig.lastSelectedRow, 0, static_cast<int>(profiles.size()) - 1));
	else
	{
		updateDetailsPane();
		updateButtonStates();
	}
}

void LauncherMainWindow::onFilterCategoryChanged(int)
{
	onFilterTextChanged(searchEdit->text());
}

void LauncherMainWindow::onHeaderSectionClicked(int logicalIndex)
{
	if (logicalIndex == 0)
	{
		if (launcherConfig.sortColumn >= 1)
		{
			launcherConfig.sortColumn = -1;
			launcherConfig.sortDescending = false;
			saveLauncherConfig(launcherConfig);
			refreshTable(selectedProfileIndex());
		}
		return;
	}

	if (logicalIndex < 1 || logicalIndex > 6)
		return;

	const bool defaultDesc = (logicalIndex == 5 || logicalIndex == 6);
	if (launcherConfig.sortColumn != logicalIndex)
	{
		launcherConfig.sortColumn = logicalIndex;
		launcherConfig.sortDescending = defaultDesc;
	}
	else if (launcherConfig.sortDescending == defaultDesc)
	{
		launcherConfig.sortDescending = !defaultDesc;
	}
	else
	{
		launcherConfig.sortColumn = -1;
		launcherConfig.sortDescending = false;
	}

	saveLauncherConfig(launcherConfig);
	refreshTable(selectedProfileIndex());
}

void LauncherMainWindow::onTableCellClicked(int row, int column)
{
	if (column != 0 || gameProcess)
		return;
	const int idx = profileIndexForTableRow(row);
	if (idx < 0)
		return;
	profiles[idx].favorite = !profiles[idx].favorite;
	saveProfileToDisk(profiles[idx]);
	refreshTable(idx);
}

void LauncherMainWindow::onToggleSelectedFavorite()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;
	profiles[idx].favorite = !profiles[idx].favorite;
	saveProfileToDisk(profiles[idx]);
	refreshTable(idx);
}

void LauncherMainWindow::moveSelectedProfile(int delta)
{
	if (!canMoveSelectedBy(delta))
		return;
	const int curRow = tableRowForProfileIndex(selectedProfileIndex());
	const int targetRow = curRow + delta;
	const int idxA = profileIndexForTableRow(curRow);
	const int idxB = profileIndexForTableRow(targetRow);
	if (idxA < 0 || idxB < 0)
		return;

	std::swap(profiles[idxA], profiles[idxB]);
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = idxB;
	refreshTable(idxB);
}

void LauncherMainWindow::onMoveUpClicked()
{
	moveSelectedProfile(-1);
}

void LauncherMainWindow::onMoveDownClicked()
{
	moveSelectedProfile(1);
}

void LauncherMainWindow::onPlayClicked()
{
	launchProfile(LaunchTargetMode::Single);
}

void LauncherMainWindow::onJoinClicked()
{
	launchProfile(LaunchTargetMode::Join);
}

void LauncherMainWindow::onHostClicked()
{
	launchProfile(LaunchTargetMode::Host);
}

void LauncherMainWindow::appendLaunchLogFooter(const QString& footerLine)
{
	if (activeLogFilePath.isEmpty())
		return;
	QFile f(activeLogFilePath);
	if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		const QByteArray out = ("\n" + footerLine + "\n").toUtf8();
		f.write(out);
		f.close();
	}
	activeLogFilePath.clear();
}

void LauncherMainWindow::recordRunningSessionPlaytime()
{
	if (runningProfileIndex < 0 || runningProfileIndex >= static_cast<int>(profiles.size()))
		return;

	const int profIdx = runningProfileIndex;
	runningProfileIndex = -1;

	const qint64 elapsedSecs = playTimer.isValid() ? (playTimer.elapsed() / 1000) : 0;
	Profile& prof = profiles[profIdx];
	prof.lastPlayed = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toStdString();
	if (elapsedSecs >= 30)
		prof.playtime += std::max<uint64_t>(1, static_cast<uint64_t>((elapsedSecs + 30) / 60));
	saveProfileToDisk(prof);

	if (launcherConfig.sortColumn == 5 || launcherConfig.sortColumn == 6)
	{
		refreshTable(selectedProfileIndex());
	}
	else
	{
		const int row = tableRowForProfileIndex(profIdx);
		if (row >= 0)
			updateTableRow(row, profIdx);
	}
}

void LauncherMainWindow::launchProfile(LaunchTargetMode mode)
{
	if (gameProcess)
		return;

	const int idx = selectedProfileIndex();
	if (idx < 0 || idx >= static_cast<int>(profiles.size()))
		return;

	const Profile p = profiles[idx];
	if (p.iwadPath.empty() || !QFileInfo::exists(QString::fromStdString(p.iwadPath)))
	{
		const auto choice = QMessageBox::warning(
			this,
			LStr("LAUNCHER_MISSING_IWAD_TITLE", "Missing IWAD"),
			LStr("LAUNCHER_MISSING_IWAD_MSG", "This profile does not have a valid IWAD file selected. Open Profile Settings now?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes);
		if (choice == QMessageBox::Yes)
			openSettingsForIndex(idx);
		return;
	}

	if (mode == LaunchTargetMode::Join && QString::fromStdString(p.remoteAddress).trimmed().isEmpty())
	{
		const auto choice = QMessageBox::warning(
			this,
			LStr("LAUNCHER_MISSING_HOST_TITLE", "Missing Host Address"),
			LStr("LAUNCHER_MISSING_HOST_MSG", "Please set a remote Host IP / Domain in Profile Settings before joining a multiplayer game. Open Profile Settings now?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes);
		if (choice == QMessageBox::Yes)
			openSettingsForIndex(idx);
		return;
	}

	QStringList missingFiles;
	if (!p.isIWAD && !p.pwadPath.empty())
	{
		const QString pwadQ = QString::fromStdString(p.pwadPath);
		if (!QFileInfo::exists(pwadQ))
			missingFiles << LStr("LAUNCHER_MISSING_PWAD_FMT", "(P)WAD: %1").arg(QDir::toNativeSeparators(pwadQ));
	}
	{
		QSet<QString> disabledKeys;
		for (const std::string& dm : p.disabledMods)
		{
			if (!dm.empty())
				disabledKeys.insert(normalizePathKey(QString::fromStdString(dm)));
		}
		for (const std::string& m : p.mods)
		{
			if (m.empty())
				continue;
			const QString modQ = QString::fromStdString(m);
			if (disabledKeys.contains(normalizePathKey(modQ)))
				continue;
			if (!QFileInfo::exists(modQ))
				missingFiles << LStr("LAUNCHER_MISSING_MOD_FMT", "Mod: %1").arg(QDir::toNativeSeparators(modQ));
		}
	}
	if (!missingFiles.isEmpty())
	{
		constexpr int kMaxListed = 8;
		QStringList listed = missingFiles.mid(0, kMaxListed);
		if (missingFiles.size() > kMaxListed)
			listed << LStr("LAUNCHER_AND_MORE", "... and %1 more").arg(missingFiles.size() - kMaxListed);

		QMessageBox box(this);
		box.setIcon(QMessageBox::Warning);
		box.setWindowTitle(LStr("LAUNCHER_MISSING_FILES_TITLE", "Missing Profile Files"));
		box.setText(LStr("LAUNCHER_MISSING_FILES_MSG",
			"Some files configured for '%1' could not be found:\n\n%2\n\nWould you like to open Profile Settings to fix them, or launch anyway?")
			.arg(QString::fromStdString(p.title), listed.join('\n')));
		QPushButton* btnOpenSettings = box.addButton(LStr("LAUNCHER_OPEN_SETTINGS", "Open Profile Settings"), QMessageBox::AcceptRole);
		QPushButton* btnLaunchAnyway = box.addButton(LStr("LAUNCHER_LAUNCH_ANYWAY", "Launch Anyway"), QMessageBox::ActionRole);
		box.addButton(QMessageBox::Cancel);
		box.setDefaultButton(btnOpenSettings);
		box.exec();
		if (box.clickedButton() == btnOpenSettings)
		{
			openSettingsForIndex(idx);
			return;
		}
		if (box.clickedButton() != btnLaunchAnyway)
			return;
	}

	if (p.recordDemo && !p.selectedLaunchDemoRecord.empty() && !p.demoDirPath.empty())
		QDir().mkpath(QString::fromStdString(p.demoDirPath));

	QStringList args = p.buildLaunchArguments(mode, launcherConfig.language);
	args.erase(std::remove_if(args.begin(), args.end(), [](const QString& a)
	{
		return a.compare(QStringLiteral("-showlauncher"), Qt::CaseInsensitive) == 0;
	}), args.end());

#ifdef HAS_UPDATER
	if (updater)
		updater->cancelPending();
#endif

	runningProfileIndex = idx;
	gameProcess = new QProcess(this);
	gameProcess->setWorkingDirectory(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath());

	activeLogFilePath.clear();
	if (!p.filePath.empty())
	{
		const QString profFolder = QFileInfo(QString::fromStdString(p.filePath)).absolutePath();
		const QString logsDirPath = QDir(profFolder).filePath(QStringLiteral("logs"));
		QDir().mkpath(logsDirPath);

		const QDateTime now = QDateTime::currentDateTime();
		const QString baseStamp = now.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
		QString candidate = QDir(logsDirPath).filePath(baseStamp + QStringLiteral(".log"));
		int suffix = 1;
		while (QFileInfo::exists(candidate))
			candidate = QDir(logsDirPath).filePath(QString("%1_%2.log").arg(baseStamp).arg(++suffix));
		activeLogFilePath = candidate;

		QFile logHeaderFile(activeLogFilePath);
		if (logHeaderFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
		{
			const QString header = QString("=== %1 Launch Log (%2) ===\nCommand: %3 %4\n================================================\n\n")
				.arg(GAMENAME)
				.arg(now.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
				.arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()))
				.arg(QString::fromStdString(p.giveLaunchCommand(mode, launcherConfig.language)));
			logHeaderFile.write(header.toUtf8());
			logHeaderFile.close();

			// Keep at most the 20 most recent logs per profile.
			constexpr int kMaxKeptLogs = 20;
			QDir logsDir(logsDirPath);
			const QFileInfoList existingLogs = logsDir.entryInfoList(
				{QStringLiteral("*.log")}, QDir::Files, QDir::Name | QDir::Reversed);
			for (int i = kMaxKeptLogs; i < existingLogs.size(); ++i)
				QFile::remove(existingLogs[i].absoluteFilePath());

			gameProcess->setProcessChannelMode(QProcess::MergedChannels);
			gameProcess->setStandardOutputFile(activeLogFilePath, QIODevice::Append);
		}
		else
		{
			activeLogFilePath.clear();
		}
	}

	connect(gameProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err) {
		if (err == QProcess::FailedToStart)
			cleanupFailedLaunch(true);
	});

	connect(gameProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
		[this](int exitCode, QProcess::ExitStatus exitStatus) {
			appendLaunchLogFooter(exitStatus == QProcess::CrashExit
				? QString("=== Process crashed (exit code %1) ===").arg(exitCode)
				: QString("=== Process exited (code %1) ===").arg(exitCode));

			if (wasHiddenForLaunch)
			{
				if (wasMaximizedBeforeHide)
					showMaximized();
				else
					showNormal();
				raise();
				activateWindow();
				wasHiddenForLaunch = false;
				wasMaximizedBeforeHide = false;
				QApplication::setQuitOnLastWindowClosed(true);
			}

			recordRunningSessionPlaytime();

			if (gameProcess)
			{
				gameProcess->deleteLater();
				gameProcess = nullptr;
			}
			runningProfileIndex = -1;
			setRunningState(false);
		});

	playTimer.restart();
	setRunningState(true, QString::fromStdString(p.title));
	gameProcess->start(QCoreApplication::applicationFilePath(), args);

	if (!gameProcess)
		return;

	if (gameProcess->state() == QProcess::NotRunning && gameProcess->error() == QProcess::FailedToStart)
	{
		cleanupFailedLaunch(false);
		return;
	}

	if (launcherConfig.hideWhilePlaying)
	{
		wasMaximizedBeforeHide = isMaximized();
		wasHiddenForLaunch = true;
		QApplication::setQuitOnLastWindowClosed(false);
		hide();
	}
}

void LauncherMainWindow::cleanupFailedLaunch(bool showErrorDialog)
{
	if (!gameProcess)
		return;

	appendLaunchLogFooter(QStringLiteral("=== Failed to start executable ==="));

	if (wasHiddenForLaunch)
	{
		if (wasMaximizedBeforeHide)
			showMaximized();
		else
			showNormal();
		raise();
		activateWindow();
		wasHiddenForLaunch = false;
		wasMaximizedBeforeHide = false;
		QApplication::setQuitOnLastWindowClosed(true);
	}

	QProcess* proc = gameProcess;
	gameProcess = nullptr;
	proc->disconnect(this);
	proc->deleteLater();
	runningProfileIndex = -1;
	setRunningState(false);

	if (showErrorDialog)
	{
		QMessageBox::critical(
			this,
			LStr("LAUNCHER_LAUNCH_FAILED_TITLE", "Launch Failed"),
			LStr("LAUNCHER_LAUNCH_FAILED_MSG", "Failed to launch %1 executable:\n%2")
				.arg(GAMENAME, QCoreApplication::applicationFilePath()));
	}
}

void LauncherMainWindow::openSettingsForIndex(int profileIdx)
{
	if (gameProcess || profileIdx < 0 || profileIdx >= static_cast<int>(profiles.size()))
		return;

	Profile workingCopy = profiles[profileIdx];
	ProfileSettingsDialog dlg(
		workingCopy,
		profiles,
		QString::fromStdString(launcherConfig.defaultModsDir),
		launcherConfig.language,
		this);
	dlg.exec();

	switch (dlg.actionResult())
	{
	case ProfileSettingsDialog::ActionResult::Saved:
		saveProfileToDisk(workingCopy);
		profiles[profileIdx] = std::move(workingCopy);
		cachedCoverPath.clear();
		refreshTable(profileIdx);
		break;

	case ProfileSettingsDialog::ActionResult::Cloned:
		profiles.push_back(dlg.clonedProfile());
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = static_cast<int>(profiles.size()) - 1;
		refreshTable(launcherConfig.lastSelectedRow);
		break;

	case ProfileSettingsDialog::ActionResult::Deleted:
		profiles.erase(profiles.begin() + profileIdx);
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = std::min(profileIdx, static_cast<int>(profiles.size()) - 1);
		refreshTable(launcherConfig.lastSelectedRow);
		break;

	case ProfileSettingsDialog::ActionResult::Cancelled:
	default:
		break;
	}
}

void LauncherMainWindow::onProfileSettingsClicked()
{
	openSettingsForIndex(selectedProfileIndex());
}

void LauncherMainWindow::onAddGameFiles()
{
	if (gameProcess)
		return;
	const QString filter = QString("%1 (*.wad *.iwad *.pwad *.pk3 *.ipk3 *.pk7 *.ipk7 *.deh *.bex *.zip *.7z *.uzdp);;%2 (*.*)")
		.arg(LStr("LAUNCHER_FILTER_DOOM_FILES", "Doom Files"), LStr("FILEPICKER_ANY", "All Files"));
	const QStringList files = QFileDialog::getOpenFileNames(
		this,
		LStr("LAUNCHER_MENU_ADD_FILE", "Add WAD / Mod / Archive"),
		QString::fromStdString(LauncherConst::EXEC_DIR),
		filter);
	if (!files.isEmpty())
		handleImportedFiles(files);
}

void LauncherMainWindow::onCreateBlankProfile()
{
	if (gameProcess)
		return;

	bool ok = false;
	const QString title = QInputDialog::getText(
		this,
		LStr("LAUNCHER_MENU_NEW_BLANK", "Create Blank Profile"),
		LStr("LAUNCHER_ENTER_PROFILE_NAME", "Profile title:"),
		QLineEdit::Normal,
		LStr("LAUNCHER_NEW_PROFILE", "New Profile"),
		&ok).trimmed();
	if (!ok || title.isEmpty())
		return;

	Profile p;
	p.title = title.toStdString();
	p.isIWAD = false;
	p.iwadPath = findBestDefaultIWADPath(profiles);
	p.orderIndex = static_cast<int>(profiles.size());
	createDefaultDirectoryStructure(p);
	saveProfileToDisk(p);

	profiles.push_back(std::move(p));
	const int newIdx = static_cast<int>(profiles.size()) - 1;
	launcherConfig.lastSelectedRow = newIdx;
	refreshTable(newIdx);
	openSettingsForIndex(newIdx);
}

void LauncherMainWindow::handleImportedFiles(const QStringList& localPaths)
{
	if (localPaths.isEmpty())
		return;

	int lastCreatedIdx = -1;
	int addedCount = 0;
	int skippedCount = 0;
	QStringList errors;

	for (const QString& localPath : localPaths)
	{
		const QFileInfo fi(localPath);
		const QString ext = fi.suffix().toLower();
		const int curIdx = selectedProfileIndex();

		if ((ext == "deh" || ext == "bex") && curIdx >= 0)
		{
			const auto ans = QMessageBox::question(
				this,
				LStr("LAUNCHER_ATTACH_DEH_TITLE", "Attach DeHackEd Patch"),
				LStr("LAUNCHER_ATTACH_DEH_MSG", "Attach '%1' as a mod to selected profile '%2'?\n\nClick 'No' to create a separate profile instead.")
					.arg(fi.fileName(), QString::fromStdString(profiles[curIdx].title)),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
				QMessageBox::Yes);
			if (ans == QMessageBox::Cancel)
				continue;
			if (ans == QMessageBox::Yes)
			{
				QString finalModPath = localPath;
				bool cancelledAttach = false;
				if (profiles[curIdx].copyModsToProfile && !profiles[curIdx].filePath.empty())
				{
					const QDir modsDir(QFileInfo(QString::fromStdString(profiles[curIdx].filePath)).absolutePath() + "/mods");
					modsDir.mkpath(".");
					const QString dest = modsDir.filePath(fi.fileName());
					const QString srcAbs = fi.absoluteFilePath();
					const QString destAbs = QFileInfo(dest).absoluteFilePath();
					if (normalizePathKey(srcAbs) != normalizePathKey(destAbs))
					{
						if (!QFile::exists(dest))
						{
							if (QFile::copy(localPath, dest))
								finalModPath = dest;
						}
						else
						{
							const std::string srcMd5 = calculateFileMD5(srcAbs.toStdString());
							const std::string dstMd5 = calculateFileMD5(destAbs.toStdString());
							if (!srcMd5.empty() && srcMd5 == dstMd5)
							{
								finalModPath = dest;
							}
							else
							{
								QMessageBox box(this);
								box.setIcon(QMessageBox::Question);
								box.setWindowTitle(LStr("LAUNCHER_MOD_EXISTS_TITLE", "Mod Already Exists"));
								box.setText(LStr("LAUNCHER_MOD_EXISTS_PROFILE_MSG",
									"A different file named '%1' already exists in '%2''s mods folder.\n\nOverwrite it with the selected file?")
									.arg(fi.fileName(), QString::fromStdString(profiles[curIdx].title)));
								QPushButton* btnOverwrite = box.addButton(LStr("LAUNCHER_OVERWRITE", "Overwrite"), QMessageBox::AcceptRole);
								QPushButton* btnKeep = box.addButton(LStr("LAUNCHER_KEEP_EXISTING", "Keep Existing"), QMessageBox::NoRole);
								box.addButton(QMessageBox::Cancel);
								box.setDefaultButton(btnOverwrite);
								box.exec();

								if (box.clickedButton() == btnOverwrite)
								{
									const QString tmpDest = dest + ".uztmp";
									const QString bakDest = dest + ".uzbak";
									auto makeWritable = [](const QString& p)
									{
										if (QFileInfo::exists(p))
											QFile::setPermissions(p, QFile::permissions(p) | QFileDevice::WriteOwner | QFileDevice::WriteUser);
									};
									makeWritable(tmpDest);
									makeWritable(bakDest);
									QFile::remove(tmpDest);
									QFile::remove(bakDest);
									bool replaced = false;
									if (QFile::copy(localPath, tmpDest))
									{
										const bool hadDest = QFileInfo::exists(dest);
										if (hadDest)
											makeWritable(dest);
										if (!hadDest || QFile::rename(dest, bakDest))
										{
											if (QFile::rename(tmpDest, dest))
											{
												if (hadDest)
												{
													makeWritable(bakDest);
													QFile::remove(bakDest);
												}
												replaced = true;
											}
											else
											{
												makeWritable(tmpDest);
												QFile::remove(tmpDest);
												if (hadDest)
													QFile::rename(bakDest, dest);
											}
										}
										else
										{
											makeWritable(tmpDest);
											QFile::remove(tmpDest);
										}
									}
									if (!replaced)
									{
										errors << LStr("LAUNCHER_MOD_OVERWRITE_ERR", "%1: Could not overwrite existing mod in profile folder.").arg(fi.fileName());
										cancelledAttach = true;
									}
									else
									{
										finalModPath = dest;
									}
								}
								else if (box.clickedButton() == btnKeep)
								{
									finalModPath = dest;
								}
								else
								{
									cancelledAttach = true;
								}
							}
						}
					}
				}
				if (cancelledAttach)
					continue;
				const std::string modStd = finalModPath.toStdString();
				const QString modKey = normalizePathKey(finalModPath);
				const bool alreadyPresent = std::any_of(profiles[curIdx].mods.begin(), profiles[curIdx].mods.end(), [&](const std::string& existing)
				{
					return normalizePathKey(QString::fromStdString(existing)) == modKey;
				});
				if (!alreadyPresent)
					profiles[curIdx].mods.push_back(modStd);
				auto& dmods = profiles[curIdx].disabledMods;
				dmods.erase(std::remove_if(dmods.begin(), dmods.end(), [&](const std::string& dm)
				{
					return normalizePathKey(QString::fromStdString(dm)) == modKey;
				}), dmods.end());
				saveProfileToDisk(profiles[curIdx]);
				++addedCount;
				lastCreatedIdx = curIdx;
				continue;
			}
		}

		const ImportOutcome outcome = importGameFileIntoProfiles(localPath.toStdString(), profiles);
		if (outcome.status == ImportResult::Success)
		{
			Profile p = outcome.createdProfile;
			p.orderIndex = static_cast<int>(profiles.size());
			saveProfileToDisk(p);
			profiles.push_back(std::move(p));
			lastCreatedIdx = static_cast<int>(profiles.size()) - 1;
			++addedCount;
		}
		else if (outcome.status == ImportResult::DuplicateProfile)
		{
			++skippedCount;
		}
		else if (outcome.status != ImportResult::Cancelled)
		{
			errors << QString("%1: %2").arg(fi.fileName(), QString::fromStdString(outcome.message));
		}
	}

	if (lastCreatedIdx >= 0)
	{
		persistProfileOrdering(profiles);
		launcherConfig.lastSelectedRow = lastCreatedIdx;
		refreshTable(lastCreatedIdx);
	}

	if (!errors.isEmpty())
	{
		QMessageBox::warning(this, LStr("LAUNCHER_IMPORT_ISSUE_TITLE", "Import Issue"), errors.join("\n"));
	}
	else if (skippedCount > 0 && addedCount == 0)
	{
		QMessageBox::information(
			this,
			LStr("LAUNCHER_DUPLICATE_PROFILE_TITLE", "Duplicate Profile"),
			LStr("LAUNCHER_DUPLICATE_PROFILE_MSG", "The selected file already matches an existing profile."));
	}
}

void LauncherMainWindow::startStartupIWadDetection()
{
	startIWadAutoDetect(false);
}

void LauncherMainWindow::startStartupUpdateCheck()
{
#ifdef HAS_UPDATER
	if (updater)
		updater->checkForUpdate(false, this);
#endif
}

void LauncherMainWindow::startIWadAutoDetect(bool interactive)
{
	if (detectThread)
	{
		if (detectThread->isRunning())
		{
			if (interactive)
				detectInteractive = true;
			return;
		}
		detectThread->wait();
		detectThread = nullptr;
	}

	detectInteractive = interactive;
	detectCancel = std::make_shared<std::atomic<bool>>(false);

	std::vector<Profile> profilesSnapshot = profiles;
	std::vector<DiscoveredIWad> engineIWads = g_discoveredEngineIWads;
	std::vector<std::string> extraDirs = {LauncherConst::EXEC_DIR, LauncherConst::ROOT_DIR};
	auto cancelFlag = detectCancel;

	if (!gameProcess)
		statusRunLabel->setText(LStr("LAUNCHER_SCANNING_IWADS", "Scanning for IWADs..."));

	QThread* thread = QThread::create([this,
		cancelFlag,
		profilesSnapshot = std::move(profilesSnapshot),
		engineIWads = std::move(engineIWads),
		extraDirs = std::move(extraDirs)]() mutable
	{
		std::vector<DiscoveredIWadCandidate> candidates = scanUnregisteredIWADs(
			profilesSnapshot,
			engineIWads,
			extraDirs,
			cancelFlag.get());

		if (cancelFlag->load(std::memory_order_relaxed))
			return;

		QMetaObject::invokeMethod(this, [this, cancelFlag, candidates = std::move(candidates)]()
		{
			if (cancelFlag->load(std::memory_order_relaxed))
				return;
			onIWadScanFinished(candidates);
		}, Qt::QueuedConnection);
	});

	detectThread = thread;
	QPointer<QThread> threadGuard(thread);
	connect(thread, &QThread::finished, this, [this, threadGuard]()
	{
		if (detectThread == threadGuard)
			detectThread = nullptr;
	});
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);
	thread->start();
}

void LauncherMainWindow::onIWadScanFinished(const std::vector<DiscoveredIWadCandidate>& candidates)
{
	if (!gameProcess)
		statusRunLabel->clear();

	const bool interactive = detectInteractive;
	detectInteractive = false;

	const int prevIdx = selectedProfileIndex();
	const int found = commitDiscoveredIWADs(profiles, candidates);
	if (found > 0)
	{
		persistProfileOrdering(profiles);
		refreshTable(prevIdx >= 0 ? prevIdx : 0);
	}

	if (interactive)
	{
		QMessageBox::information(
			this,
			LStr("LAUNCHER_MENU_AUTODETECT", "Auto-Detect IWADs"),
			found > 0
				? LStr("LAUNCHER_IWADS_FOUND_MSG", "Detected and imported %1 new IWAD profile(s).").arg(found)
				: LStr("LAUNCHER_IWADS_NONE_MSG", "No new IWADs were found."));
	}
}

void LauncherMainWindow::onRefreshProfiles()
{
	if (gameProcess)
		return;
	const int prevIdx = selectedProfileIndex();
	const QString prevPathKey = (prevIdx >= 0 && !profiles[prevIdx].filePath.empty())
		? normalizePathKey(QString::fromStdString(profiles[prevIdx].filePath))
		: QString();
	profiles = loadAllProfiles();
	int nextSelectIdx = prevIdx;
	if (!prevPathKey.isEmpty())
	{
		int matched = -1;
		for (int i = 0; i < static_cast<int>(profiles.size()); ++i)
		{
			if (!profiles[i].filePath.empty()
				&& normalizePathKey(QString::fromStdString(profiles[i].filePath)) == prevPathKey)
			{
				matched = i;
				break;
			}
		}
		if (matched >= 0)
			nextSelectIdx = matched;
		else if (!profiles.empty())
			nextSelectIdx = std::clamp(prevIdx, 0, static_cast<int>(profiles.size()) - 1);
		else
			nextSelectIdx = -1;
	}
	refreshTable(nextSelectIdx);
	startIWadAutoDetect(false);
}

void LauncherMainWindow::onOpenLauncherFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(LauncherConst::ROOT_DIR)));
}

void LauncherMainWindow::onOpenExecFolder()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(LauncherConst::EXEC_DIR)));
}

void LauncherMainWindow::onOpenSelectedProfileFolder()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;
	const QFileInfo fi(QString::fromStdString(profiles[idx].filePath));
	const QString dir = fi.absolutePath();
	if (!dir.isEmpty())
		QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void LauncherMainWindow::onCloneSelectedProfile()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	bool ok = false;
	const QString newTitle = QInputDialog::getText(
		this,
		LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile"),
		LStr("LAUNCHER_CLONE_PROMPT", "New profile title:"),
		QLineEdit::Normal,
		QString::fromStdString(profiles[idx].title) + " " + LStr("LAUNCHER_COPY_SUFFIX", "(Copy)"),
		&ok).trimmed();
	if (!ok || newTitle.isEmpty())
		return;

	Profile cloned;
	if (!cloneProfile(profiles[idx], newTitle.toStdString(), cloned))
	{
		QMessageBox::warning(
			this,
			LStr("LAUNCHER_CLONE_FAILED_TITLE", "Clone Failed"),
			LStr("LAUNCHER_CLONE_FAILED_MSG", "Could not clone the selected profile."));
		return;
	}
	profiles.push_back(std::move(cloned));
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = static_cast<int>(profiles.size()) - 1;
	refreshTable(launcherConfig.lastSelectedRow);
}

void LauncherMainWindow::onExportSelectedProfile()
{
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const Profile p = profiles[idx];
	const QString suggested = QString::fromStdString(sanitizeFolderName(p.title)) + ".uzdp";
	const QString filter = QString("%1 (*.uzdp)").arg(LStr("LAUNCHER_FILTER_UZDP", "UZDoom Profile Archive"));
	const QString dest = QFileDialog::getSaveFileName(
		this,
		LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile"),
		QDir::home().filePath(suggested),
		filter);
	if (dest.isEmpty())
		return;

	std::string err;
	if (!exportProfileToZip(p, dest.toStdString(), err))
	{
		QMessageBox::warning(this, LStr("LAUNCHER_EXPORT_FAILED_TITLE", "Export Failed"), QString::fromStdString(err));
	}
	else
	{
		QMessageBox::information(
			this,
			LStr("LAUNCHER_EXPORT_COMPLETE_TITLE", "Export Complete"),
			LStr("LAUNCHER_EXPORT_COMPLETE_MSG", "Exported profile to:\n%1").arg(dest));
	}
}

void LauncherMainWindow::onDeleteSelectedProfile()
{
	if (gameProcess)
		return;
	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const auto ans = QMessageBox::warning(
		this,
		LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile"),
		LStr("LAUNCHER_DELETE_PROFILE_MSG",
			"Delete profile '%1' and its profile folder?\nExternal IWAD files outside the profile folder are preserved.")
			.arg(QString::fromStdString(profiles[idx].title)),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);
	if (ans != QMessageBox::Yes)
		return;

	if (!deleteProfileDirectory(profiles[idx]))
	{
		QMessageBox::warning(
			this,
			LStr("LAUNCHER_ERROR", "Error"),
			LStr("LAUNCHER_DELETE_PROFILE_FAILED_OPEN", "Failed to remove profile directory. Make sure no files or folders inside it are open in another program."));
		return;
	}
	profiles.erase(profiles.begin() + idx);
	persistProfileOrdering(profiles);
	launcherConfig.lastSelectedRow = std::min(idx, static_cast<int>(profiles.size()) - 1);
	refreshTable(launcherConfig.lastSelectedRow);
}

void LauncherMainWindow::onTableContextMenu(const QPoint& pos)
{
	const QTableWidgetItem* hit = table->itemAt(pos);
	if (!hit)
		return;
	table->selectRow(hit->row());

	const int idx = selectedProfileIndex();
	if (idx < 0)
		return;

	const bool idle = (gameProcess == nullptr);

	QMenu menu(this);
	QAction* actPlay = menu.addAction(btnPlay->text(), this, &LauncherMainWindow::onPlayClicked);
	QAction* actJoin = menu.addAction(btnJoin->text(), this, &LauncherMainWindow::onJoinClicked);
	QAction* actHost = menu.addAction(btnHost->text(), this, &LauncherMainWindow::onHostClicked);
	actPlay->setEnabled(idle);
	actJoin->setEnabled(idle);
	actHost->setEnabled(idle);

	menu.addSeparator();
	QAction* actFav = menu.addAction(
		profiles[idx].favorite
			? LStr("LAUNCHER_UNPIN_FAVORITE", "Unpin Favorite")
			: LStr("LAUNCHER_PIN_FAVORITE", "Pin to Top (Favorite)"),
		this, &LauncherMainWindow::onToggleSelectedFavorite);
	actFav->setEnabled(idle);
	QAction* actSettings = menu.addAction(btnSettings->text(), this, &LauncherMainWindow::onProfileSettingsClicked);
	QAction* actClone = menu.addAction(LStr("LAUNCHER_CLONE_PROFILE", "Clone Profile") + QStringLiteral("..."), this, &LauncherMainWindow::onCloneSelectedProfile);
	menu.addAction(LStr("LAUNCHER_EXPORT_PROFILE", "Export Profile") + QStringLiteral("..."), this, &LauncherMainWindow::onExportSelectedProfile);
	menu.addAction(LStr("LAUNCHER_OPEN_PROFILE_DIR", "Open Profile Folder"), this, &LauncherMainWindow::onOpenSelectedProfileFolder);
	actSettings->setEnabled(idle);
	actClone->setEnabled(idle);

	menu.addSeparator();
	QAction* actUp = menu.addAction(btnMoveUp->text(), this, &LauncherMainWindow::onMoveUpClicked);
	QAction* actDown = menu.addAction(btnMoveDown->text(), this, &LauncherMainWindow::onMoveDownClicked);
	actUp->setEnabled(canMoveSelectedBy(-1));
	actDown->setEnabled(canMoveSelectedBy(1));

	menu.addSeparator();
	QAction* actDel = menu.addAction(LStr("LAUNCHER_DELETE_PROFILE", "Delete Profile") + QStringLiteral("..."), this, &LauncherMainWindow::onDeleteSelectedProfile);
	actDel->setEnabled(idle);

	menu.exec(table->viewport()->mapToGlobal(pos));
}

void LauncherMainWindow::onAboutClicked()
{
#ifdef HAS_UPDATER
	std::function<void(QWidget*)> checkUpdatesCb;
	if (updater && !gameProcess)
	{
		checkUpdatesCb = [this](QWidget* dlgParent) {
			if (updater && !gameProcess)
				updater->checkForUpdate(true, dlgParent);
		};
	}
	AboutDialog::showAdditionalInfo(launcherConfig.showReleaseNotes, false, this, std::move(checkUpdatesCb), showUpcomingReleaseNotes);
#else
	AboutDialog::showAdditionalInfo(launcherConfig.showReleaseNotes, false, this, {}, showUpcomingReleaseNotes);
#endif
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::onLanguageTriggered(QAction* action)
{
	if (!action)
		return;
	const std::string code = action->data().toString().toStdString();
	launcherConfig.language = code;
	GStrings.UpdateLanguage(code.c_str());
	saveLauncherConfig(launcherConfig);
	retranslateUi();
}

void LauncherMainWindow::onThemeTriggered(QAction* action)
{
	if (!action)
		return;
	launcherConfig.theme = action->data().toString().toStdString();
	applyLauncherTheme(launcherConfig.theme);
	saveLauncherConfig(launcherConfig);
	if (prideBannerWidget)
		prideBannerWidget->update();
	refreshTable(selectedProfileIndex());
}

void LauncherMainWindow::applyBannerSelection(const std::string &rawId)
{
	const std::string idCopy = rawId;
	prideBannerColors = idCopy;
	if (!prideBannerWidget)
		return;

	auto *banner = static_cast<PrideBannerWidget *>(prideBannerWidget);
	const QString raw = QString::fromStdString(idCopy).trimmed();
	const QString lower = raw.toLower();

	if (lower.isEmpty())
	{
		banner->setBanner({}, false);
	}
	else if (lower == QStringLiteral("default"))
	{
		banner->setBanner({}, true);
	}
	else if (lower.startsWith(QStringLiteral("file:")))
	{
		const QString path = raw.mid(5).trimmed();
		banner->setBanner({}, false, loadCustomBannerPixmap(path));
	}
	else if (QFileInfo::exists(raw))
	{
		banner->setBanner({}, false, loadCustomBannerPixmap(raw));
	}
	else
	{
		std::vector<QColor> stripeColors = resolvePrideFlagColors(idCopy);
		banner->setBanner(std::move(stripeColors), false);
	}
}

void LauncherMainWindow::syncBannerMenuState()
{
	if (!bannerGroup)
		return;

	const QString raw = QString::fromStdString(prideBannerColors).trimmed();
	const QString cur = raw.toLower();
	const QString lookup = (cur == QStringLiteral("list")) ? QStringLiteral("random") : cur;
	QAction* matched = nullptr;
	QAction* customAct = nullptr;

	for (QAction* act : bannerGroup->actions())
	{
		const QString data = act->data().toString();
		if (data == QStringLiteral("__custom__"))
			customAct = act;
		else if (data == lookup)
			matched = act;
	}

	if (matched)
		matched->setChecked(true);
	else if (!cur.isEmpty() && customAct)
		customAct->setChecked(true);
}

void LauncherMainWindow::onBannerTriggered(QAction* action)
{
	if (!action)
		return;

	const QString id = action->data().toString();
	if (id == QStringLiteral("__custom__"))
	{
		QString startDir = QDir::homePath();
		const QString cur = QString::fromStdString(prideBannerColors).trimmed();
		if (cur.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive))
		{
			const QFileInfo fi(cur.mid(5).trimmed());
			if (fi.dir().exists())
				startDir = fi.absolutePath();
		}

		const QString picked = QFileDialog::getOpenFileName(
			this,
			LStr("LAUNCHER_BANNER_CUSTOM_TITLE", "Select Custom Banner Image (Recommended ratio: ~9:1, e.g. 940x104)"),
			startDir,
			buildBannerImageFilter());
		if (picked.isEmpty())
		{
			syncBannerMenuState();
			return;
		}

		const QString cleanPath = QDir::cleanPath(picked);
		if (loadCustomBannerPixmap(cleanPath).isNull())
		{
			QMessageBox::warning(
				this,
				LStr("LAUNCHER_BANNER_INVALID_IMAGE_TITLE", "Invalid Banner Image"),
				LStr("LAUNCHER_BANNER_INVALID_IMAGE_MSG", "Could not load the selected image file."));
			syncBannerMenuState();
			return;
		}

		QString copiedPath;
		if (!copyBannerToExecDir(cleanPath, copiedPath))
		{
			QMessageBox::warning(
				this,
				LStr("LAUNCHER_BANNER_COPY_FAILED_TITLE", "Banner Copy Failed"),
				LStr("LAUNCHER_BANNER_COPY_FAILED_MSG", "Could not copy the banner image to the application folder."));
			syncBannerMenuState();
			return;
		}

		const std::string nextId = (QStringLiteral("file:") + copiedPath).toStdString();
		applyBannerSelection(nextId);
		launcherConfig.banner = nextId;
		saveLauncherConfig(launcherConfig);
		return;
	}

	const std::string nextId = id.toStdString();
	applyBannerSelection(nextId);
	launcherConfig.banner = nextId;
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::updateDefaultModsMenuState()
{
	const bool hasDir = !launcherConfig.defaultModsDir.empty();
	if (actSetDefaultModsDir)
	{
		actSetDefaultModsDir->setToolTip(hasDir
			? LStr("LAUNCHER_DEFAULT_MODS_CURRENT_TIP", "Current Default Mods Folder:\n%1").arg(QString::fromStdString(launcherConfig.defaultModsDir))
			: LStr("LAUNCHER_DEFAULT_MODS_EMPTY_TIP", "Choose a master folder that 'Add Mod File' opens in."));
	}
	if (actClearDefaultModsDir)
	{
		actClearDefaultModsDir->setVisible(hasDir);
		actClearDefaultModsDir->setEnabled(hasDir);
	}
}

void LauncherMainWindow::onSetDefaultModsDir()
{
	const QString startDir = QString::fromStdString(
		launcherConfig.defaultModsDir.empty() ? LauncherConst::EXEC_DIR : launcherConfig.defaultModsDir);
	const QString picked = QFileDialog::getExistingDirectory(
		this,
		LStr("LAUNCHER_SELECT_DEFAULT_MODS_TITLE", "Select Default Mods Folder"),
		startDir);
	if (picked.isEmpty())
		return;
	launcherConfig.defaultModsDir = QDir::cleanPath(picked).toStdString();
	saveLauncherConfig(launcherConfig);
	updateDefaultModsMenuState();
}

void LauncherMainWindow::onClearDefaultModsDir()
{
	launcherConfig.defaultModsDir.clear();
	saveLauncherConfig(launcherConfig);
	updateDefaultModsMenuState();
}

void LauncherMainWindow::onToggleHideWhilePlaying(bool checked)
{
	launcherConfig.hideWhilePlaying = checked;
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::onToggleDontAskAgain(bool checked)
{
	launcherConfig.dontAskAgain = checked;
	saveLauncherConfig(launcherConfig);
}

#ifdef HAS_UPDATER
void LauncherMainWindow::updateUpdaterMenuState()
{
	if (!updater)
		return;

	const int mode = static_cast<int>(updater->currentMode());
	if (updaterModeGroup)
	{
		for (QAction* act : updaterModeGroup->actions())
			act->setChecked(act->data().toInt() == mode);
	}

	const int days = updater->currentIntervalDays();
	if (updaterIntervalGroup)
	{
		for (QAction* act : updaterIntervalGroup->actions())
			act->setChecked(act->data().toInt() == days);
	}
}

void LauncherMainWindow::onUpdaterModeTriggered(QAction* action)
{
	if (!action || !updater)
		return;
	updater->setMode(static_cast<UpdaterMode>(action->data().toInt()));
}

void LauncherMainWindow::onUpdaterIntervalTriggered(QAction* action)
{
	if (!action || !updater)
		return;
	updater->setIntervalDays(action->data().toInt());
}
#endif

void LauncherMainWindow::setShowReleaseNotes(bool show)
{
	launcherConfig.showReleaseNotes = show;
	saveLauncherConfig(launcherConfig);
}

void LauncherMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	if (!gameProcess && hasSupportedLocalFiles(event->mimeData()))
		event->acceptProposedAction();
}

void LauncherMainWindow::dropEvent(QDropEvent* event)
{
	if (gameProcess)
		return;
	const QStringList files = extractSupportedLocalFiles(event->mimeData());
	if (!files.isEmpty())
	{
		event->acceptProposedAction();
		// Defer modal import dialogs / extraction so the OS drag source (Explorer/Finder) doesn't freeze.
		QTimer::singleShot(0, this, [this, files]() {
			if (!gameProcess)
				handleImportedFiles(files);
		});
	}
}

bool LauncherMainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == coverLabel && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
	{
		if (!cachedCoverPixmap.isNull() || usingFallbackCover)
			updateCoverPixmapScale();
	}
	else if (watched == table && event->type() == QEvent::KeyPress)
	{
		const auto* keyEv = static_cast<QKeyEvent*>(event);
		if (!gameProcess && selectedProfileIndex() >= 0)
		{
			if (keyEv->key() == Qt::Key_Return || keyEv->key() == Qt::Key_Enter)
			{
				onPlayClicked();
				return true;
			}
			if (keyEv->key() == Qt::Key_Delete)
			{
				onDeleteSelectedProfile();
				return true;
			}
		}
	}
	return QMainWindow::eventFilter(watched, event);
}

void LauncherMainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	if (!cachedCoverPixmap.isNull() || usingFallbackCover)
		updateCoverPixmapScale();
}

void LauncherMainWindow::closeEvent(QCloseEvent* event)
{
	if (gameProcess && gameProcess->state() != QProcess::NotRunning)
	{
		const auto ans = QMessageBox::question(
			this,
			LStr("LAUNCHER_GAME_RUNNING_TITLE", "Game Running"),
			LStr("LAUNCHER_GAME_RUNNING_MSG", "%1 is currently running. Closing the launcher will terminate the active session.\nClose anyway?").arg(GAMENAME),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		if (ans != QMessageBox::Yes)
		{
			event->ignore();
			return;
		}
	}

	launcherConfig.maximized = isMaximized();
	if (verticalSplitter)
	{
		const QList<int> sizes = verticalSplitter->sizes();
		if (sizes.size() >= 2 && sizes[0] > 0 && sizes[1] > 0)
		{
			launcherConfig.splitterTopSize = sizes[0];
			launcherConfig.splitterBottomSize = sizes[1];
		}
	}
	saveLauncherConfig(launcherConfig);
	QMainWindow::closeEvent(event);
}
