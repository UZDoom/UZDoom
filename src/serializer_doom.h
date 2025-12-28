/*
** serializer_doom.h
**
** Savegame wrapper around RapidJSON
**
**---------------------------------------------------------------------------
**
** Copyright 2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

#include "serializer.h"

class player_t;
class DBehavior;
struct sector_t;
struct line_t;
struct side_t;
struct vertex_t;
struct usercmd_t;
class PClassActor;
struct FStrifeDialogueNode;
struct FState;
struct FDoorAnimation;
struct FPolyObj;
struct FInterpolator;
struct FLevelLocals;
class FDecalBase;

class FDoomSerializer : public FSerializer
{
	
	void CloseReaderCustom() override;
	
public:
	FLevelLocals* Level;

	FDoomSerializer(FLevelLocals *l) : Level(l)
	{
		SetUniqueSoundNames();
	}

	FSerializer &Sprite(const char *key, int32_t &spritenum, int32_t *def) override;
	FSerializer& StatePointer(const char* key, void* ptraddr, bool *res) override;

};

FSerializer &SerializeArgs(FSerializer &arc, const char *key, int *args, int *defargs, int special);
FSerializer &SerializeTerrain(FSerializer &arc, const char *key, int &terrain, int *def = nullptr);

FSerializer& Serialize(FSerializer& arc, const char* key, char& value, char* defval);
FSerializer &Serialize(FSerializer &arc, const char *key, usercmd_t &cmd, usercmd_t *def);
FSerializer &Serialize(FSerializer &arc, const char *key, FInterpolator &rs, FInterpolator *def);
FSerializer& Serialize(FSerializer& arc, const char* key, struct FStandaloneAnimation& value, struct FStandaloneAnimation* defval);
FSerializer& Serialize(FSerializer& arc, const char* key, TMap<FName, TObjPtr<DBehavior*>>& value, TMap<FName, TObjPtr<DBehavior*>>* def);

template<> FSerializer &Serialize(FSerializer &arc, const char *key, FPolyObj *&value, FPolyObj **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, sector_t *&value, sector_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, const FPolyObj *&value, const FPolyObj **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, const sector_t *&value, const sector_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, player_t *&value, player_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, line_t *&value, line_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, side_t *&value, side_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, vertex_t *&value, vertex_t **defval);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, PClassActor *&clst, PClassActor **def);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, FStrifeDialogueNode *&node, FStrifeDialogueNode **def);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, FString *&pstr, FString **def);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, FDoorAnimation *&pstr, FDoorAnimation **def);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, char *&pstr, char **def);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, FLevelLocals *&font, FLevelLocals **def);
FSerializer &Serialize(FSerializer &arc, const char *key, FState *&state, FState **def, bool *retcode);
template<> FSerializer &Serialize(FSerializer &arc, const char *key, FDecalBase *&decal, FDecalBase **def);
template<> inline FSerializer &Serialize(FSerializer &arc, const char *key, FState *&state, FState **def)
{
	return Serialize(arc, key, state, def, nullptr);
}
