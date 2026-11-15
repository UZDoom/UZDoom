/*
** netstartwindow.h
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
#include <exception>
#include <string>
#include <vector>

class QEventLoop;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

class NetStartWindow : public QDialog
{
	Q_OBJECT
public:
	static void NetInit(const char* message, bool host);
	static void NetMessage(const char* message);
	static void NetConnect(int client, const char* name, unsigned flags, int status);
	static void NetUpdate(int client, int status);
	static void NetDisconnect(int client);
	static void NetProgress(int cur, int limit);
	static void NetDone();
	static void NetClose();
	static bool ShouldStartNet();
	static int GetNetKickClient();
	static int GetNetBanClient();
	static bool NetLoop(bool (*timer_callback)(void*), void* userdata);

protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void onAbort();
	void onForceStart();
	void onTimerTick();

private:
	explicit NetStartWindow(bool host, QWidget* parent = nullptr);
	~NetStartWindow() override;

	void ensureRowCount(int limit);
	void updateRowStatus(int client, int status);
	void queueKick(int client);
	void queueBan(int client);

	static NetStartWindow* Instance;

	bool hosting = false;
	bool shouldStart = false;
	bool exitReason = false;
	int curPlayers = 0;
	int maxPlayers = 0;

	bool (*callback)(void*) = nullptr;
	void* userdata = nullptr;
	std::exception_ptr callbackException;

	std::vector<int> kickQueue;
	std::vector<int> banQueue;

	QLabel* msgLabel = nullptr;
	QLabel* progressLabel = nullptr;
	QTableWidget* table = nullptr;
	QPushButton* btnAbort = nullptr;
	QPushButton* btnStart = nullptr;
	QTimer* timer = nullptr;
	QEventLoop* loop = nullptr;
};
