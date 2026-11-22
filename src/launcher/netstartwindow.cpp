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
#include <QPointer>
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
			return LStr("NETMENU_STATUS_CONNECTING", "CONNECTING");
		case 2:
			return LStr("NETMENU_STATUS_WAITING", "WAITING");
		case 3:
			return LStr("NETMENU_STATUS_READY", "READY");
		default:
			return {};
		}
	}
}

NetStartWindow::NetStartWindow(bool host, QWidget* parent)
	: QDialog(parent), hosting(host)
{
	setWindowTitle(host
		? QString("%1 - %2").arg(GAMENAME, LStr("NETMENU_SERVER_CONTROL", "Server Control"))
		: QString("%1 - %2").arg(GAMENAME, LStr("NETMENU_LOBBY", "Lobby")));
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
	QStringList headers = {
		QStringLiteral("#"),
		LStr("NETMENU_COL_FLAGS", "Flags"),
		LStr("NETMENU_COL_PLAYER", "Player"),
		LStr("NETMENU_COL_STATUS", "Status")
	};
	if (host)
		headers << LStr("NETMENU_COL_ACTIONS", "Actions");
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

			btnKick->setEnabled(false);
			btnBan->setEnabled(false);

			actLayout->addWidget(btnKick);
			actLayout->addWidget(btnBan);
			table->setCellWidget(row, 4, actionCell);
			kickButtons.push_back(btnKick);
			banButtons.push_back(btnBan);
		}
	}
}

void NetStartWindow::setRowActionsEnabled(int row, bool enabled)
{
	if (!hosting || row < 0)
		return;
	const bool allow = enabled && row != 0;
	if (static_cast<size_t>(row) < kickButtons.size() && kickButtons[row])
		kickButtons[row]->setEnabled(allow);
	if (static_cast<size_t>(row) < banButtons.size() && banButtons[row])
		banButtons[row]->setEnabled(allow);
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
	else
	{
		Instance->aborted = false;
		Instance->exitReason = false;
		if (!Instance->isVisible())
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
	Instance->setRowActionsEnabled(client, true);
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
	Instance->setRowActionsEnabled(client, false);
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
	if (win->timer)
		win->timer->stop();
	win->hide();
	if (win->loop)
	{
		win->loop->quit();
		win->deleteLater();
	}
	else
	{
		delete win;
	}
}

void NetStartWindow::NetClose()
{
	if (Instance)
	{
		Instance->onAbort();
		Instance->hide();
	}
}

bool NetStartWindow::ShouldStartNet()
{
	return Instance ? Instance->shouldStart : false;
}

int NetStartWindow::GetNetKickClient()
{
	if (!Instance || Instance->kickQueue.empty())
		return -1;
	int c = Instance->kickQueue.front();
	Instance->kickQueue.pop_front();
	return c;
}

int NetStartWindow::GetNetBanClient()
{
	if (!Instance || Instance->banQueue.empty())
		return -1;
	int c = Instance->banQueue.front();
	Instance->banQueue.pop_front();
	return c;
}

bool NetStartWindow::NetLoop(bool (*timer_callback)(void*), void* userdata)
{
	if (!Instance || Instance->aborted)
		return false;

	QPointer<NetStartWindow> win(Instance);
	win->callback = timer_callback;
	win->userdata = userdata;
	win->callbackException = {};
	win->exitReason = false;

	QEventLoop eventLoop;
	win->loop = &eventLoop;
	win->timer->start();
	eventLoop.exec();

	if (!win)
		return false;

	win->timer->stop();
	win->loop = nullptr;

	win->callback = nullptr;
	win->userdata = nullptr;

	const std::exception_ptr pendingEx = win->callbackException;
	const bool result = win->exitReason && !win->aborted;
	if (pendingEx)
		std::rethrow_exception(pendingEx);

	return result;
}

void NetStartWindow::reject()
{
	onAbort();
	QDialog::reject();
}

void NetStartWindow::closeEvent(QCloseEvent* event)
{
	onAbort();
	event->accept();
}

void NetStartWindow::onAbort()
{
	aborted = true;
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
	if (!callback || inCallback || aborted)
		return;

	QPointer<NetStartWindow> self(this);
	inCallback = true;
	if (timer)
		timer->stop();

	bool finished = false;
	try
	{
		finished = callback(userdata);
	}
	catch (...)
	{
		if (self)
		{
			self->inCallback = false;
			self->callbackException = std::current_exception();
			self->exitReason = false;
			if (self->timer)
				self->timer->stop();
			if (self->loop)
				self->loop->quit();
		}
		return;
	}

	if (!self)
		return;

	self->inCallback = false;

	if (self->aborted)
	{
		self->exitReason = false;
		if (self->loop)
			self->loop->quit();
		return;
	}

	if (finished)
	{
		self->exitReason = true;
		if (self->loop)
			self->loop->quit();
		return;
	}

	if (self->loop && self->timer)
		self->timer->start();
}
