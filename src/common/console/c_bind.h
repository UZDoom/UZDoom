/*
** c_bind.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#ifndef __C_BINDINGS_H__
#define __C_BINDINGS_H__

#include "keydef.h"
#include "zstring.h"
#include "tarray.h"

struct event_t;
class FConfigFile;
class FCommandLine;

void C_NameKeys (char *str, int first, int second);
FString C_NameKeys (int *keys, int count, bool colors = false);

class FKeyBindings
{
	FString Binds[NUM_KEYS];

public:
	void PerformBind(FCommandLine &argv, const char *msg);
	bool DoKey(event_t *ev);
	void ArchiveBindings(FConfigFile *F, const char *matchcmd = NULL);
	int  GetKeysForCommand (const char *cmd, int *first, int *second);
	TArray<int> GetKeysForCommand (const char *cmd);
	void UnbindACommand (const char *str);
	void UnbindAll (const TArray<int> *filter = nullptr);
	void UnbindKey(const char *key);
	void DoBind (const char *key, const char *bind);
	void DefaultBind(const char *keyname, const char *cmd);

	void SetBind(unsigned int key, const char *bind, bool override = true)
	{
		if (!override && Binds[key].IsNotEmpty()) return;
		if (key < NUM_KEYS) Binds[key] = bind;
	}

	const FString &GetBinding(unsigned int index) const
	{
		return Binds[index];
	}

	const char *GetBind(unsigned int index) const
	{
		if (index < NUM_KEYS)
		{
			auto c = Binds[index].GetChars();
			if (*c) return c;
		}
		return NULL;
	}

	const char *GetBind(const char *key);
};

extern FKeyBindings Bindings;
extern FKeyBindings DoubleBindings;
extern FKeyBindings AutomapBindings;


bool C_DoKey (event_t *ev, FKeyBindings *binds, FKeyBindings *doublebinds);

// Stuff used by the customize controls menu
void C_SetDefaultBindings (const TArray<int> *filter = nullptr);
void C_UnbindAll (const TArray<int> *filter = nullptr);

extern const char *KeyNames[];

struct FKeyAction
{
	FString mTitle;
	FString mAction;
};

struct FKeySection
{
	FString mTitle;
	FString mSection;
	TArray<FKeyAction> mActions;
};
extern TArray<FKeySection> KeySections;

#endif //__C_BINDINGS_H__

