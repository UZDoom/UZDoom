/*
** netstartwindow.cpp
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

#include "netstartwindow.h"
#include "const.h"

#include "gstrings.h"
#include "version.h"

#include <QApplication>
#include <QCloseEvent>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

NetStartWindow* NetStartWindow::Instance = nullptr;

namespace
{
	QString statusTextForCode(int status)
	{
		switch (status)
		{
		case 1:
			return "CONNECTING";
		case 2:
			return "WAITING";
		case 3:
			return "READY";
		default:
			return {};
		}
	}
}

NetStartWindow::NetStartWindow(bool host, QWidget* parent)
	: QDialog(parent), hosting(host)
{
	setWindowTitle(host
		? QString("%1 - Server Control").arg(GAMENAME)
		: QString("%1 - Lobby").arg(GAMENAME));
	setMinimumSize(460, 420);
	resize(540, 500);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QHBoxLayout* headerRow = new QHBoxLayout();
	msgLabel = new QLabel(LStr("NETMENU_WAITING", "Waiting for other players..."), this);
	msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	QFont mf = msgLabel->font();
	mf.setBold(true);
	msgLabel->setFont(mf);

	progressLabel = new QLabel("0/0", this);
	progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	headerRow->addWidget(msgLabel, 1);
	headerRow->addWidget(progressLabel);
	mainLayout->addLayout(headerRow);

	table = new QTableWidget(0, host ? 5 : 4, this);
	QStringList headers = {"#", "Flags", "Player", "Status"};
	if (host)
		headers << "Actions";
	table->setHorizontalHeaderLabels(headers);
	table->verticalHeader()->setVisible(false);
	table->verticalHeader()->setDefaultSectionSize(28);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);

	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	if (host)
		table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	mainLayout->addWidget(table, 1);

	QHBoxLayout* btnLayout = new QHBoxLayout();
	btnLayout->addStretch();
	btnAbort = new QPushButton(LStr("ACTION_ABORT", "Abort"), this);
	connect(btnAbort, &QPushButton::clicked, this, &NetStartWindow::onAbort);
	btnLayout->addWidget(btnAbort);

	if (host)
	{
		btnStart = new QPushButton(LStr("ACTION_STARTGAME", "Start Game"), this);
		connect(btnStart, &QPushButton::clicked, this, &NetStartWindow::onForceStart);
		btnLayout->addWidget(btnStart);
	}
	mainLayout->addLayout(btnLayout);

	timer = new QTimer(this);
	timer->setInterval(50);
	connect(timer, &QTimer::timeout, this, &NetStartWindow::onTimerTick);
}

NetStartWindow::~NetStartWindow()
{
	if (timer)
		timer->stop();
}

void NetStartWindow::ensureRowCount(int limit)
{
	const int oldRows = table->rowCount();
	if (limit <= oldRows)
		return;

	table->setRowCount(limit);
	const QColor iconColor = palette().color(QPalette::ButtonText);
	for (int row = oldRows; row < limit; ++row)
	{
		table->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
		table->setItem(row, 1, new QTableWidgetItem());
		table->setItem(row, 2, new QTableWidgetItem());
		table->setItem(row, 3, new QTableWidgetItem());

		if (hosting)
		{
			QWidget* actionCell = new QWidget(table);
			QHBoxLayout* actLayout = new QHBoxLayout(actionCell);
			actLayout->setContentsMargins(2, 1, 2, 1);
			actLayout->setSpacing(4);

			QToolButton* btnKick = new QToolButton(actionCell);
			btnKick->setIcon(makeLauncherIcon("kick", iconColor, 14));
			btnKick->setToolTip(LStr("ACTION_KICK", "Kick Player"));
			btnKick->setAutoRaise(true);
			connect(btnKick, &QToolButton::clicked, this, [this, row]() { queueKick(row); });

			QToolButton* btnBan = new QToolButton(actionCell);
			btnBan->setIcon(makeLauncherIcon("ban", iconColor, 14));
			btnBan->setToolTip(LStr("ACTION_BAN", "Ban Player"));
			btnBan->setAutoRaise(true);
			connect(btnBan, &QToolButton::clicked, this, [this, row]() { queueBan(row); });

			btnKick->setEnabled(row != 0);
			btnBan->setEnabled(row != 0);

			actLayout->addWidget(btnKick);
			actLayout->addWidget(btnBan);
			table->setCellWidget(row, 4, actionCell);
		}
	}
}

void NetStartWindow::updateRowStatus(int client, int status)
{
	if (client < 0)
		return;
	ensureRowCount(client + 1);
	if (QTableWidgetItem* it = table->item(client, 3))
		it->setText(statusTextForCode(status));
}

void NetStartWindow::queueKick(int client)
{
	if (client > 0 && std::find(kickQueue.begin(), kickQueue.end(), client) == kickQueue.end())
		kickQueue.push_back(client);
}

void NetStartWindow::queueBan(int client)
{
	if (client > 0 && std::find(banQueue.begin(), banQueue.end(), client) == banQueue.end())
		banQueue.push_back(client);
}

void NetStartWindow::NetInit(const char* message, bool host)
{
	if (!Instance)
	{
		ensureLauncherQtApplication();
		Instance = new NetStartWindow(host);
		Instance->show();
	}
	if (message)
		Instance->msgLabel->setText(QString::fromUtf8(message));
	QApplication::processEvents();
}

void NetStartWindow::NetMessage(const char* message)
{
	if (Instance && message)
	{
		Instance->msgLabel->setText(QString::fromUtf8(message));
		QApplication::processEvents();
	}
}

void NetStartWindow::NetConnect(int client, const char* name, unsigned flags, int status)
{
	if (!Instance || client < 0)
		return;

	Instance->ensureRowCount(client + 1);
	QString flagStr;
	if (flags & 1u)
		flagStr += '*';
	if (flags & 2u)
		flagStr += 'H';

	Instance->table->item(client, 1)->setText(flagStr);
	Instance->table->item(client, 2)->setText(name ? QString::fromUtf8(name) : QString());
	Instance->updateRowStatus(client, status);
	QApplication::processEvents();
}

void NetStartWindow::NetUpdate(int client, int status)
{
	if (!Instance || client < 0)
		return;
	Instance->updateRowStatus(client, status);
	QApplication::processEvents();
}

void NetStartWindow::NetDisconnect(int client)
{
	if (!Instance || client < 0 || client >= Instance->table->rowCount())
		return;
	Instance->table->item(client, 1)->setText(QString());
	Instance->table->item(client, 2)->setText(QString());
	Instance->table->item(client, 3)->setText(QString());
	QApplication::processEvents();
}

void NetStartWindow::NetProgress(int cur, int limit)
{
	if (!Instance)
		return;
	Instance->curPlayers = cur;
	Instance->maxPlayers = limit;
	Instance->ensureRowCount(limit);
	if (limit > 0)
		Instance->progressLabel->setText(QString("%1/%2").arg(cur).arg(limit));
	QApplication::processEvents();
}

void NetStartWindow::NetDone()
{
	if (!Instance)
		return;
	NetStartWindow* win = Instance;
	Instance = nullptr;
	win->timer->stop();
	win->hide();
	delete win;
}

void NetStartWindow::NetClose()
{
	if (Instance)
		Instance->onAbort();
}

bool NetStartWindow::ShouldStartNet()
{
	return Instance ? Instance->shouldStart : false;
}

int NetStartWindow::GetNetKickClient()
{
	if (!Instance || Instance->kickQueue.empty())
		return -1;
	int c = Instance->kickQueue.back();
	Instance->kickQueue.pop_back();
	return c;
}

int NetStartWindow::GetNetBanClient()
{
	if (!Instance || Instance->banQueue.empty())
		return -1;
	int c = Instance->banQueue.back();
	Instance->banQueue.pop_back();
	return c;
}

bool NetStartWindow::NetLoop(bool (*timer_callback)(void*), void* userdata)
{
	if (!Instance)
		return false;

	Instance->callback = timer_callback;
	Instance->userdata = userdata;
	Instance->callbackException = {};

	QEventLoop eventLoop;
	Instance->loop = &eventLoop;
	Instance->timer->start();
	eventLoop.exec();
	Instance->timer->stop();
	Instance->loop = nullptr;

	Instance->callback = nullptr;
	Instance->userdata = nullptr;

	if (Instance->callbackException)
		std::rethrow_exception(Instance->callbackException);

	return Instance->exitReason;
}

void NetStartWindow::closeEvent(QCloseEvent* event)
{
	onAbort();
	event->accept();
}

void NetStartWindow::onAbort()
{
	exitReason = false;
	if (timer)
		timer->stop();
	if (loop)
		loop->quit();
}

void NetStartWindow::onForceStart()
{
	shouldStart = true;
}

void NetStartWindow::onTimerTick()
{
	if (!callback)
		return;

	bool finished = false;
	try
	{
		finished = callback(userdata);
	}
	catch (...)
	{
		callbackException = std::current_exception();
		exitReason = false;
		timer->stop();
		if (loop)
			loop->quit();
		return;
	}

	if (finished)
	{
		exitReason = true;
		timer->stop();
		if (loop)
			loop->quit();
	}
}
