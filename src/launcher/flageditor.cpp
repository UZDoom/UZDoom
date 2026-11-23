/*
** flageditor.cpp
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

#include "flageditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

namespace
{
	uint32_t parseUIntText(const QString& text, bool* okOut = nullptr)
	{
		const QString trimmed = text.trimmed();
		if (trimmed.isEmpty())
		{
			if (okOut) *okOut = true;
			return 0;
		}
		bool ok = false;
		uint32_t val = 0;
		if (trimmed.startsWith("0x", Qt::CaseInsensitive))
			val = trimmed.mid(2).toUInt(&ok, 16);
		else if (trimmed.startsWith('-'))
			val = static_cast<uint32_t>(trimmed.toInt(&ok, 10));
		else
			val = trimmed.toUInt(&ok, 10);
		if (okOut) *okOut = ok;
		return val;
	}

	QLineEdit* makeUIntEdit(QWidget* parent)
	{
		QLineEdit* edit = new QLineEdit(parent);
		edit->setPlaceholderText("0");
		edit->setMaximumWidth(120);
		edit->setValidator(new QRegularExpressionValidator(
			QRegularExpression("^(0[xX][0-9a-fA-F]{0,8}|-?[0-9]{0,10})$"), edit));
		return edit;
	}
}

FlagEditorDialog::FlagEditorDialog(const FlagValues& initial, QWidget* parent)
	: QDialog(parent), flagValues(initial)
{
	setWindowTitle(LStr("LAUNCHER_FLAG_EDITOR", "Flag Editor"));
	setMinimumSize(540, 620);
	resize(560, 700);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QHBoxLayout* topRow = new QHBoxLayout();
	editSearch = new QLineEdit(this);
	editSearch->setPlaceholderText(LStr("LAUNCHER_SEARCH_FLAGS", "Filter flags..."));
	editSearch->setClearButtonEnabled(true);
	connect(editSearch, &QLineEdit::textChanged, this, &FlagEditorDialog::onSearchTextChanged);
	topRow->addWidget(editSearch, 1);

	QPushButton* btnReset = new QPushButton(LStr("OPTMNU_DEFAULTS", "Reset to defaults"), this);
	connect(btnReset, &QPushButton::clicked, this, &FlagEditorDialog::onResetDefaults);
	topRow->addWidget(btnReset);
	mainLayout->addLayout(topRow);

	QHBoxLayout* applyRow = new QHBoxLayout();
	chkApplyDM = new QCheckBox(LStr("LAUNCHER_APPLY_DMFLAGS", "Always apply DMFlags (even in Singleplayer)"), this);
	chkApplyDM->setChecked(flagValues.alwaysApplyDMFlags);
	connect(chkApplyDM, &QCheckBox::toggled, this, [this](bool on) { flagValues.alwaysApplyDMFlags = on; });

	applyRow->addWidget(chkApplyDM);
	mainLayout->addLayout(applyRow);

	QTabWidget* tabs = new QTabWidget(this);

	auto makeScrollTab = [&](const QString& tabTitle, bool isCompat, const std::vector<FlagInfo>& defs)
	{
		QScrollArea* scroll = new QScrollArea(tabs);
		scroll->setWidgetResizable(true);
		QWidget* inner = new QWidget(scroll);
		QVBoxLayout* vbox = new QVBoxLayout(inner);
		vbox->setSpacing(4);
		populateCheckboxes(vbox, isCompat, defs);
		vbox->addStretch();
		scroll->setWidget(inner);
		tabs->addTab(scroll, tabTitle);
	};

	makeScrollTab(LStr("GMPLYMNU_TITLE", "Gameplay Flags (DMFlags)"), false, getDMFlagsList());
	makeScrollTab(LStr("CMPTMNU_TITLE", "Compatibility Flags"), true, getCompatFlagsList());
	mainLayout->addWidget(tabs, 1);

	QGridLayout* numGrid = new QGridLayout();
	editDm1 = makeUIntEdit(this);
	editDm2 = makeUIntEdit(this);
	editDm3 = makeUIntEdit(this);
	editCompat1 = makeUIntEdit(this);
	editCompat2 = makeUIntEdit(this);

	for (QLineEdit* ed : {editDm1, editDm2, editDm3, editCompat1, editCompat2})
	{
		connect(ed, &QLineEdit::textEdited, this, &FlagEditorDialog::onNumericFieldEdited);
		connect(ed, &QLineEdit::editingFinished, this, &FlagEditorDialog::syncNumericFromValues);
	}

	numGrid->addWidget(new QLabel("dmflags:", this), 0, 0);
	numGrid->addWidget(editDm1, 0, 1);
	numGrid->addWidget(new QLabel("dmflags2:", this), 1, 0);
	numGrid->addWidget(editDm2, 1, 1);
	numGrid->addWidget(new QLabel("dmflags3:", this), 2, 0);
	numGrid->addWidget(editDm3, 2, 1);

	numGrid->addWidget(new QLabel("compatflags:", this), 0, 2);
	numGrid->addWidget(editCompat1, 0, 3);
	numGrid->addWidget(new QLabel("compatflags2:", this), 1, 2);
	numGrid->addWidget(editCompat2, 1, 3);

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	bottomLayout->addLayout(numGrid);
	bottomLayout->addStretch();

	QVBoxLayout* actionCol = new QVBoxLayout();
	actionCol->addStretch();
	QHBoxLayout* btns = new QHBoxLayout();
	QPushButton* btnCancel = new QPushButton(LStr("OPTVAL_CANCEL", "Cancel"), this);
	QPushButton* btnReturn = new QPushButton(LStr("TXT_OK", "OK"), this);
	btnReturn->setDefault(true);
	connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
	connect(btnReturn, &QPushButton::clicked, this, &QDialog::accept);
	btns->addWidget(btnCancel);
	btns->addWidget(btnReturn);
	actionCol->addLayout(btns);
	bottomLayout->addLayout(actionCol);

	mainLayout->addLayout(bottomLayout);

	syncCheckboxesFromValues();
	syncNumericFromValues();
}

uint32_t* FlagEditorDialog::targetRegister(bool isCompat, int setIdx)
{
	return const_cast<uint32_t*>(static_cast<const FlagEditorDialog*>(this)->targetRegister(isCompat, setIdx));
}

const uint32_t* FlagEditorDialog::targetRegister(bool isCompat, int setIdx) const
{
	if (isCompat)
		return (setIdx == 0) ? &flagValues.compatflags : &flagValues.compatflags2;
	if (setIdx == 0)
		return &flagValues.dmflags;
	if (setIdx == 1)
		return &flagValues.dmflags2;
	return &flagValues.dmflags3;
}

void FlagEditorDialog::populateCheckboxes(QVBoxLayout* container, bool isCompat, const std::vector<FlagInfo>& defs)
{
	int currentSet = -1;
	for (const FlagInfo& def : defs)
	{
		if (def.setIdx != currentSet)
		{
			currentSet = def.setIdx;
			QString headerText;
			if (isCompat)
				headerText = (currentSet == 0) ? "compatflags" : "compatflags2";
			else
				headerText = (currentSet == 0) ? "dmflags" : (currentSet == 1 ? "dmflags2" : "dmflags3");

			QLabel* section = new QLabel(headerText, this);
			QFont f = section->font();
			f.setBold(true);
			section->setFont(f);
			section->setContentsMargins(0, currentSet == 0 ? 2 : 10, 0, 2);
			container->addWidget(section);
			sections.push_back({section, {}});
		}

		if (!def.choices.empty())
		{
			QWidget* row = new QWidget(this);
			QHBoxLayout* rowLayout = new QHBoxLayout(row);
			rowLayout->setContentsMargins(0, 1, 0, 1);
			rowLayout->setSpacing(8);

			QLabel* lbl = new QLabel(ensureTrailingColon(def.label), row);
			QComboBox* combo = new QComboBox(row);
			combo->setMinimumWidth(110);
			combo->setFocusPolicy(Qt::StrongFocus);
			combo->installEventFilter(this);
			for (const FlagChoice& choice : def.choices)
				combo->addItem(choice.label, static_cast<unsigned int>(choice.bitVal));
			lbl->setToolTip(def.tooltip);
			combo->setToolTip(def.tooltip);
			row->setToolTip(def.tooltip);

			rowLayout->addWidget(lbl);
			rowLayout->addWidget(combo);
			rowLayout->addStretch(1);
			container->addWidget(row);

			const size_t idx = boundFlags.size();
			boundFlags.push_back({def, isCompat, row, nullptr, combo});
			sections.back().flagIndices.push_back(idx);

			connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, idx](int comboIdx)
			{
				if (updating || comboIdx < 0)
					return;

				const BoundFlag& bf = boundFlags[idx];
				const uint32_t selectedVal = static_cast<uint32_t>(bf.combo->itemData(comboIdx).toUInt());
				uint32_t* reg = targetRegister(bf.isCompat, bf.info.setIdx);
				*reg = (*reg & ~bf.info.bitMask) | (selectedVal & bf.info.bitMask);
				syncNumericFromValues();
			});
		}
		else
		{
			QCheckBox* chk = new QCheckBox(def.label, this);
			chk->setToolTip(def.tooltip);
			container->addWidget(chk);

			const size_t idx = boundFlags.size();
			boundFlags.push_back({def, isCompat, chk, chk, nullptr});
			sections.back().flagIndices.push_back(idx);

			connect(chk, &QCheckBox::toggled, this, [this, idx](bool checked)
			{
				if (updating)
					return;

				const BoundFlag& bf = boundFlags[idx];
				uint32_t* reg = targetRegister(bf.isCompat, bf.info.setIdx);
				if (bf.info.bitMask != 0)
				{
					if (checked)
						*reg = (*reg & ~bf.info.bitMask) | bf.info.bitVal;
					else
						*reg &= ~bf.info.bitMask;
				}
				else
				{
					const bool setBit = bf.info.invert ? !checked : checked;
					if (setBit)
						*reg |= bf.info.bitVal;
					else
						*reg &= ~bf.info.bitVal;
				}

				if (bf.info.bitMask != 0)
					syncCheckboxesFromValues(bf.isCompat ? 1 : 0, bf.info.setIdx);
				syncNumericFromValues();
			});
		}
	}
}

void FlagEditorDialog::syncCheckboxesFromValues(int filterIsCompat, int filterSetIdx)
{
	updating = true;
	for (const BoundFlag& bf : boundFlags)
	{
		if (filterIsCompat >= 0 && (bf.isCompat ? 1 : 0) != filterIsCompat)
			continue;
		if (filterSetIdx >= 0 && bf.info.setIdx != filterSetIdx)
			continue;

		const uint32_t reg = *targetRegister(bf.isCompat, bf.info.setIdx);
		if (bf.combo)
		{
			const uint32_t masked = reg & bf.info.bitMask;
			int matchIdx = bf.combo->findData(static_cast<unsigned int>(masked));
			if (matchIdx < 0)
				matchIdx = 0;
			bf.combo->setCurrentIndex(matchIdx);
		}
		else if (bf.checkbox)
		{
			bool isChecked = false;
			if (bf.info.bitMask != 0)
				isChecked = (reg & bf.info.bitMask) == bf.info.bitVal;
			else
				isChecked = bf.info.invert ? ((reg & bf.info.bitVal) == 0) : ((reg & bf.info.bitVal) != 0);
			bf.checkbox->setChecked(isChecked);
		}
	}
	updating = false;
}

bool FlagEditorDialog::eventFilter(QObject* watched, QEvent* event)
{
	if (event && event->type() == QEvent::Wheel && qobject_cast<QComboBox*>(watched))
	{
		event->ignore();
		return true;
	}
	return QDialog::eventFilter(watched, event);
}

void FlagEditorDialog::syncNumericFromValues()
{
	updating = true;
	for (QLineEdit* ed : {editDm1, editDm2, editDm3, editCompat1, editCompat2})
		ed->setStyleSheet(QString());
	editDm1->setText(QString::number(flagValues.dmflags));
	editDm2->setText(QString::number(flagValues.dmflags2));
	editDm3->setText(QString::number(flagValues.dmflags3));
	editCompat1->setText(QString::number(flagValues.compatflags));
	editCompat2->setText(QString::number(flagValues.compatflags2));
	updating = false;
}

void FlagEditorDialog::onNumericFieldEdited()
{
	if (updating)
		return;

	auto applyField = [](QLineEdit* edit, uint32_t& dst)
	{
		bool ok = false;
		const uint32_t val = parseUIntText(edit->text(), &ok);
		if (ok)
		{
			dst = val;
			edit->setStyleSheet(QString());
		}
		else
		{
			edit->setStyleSheet("border: 1px solid #d94336;");
		}
	};

	applyField(editDm1, flagValues.dmflags);
	applyField(editDm2, flagValues.dmflags2);
	applyField(editDm3, flagValues.dmflags3);
	applyField(editCompat1, flagValues.compatflags);
	applyField(editCompat2, flagValues.compatflags2);

	syncCheckboxesFromValues();
}

void FlagEditorDialog::onSearchTextChanged(const QString& text)
{
	const QString needle = text.trimmed();
	for (const FlagSection& section : sections)
	{
		bool anyVisible = false;
		for (const size_t idx : section.flagIndices)
		{
			const BoundFlag& bf = boundFlags[idx];
			bool match = needle.isEmpty() ||
				bf.info.label.contains(needle, Qt::CaseInsensitive) ||
				bf.info.tooltip.contains(needle, Qt::CaseInsensitive);
			if (!match)
			{
				for (const FlagChoice& choice : bf.info.choices)
				{
					if (choice.label.contains(needle, Qt::CaseInsensitive))
					{
						match = true;
						break;
					}
				}
			}
			if (bf.rowWidget)
				bf.rowWidget->setVisible(match);
			anyVisible = anyVisible || match;
		}
		if (section.header)
			section.header->setVisible(anyVisible);
	}
}

void FlagEditorDialog::onResetDefaults()
{
	flagValues.alwaysApplyDMFlags = false;
	flagValues.dmflags = 0;
	flagValues.dmflags2 = 0;
	flagValues.dmflags3 = 0;
	flagValues.compatflags = 0;
	flagValues.compatflags2 = 0;
	if (chkApplyDM)
		chkApplyDM->setChecked(false);
	syncCheckboxesFromValues();
	syncNumericFromValues();
}
