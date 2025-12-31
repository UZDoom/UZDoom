/*
** s_sndseq.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1994-1996 Raven Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
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
*/

#ifndef __S_SNDSEQ_H__
#define __S_SNDSEQ_H__

#include <stddef.h>
#include "dobject.h"
#include "s_sound.h"

#define MAX_SNDSEQS			(4096)

enum
{
	// Sound channel aliases for sound sequences.
	CHAN_FLOOR = CHAN_WEAPON,
	CHAN_CEILING = CHAN_VOICE,
	CHAN_FULLHEIGHT = CHAN_ITEM,
	CHAN_INTERIOR = CHAN_BODY,
};

typedef enum {
	SEQ_PLATFORM,
	SEQ_DOOR,
	SEQ_ENVIRONMENT,
	SEQ_NUMSEQTYPES,
	SEQ_NOTRANS
} seqtype_t;

struct sector_t;

class DSeqNode : public DObject
{
	DECLARE_CLASS (DSeqNode, DObject)
	HAS_OBJECT_POINTERS
public:
	void Serialize(FSerializer &arc) override;
	void StopAndDestroy ();
	void OnDestroy() override;
	void Tick ();
	void ChangeData (int seqOffset, int delayTics, float volume, FSoundID currentSoundID);
	void AddChoice (int seqnum, seqtype_t type);
	int GetModeNum() const { return m_ModeNum; }
	FName GetSequenceName() const;
	bool IsLoopingSound() const { return m_IsLooping; }

	virtual void MakeSound (int loop, FSoundID id) {}
	virtual void *Source () { return NULL; }
	virtual bool IsPlaying () { return false; }
	virtual DSeqNode *SpawnChild (int seqnum) { return NULL; }

	inline DSeqNode *NextSequence() const { return m_Next; }


protected:
	DSeqNode ();
	DSeqNode (FLevelLocals *l, int sequence, int modenum);

	int32_t *m_SequencePtr;
	int m_Sequence;

	FSoundID m_CurrentSoundID;
	FSoundID m_StopSound;
	int m_DelayTics;
	float m_Volume;
	float m_Atten;
	int m_ModeNum;
	bool m_IsLooping;

	FLevelLocals *Level;
	TArray<int> m_SequenceChoices;
	TObjPtr<DSeqNode*> m_ChildSeqNode;
	TObjPtr<DSeqNode*> m_ParentSeqNode;

private:
	DSeqNode *m_Next, *m_Prev;

	void ActivateSequence (int sequence);

	friend void SN_StopAllSequences (FLevelLocals *Level);
};

void SN_StopAllSequences (FLevelLocals *Level);

struct FSoundSequence
{
	FName	 SeqName;
	FName	 Slot;
	FSoundID StopSound;
	int32_t	 Script[1];	// + more until end of sequence script
};

void S_ParseSndSeq (int levellump);
DSeqNode *SN_StartSequence (AActor *mobj, int sequence, seqtype_t type, int modenum, bool nostop=false);
DSeqNode *SN_StartSequence (AActor *mobj, const char *name, int modenum);
DSeqNode *SN_StartSequence (AActor *mobj, FName seqname, int modenum);
DSeqNode *SN_StartSequence (sector_t *sector, int chan, int sequence, seqtype_t type, int modenum, bool nostop=false);
DSeqNode *SN_StartSequence (sector_t *sector, int chan, const char *name, int modenum);
DSeqNode *SN_StartSequence (sector_t *sec, int chan, FName seqname, int modenum);
DSeqNode *SN_StartSequence (FPolyObj *poly, int sequence, seqtype_t type, int modenum, bool nostop=false);
DSeqNode *SN_StartSequence (FPolyObj *poly, const char *name, int modenum);
DSeqNode *SN_CheckSequence (sector_t *sector, int chan);
void SN_StopSequence (AActor *mobj);
void SN_StopSequence (sector_t *sector, int chan);
void SN_StopSequence (FPolyObj *poly);
bool SN_AreModesSame(int sequence, seqtype_t type, int mode1, int mode2);
bool SN_AreModesSame(FName name, int mode1, int mode2);
void SN_UpdateActiveSequences (FLevelLocals *Level);
ptrdiff_t SN_GetSequenceOffset (int sequence, int32_t *sequencePtr);
void SN_DoStop (FLevelLocals *Level, void *);
void SN_ChangeNodeData (FLevelLocals *Level, int nodeNum, int seqOffset, int delayTics, float volume, int currentSoundID);
FName SN_GetSequenceSlot (int sequence, seqtype_t type);
void SN_MarkPrecacheSounds (int sequence, seqtype_t type);
bool SN_IsMakingLoopingSound (sector_t *sector);

#endif //__S_SNDSEQ_H__
