/*
** i_net.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 by id Software, Inc.
** Copyright 1999-2016 Marisa Heit
** Copyright 2009-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** Alternatively the following applies:
**
** This source is available for distribution and/or modification
** only under the terms of the DOOM Source Code License as
** published by id Software. All rights reserved.
**
** The source is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
** for more details.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __I_NET_H__
#define __I_NET_H__

#include <stdint.h>
#include "m_argv.h"
#include "tarray.h"

inline constexpr size_t MAXPLAYERS = 64u;

EXTERN_FARG(host);
EXTERN_FARG(join);

enum ENetConstants
{
	BACKUPTICS = 35 * 5,	// Remember up to 5 seconds of data.
	MAXTICDUP = 3,
	MAXSENDTICS = 35 * 1,	// Only send up to 1 second of data at a time.
	STABILITYTICS = 17,
	LOCALCMDTICS = (BACKUPTICS * MAXTICDUP),
	MAX_MSGLEN = 14000,
};

enum ENetCommand
{
	CMD_NONE,
	CMD_SEND,
	CMD_GET,
};

enum ENetFlags
{
	NCMD_EXIT = 0x80,		// Client has left the game
	NCMD_RETRANSMIT = 0x40,		// 
	NCMD_SETUP = 0x20,		// Guest is letting the host know who it is
	NCMD_LEVELREADY = 0x10,		// After loading a level, guests send this over to the host who then sends it back after all are received
	NCMD_QUITTERS = 0x08,		// Client is getting info about one or more players quitting
	NCMD_COMPRESSED = 0x04,		// Remainder of packet is compressed
	NCMD_LATENCYACK = 0x02,		// A latency packet was just read, so let the sender know.
	NCMD_LATENCY = 0x01,		// Latency packet, used for measuring RTT.		
};

struct FClientStack : public TArray<int>
{
	inline bool InGame(int i) const { return Find(i) < Size(); }

	void operator+=(const int i)
	{
		if (!InGame(i))
			SortedInsert(i);
	}

	void operator-=(const int i)
	{
		Delete(Find(i));
	}
};

extern bool netgame, multiplayer;
extern int consoleplayer;
extern int Net_Arbitrator;
extern FClientStack NetworkClients;
extern uint8_t NetBuffer[MAX_MSGLEN];
extern size_t NetBufferLength;
extern uint8_t TicDup;
extern int RemoteClient;
extern int MaxClients;

bool I_InitNetwork();
void I_ClearClient(size_t client);
void I_NetCmd(ENetCommand cmd);
void I_NetDone();
void HandleIncomingConnection();
void CloseNetwork();

#endif
