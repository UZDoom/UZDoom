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
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: LicenseRef-Doom-Source-License
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
inline constexpr int SMAXPLAYERS = static_cast<int>(MAXPLAYERS);

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

enum ENetPacketType : uint8_t
{
	NCMD_GAME_DATA,		// Clients are sending gameplay data to each other.
	NCMD_CONNECT,		// Guest is letting the host know who it is
	NCMD_DISCONNECT,	// Client has left the game
	NCMD_LEVEL_READY,	// After loading a level, guests send this over to the host who then sends it back after all are received
	NCMD_LATENCY,		// Latency packet, used for measuring RTT.
	NCMD_LATENCY_ACK,	// A latency packet was just read, so let the sender know.

	NCMD_FLAG_START = 1 << 5,	// Packet types cannot reach this number, otherwise they start interfering with the bit flags.

	NCMD_FLAG_QUITTERS		= NCMD_FLAG_START,		// Client is getting info about one or more players quitting
	NCMD_FLAG_RETRANSMIT	= NCMD_FLAG_START << 1,	// Some information was missing and must be sent back over.
	NCMD_FLAG_COMPRESSED	= NCMD_FLAG_START << 2,	// Packet must be decompressed to read correctly.

	NCMD_FLAGS				= NCMD_FLAG_QUITTERS | NCMD_FLAG_RETRANSMIT | NCMD_FLAG_COMPRESSED
};

struct FVerificationError
{
	enum EVerifyError : uint8_t
	{
		VE_NONE,
		VE_ENGINE,
		VE_FILE_UNKNOWN,
		VE_FILE_MISSING,
		VE_FILE_ORDER,
	};

	EVerifyError Error = VE_NONE;
	uint8_t Major = 0u, Minor = 0u, Revision = 0u;
	uint8_t NetMajor = 0u, NetMinor = 0u, NetRevision = 0u;
	TArray<FString> UnknownFiles = {};
	TArray<FString> ExpectedOrder = {};
	// Since the guest didn't load these, we have no checksum to fetch the proper name, so send over our own.
	TArray<FString> MissingFiles = {};
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
uint8_t GetPacketFlags(uint8_t flags);
ENetPacketType GetPacketType(uint8_t type);

void StartNetworkLean();
bool IsNetworkStartedLean();

#endif
