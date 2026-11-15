/*
** flageditor.h
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

#include <QDialog>
#include <cstdint>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QWidget;

class FlagEditorDialog : public QDialog
{
	Q_OBJECT
public:
	struct FlagValues
	{
		bool alwaysApplyDMFlags = false;
		uint32_t dmflags = 0;
		uint32_t dmflags2 = 0;
		uint32_t dmflags3 = 0;
		uint32_t compatflags = 0;
		uint32_t compatflags2 = 0;
	};

	explicit FlagEditorDialog(const FlagValues& initial, QWidget* parent = nullptr);

	FlagValues values() const { return flagValues; }

private slots:
	void onSearchTextChanged(const QString& text);
	void onNumericFieldEdited();
	void onResetDefaults();

private:
	struct BoundFlag
	{
		FlagInfo info;
		bool isCompat = false;
		QWidget* rowWidget = nullptr;
		QCheckBox* checkbox = nullptr;
		QComboBox* combo = nullptr;
	};

	struct FlagSection
	{
		QLabel* header = nullptr;
		std::vector<size_t> flagIndices;
	};

	void populateCheckboxes(class QVBoxLayout* container, bool isCompat, const std::vector<FlagInfo>& defs);
	void syncCheckboxesFromValues(int filterIsCompat = -1, int filterSetIdx = -1);
	void syncNumericFromValues();
	uint32_t* targetRegister(bool isCompat, int setIdx);
	const uint32_t* targetRegister(bool isCompat, int setIdx) const;

	FlagValues flagValues;
	bool updating = false;
	std::vector<BoundFlag> boundFlags;
	std::vector<FlagSection> sections;

	QCheckBox* chkApplyDM = nullptr;
	QLineEdit* editSearch = nullptr;
	QLineEdit* editDm1 = nullptr;
	QLineEdit* editDm2 = nullptr;
	QLineEdit* editDm3 = nullptr;
	QLineEdit* editCompat1 = nullptr;
	QLineEdit* editCompat2 = nullptr;
};
