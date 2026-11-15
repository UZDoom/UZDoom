/*
** errorwindow.h
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

#include <QDialog>
#include <cstdint>
#include <string>
#include <vector>

class QPlainTextEdit;

class ErrorWindow : public QDialog
{
	Q_OBJECT
public:
	static bool ExecModal(const std::string& text, const std::string& log, std::vector<uint8_t> minidump = {});

	explicit ErrorWindow(const std::string& text, const std::string& log, std::vector<uint8_t> minidumpData, QWidget* parent = nullptr);

private slots:
	void onCopyToClipboard();
	void onSaveReport();
	void onRestartClicked();

private:
	std::vector<uint8_t> minidump;
	std::string cleanReportText;
	bool restart = false;
	QPlainTextEdit* logView = nullptr;
};
