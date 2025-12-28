/*
** savegamemanager.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2001-2016 Marisa Heit
** Copyright 2010-2020 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
**
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "zstring.h"
#include "tarray.h"

class FGameTexture;
class FSerializer;

// The savegame manager contains too much code that is game specific. Parts are shareable but need more work first.
struct FSaveGameNode
{
	FString SaveTitle;
	FString Filename;
	FString CreationTime;
	FString UUID;
	bool bOldVersion = false;
	bool bMissingWads = false;
	bool bNoDelete = false;
};

struct FSavegameManagerBase
{
protected:
	TArray<FSaveGameNode*> SaveGames;
	FSaveGameNode NewSaveNode;
	int LastSaved = -1;
	int LastAccessed = -1;
	FGameTexture *SavePic = nullptr;

public:
	int WindowSize = 0;
	FString SaveCommentString;
	FSaveGameNode *quickSaveSlot = nullptr;
	virtual ~FSavegameManagerBase();

protected:
	int InsertSaveNode(FSaveGameNode *node);
	virtual void PerformSaveGame(const char *fn, const char *sgdesc) = 0;
	virtual void PerformLoadGame(const char *fn, bool) = 0;
	virtual FString ExtractSaveComment(FSerializer &arc) = 0;
	virtual FString BuildSaveName(const char* prefix, int slot) = 0;
public:
	void NotifyNewSave(const FString &file, const FString &title, bool okForQuicksave, bool forceQuicksave);
	void ClearSaveGames();

	virtual void ReadSaveStrings() = 0;
	void UnloadSaveData();

	int RemoveSaveSlot(int index);
	void LoadSavegame(int Selected);
	void DoSave(int Selected, const char *savegamestring);
	unsigned ExtractSaveData(int index);
	void ClearSaveStuff();
	bool DrawSavePic(int x, int y, int w, int h);
	void DrawSaveComment(FFont *font, int cr, int x, int y, int scalefactor);
	void SetFileInfo(int Selected);
	unsigned SavegameCount();
	FSaveGameNode *GetSavegame(int i);
	void InsertNewSaveNode();
	bool RemoveNewSaveNode();
	int RemoveUUIDSaveSlots();

};

extern FString SavegameFolder;	// specifies a subdirectory for the current IWAD.
FString G_GetSavegamesFolder();
FString G_BuildSaveName(const char* prefix);
