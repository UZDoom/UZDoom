/*
** sounddata.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class SeqNode native
{
	enum ESeqType 
	{
		PLATFORM,
		DOOR,
		ENVIRONMENT,
		NUMSEQTYPES,
		NOTRANS
	};

	native bool AreModesSameID(int sequence, int type, int mode1);
	native bool AreModesSame(Name name, int mode1);
	native Name GetSequenceName();
	native void AddChoice (int seqnum, int type);
	native static Name GetSequenceSlot (int sequence, int type);
	native static void MarkPrecacheSounds(int sequence, int type);
}

