/*
** errorwindow.cpp
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

#include "errorwindow.h"
#include "const.h"

#include "gstrings.h"
#include "m_misc.h"
#include "printf.h"
#include "utf8.h"
#include "v_font.h"
#include "version.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <miniz.h>

namespace
{
	std::string stripDoomColorEscapes(const std::string& log)
	{
		std::string out;
		out.reserve(log.size() + 64);

		const uint8_t* ptr = reinterpret_cast<const uint8_t*>(log.c_str());
		while (int ch = GetCharFromString(ptr))
		{
			if (ch == TEXTCOLOR_ESCAPE)
			{
				if (*ptr == '[')
				{
					while (*ptr && *ptr != ']')
						++ptr;
					if (*ptr == ']')
						++ptr;
				}
				else if (*ptr)
				{
					++ptr;
				}
				continue;
			}
			if (ch >= 0x1D && ch <= 0x1F)
				ch = 0x2550;
			out += MakeUTF8(ch);
		}
		return out;
	}
}

bool ErrorWindow::ExecModal(const std::string& text, const std::string& log, std::vector<uint8_t> minidump)
{
	ensureLauncherQtApplication();

	ErrorWindow dlg(text, log, std::move(minidump));
	dlg.exec();
	return dlg.restart;
}

ErrorWindow::ErrorWindow(const std::string& text, const std::string& log, std::vector<uint8_t> minidumpData, QWidget* parent)
	: QDialog(parent), minidump(std::move(minidumpData))
{
	const QString title = QString("%1 - %2 %3 (%4)")
		.arg(LStr("ERROR_FATAL", "Fatal Error"))
		.arg(GAMENAME)
		.arg(GetVersionString())
		.arg(GetGitTime());
	setWindowTitle(title);
	setMinimumSize(700, 480);
	resize(1040, 660);

	cleanReportText = stripDoomColorEscapes(log);
	cleanReportText += "\nExecution could not continue.\n";
	cleanReportText += stripDoomColorEscapes(text);
	cleanReportText += "\n";

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QLabel* header = new QLabel(LStr("ERROR_ABORT", "Execution could not continue."), this);
	QFont headerFont = header->font();
	headerFont.setPixelSize(16);
	headerFont.setBold(true);
	header->setFont(headerFont);
	{
		QPalette hp = header->palette();
		const QColor bg = hp.color(QPalette::Window);
		const bool isHighContrast = (bg.lightness() < 15 && hp.color(QPalette::Highlight) == QColor(255, 255, 0));
		const QColor errColor = isHighContrast
			? QColor(255, 255, 0)
			: (bg.lightness() > 127 ? QColor(175, 25, 25) : QColor(255, 110, 110));
		hp.setColor(QPalette::WindowText, errColor);
		header->setPalette(hp);
	}
	mainLayout->addWidget(header);

	QLabel* errLabel = new QLabel(QString::fromUtf8(stripDoomColorEscapes(text).c_str()), this);
	errLabel->setWordWrap(true);
	errLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mainLayout->addWidget(errLabel);

	logView = new QPlainTextEdit(this);
	logView->setReadOnly(true);
	logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	logView->setPlainText(QString::fromUtf8(cleanReportText.c_str()));
	mainLayout->addWidget(logView, 1);

	QHBoxLayout* btnLayout = new QHBoxLayout();

	QPushButton* btnCopy = new QPushButton(LStr("ACTION_COPYTOCLIPBOARD", "Copy to Clipboard"), this);
	connect(btnCopy, &QPushButton::clicked, this, &ErrorWindow::onCopyToClipboard);
	btnLayout->addWidget(btnCopy);

	btnLayout->addStretch();

	QPushButton* btnWadDir = new QPushButton(LStr("ACTION_OPENWADDIR", "Open WAD Directory"), this);
	connect(btnWadDir, &QPushButton::clicked, this, []() { M_OpenWadDir(); });
	btnLayout->addWidget(btnWadDir);

	QPushButton* btnConfigDir = new QPushButton(LStr("ACTION_OPENCONFIGDIR", "Open Config Directory"), this);
	connect(btnConfigDir, &QPushButton::clicked, this, []() { M_OpenConfigDir(); });
	btnLayout->addWidget(btnConfigDir);

	QPushButton* btnSave = new QPushButton(LStr("ERRORMNU_SAVE", "Save Report..."), this);
	connect(btnSave, &QPushButton::clicked, this, &ErrorWindow::onSaveReport);
	btnLayout->addWidget(btnSave);

	if (minidump.empty())
	{
		QPushButton* btnRestart = new QPushButton(LStr("ACTION_RESTART", "Restart"), this);
		connect(btnRestart, &QPushButton::clicked, this, &ErrorWindow::onRestartClicked);
		btnLayout->addWidget(btnRestart);
	}

	QPushButton* btnClose = new QPushButton(LStr("OPTVAL_CLOSE", "Close"), this);
	connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
	btnLayout->addWidget(btnClose);

	mainLayout->addLayout(btnLayout);
}

void ErrorWindow::onCopyToClipboard()
{
	if (QClipboard* cb = QApplication::clipboard())
		cb->setText(QString::fromUtf8(cleanReportText.c_str()));
}

void ErrorWindow::onSaveReport()
{
	const QString filter = QString("%1 (*.zip);;%2 (*.*)")
		.arg(LStr("FILEPICKER_REPORTS", "Crash Reports"))
		.arg(LStr("FILEPICKER_ANY", "All Files"));
	const QString defaultPath = QDir::home().filePath("CrashReport.zip");
	const QString path = QFileDialog::getSaveFileName(this, LStr("ERRORMNU_SAVE", "Save Crash Report"), defaultPath, filter);
	if (path.isEmpty())
		return;

	mz_zip_archive zip{};
	if (!mz_zip_writer_init_heap(&zip, 0, 1024 * 1024))
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), LStr("ERROR_ARCHIVE_INIT", "Could not create archive in memory."));
		return;
	}

	bool addedOk = true;
	if (!minidump.empty())
		addedOk = mz_zip_writer_add_mem(&zip, "minidump.dmp", minidump.data(), minidump.size(), MZ_DEFAULT_COMPRESSION);
	if (addedOk)
		addedOk = mz_zip_writer_add_mem(&zip, "log.txt", cleanReportText.data(), cleanReportText.size(), MZ_DEFAULT_COMPRESSION);

	if (!addedOk)
	{
		mz_zip_writer_end(&zip);
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), LStr("ERROR_ARCHIVE_ADD", "Failed to add crash report files to archive."));
		return;
	}

	void* buffer = nullptr;
	size_t bufferSize = 0;
	const bool finalized = mz_zip_writer_finalize_heap_archive(&zip, &buffer, &bufferSize);
	mz_zip_writer_end(&zip);

	if (!finalized || !buffer)
	{
		if (buffer)
			mz_free(buffer);
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), LStr("ERROR_ARCHIVE_FINALIZE", "Failed to finalize crash report archive."));
		return;
	}

	QFile outFile(path);
	const bool wrote = outFile.open(QIODevice::WriteOnly)
		&& outFile.write(static_cast<const char*>(buffer), static_cast<qint64>(bufferSize)) == static_cast<qint64>(bufferSize);
	mz_free(buffer);
	if (!wrote)
	{
		QMessageBox::warning(this, LStr("ERROR_FATAL", "Error"), LStr("ERROR_ARCHIVE_WRITE", "Could not write archive to selected destination."));
	}
}

void ErrorWindow::onRestartClicked()
{
	restart = true;
	accept();
}
