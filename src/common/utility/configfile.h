/*
** configfile.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
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

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <stdio.h>
#include "files.h"
#include "zstring.h"

class FConfigFile
{
public:
	FConfigFile ();
	FConfigFile (const char *pathname);
	FConfigFile (const FConfigFile &other);
	virtual ~FConfigFile ();

	void ClearConfig ();
	FConfigFile &operator= (const FConfigFile &other);

	bool HaveSections () { return Sections != NULL; }
	void CreateSectionAtStart (const char *name);
	void MoveSectionToStart (const char *name);
	void SetSectionNote (const char *section, const char *note);
	void SetSectionNote (const char *note);
	bool SetSection (const char *section, bool allowCreate=false);
	bool SetFirstSection ();
	bool SetNextSection ();
	const char *GetCurrentSection () const;
	void ClearCurrentSection ();
	bool DeleteCurrentSection ();
	void ClearKey (const char *key);

	bool SectionIsEmpty ();
	bool NextInSection (const char *&key, const char *&value);
	const char *GetValueForKey (const char *key) const;
	void SetValueForKey (const char *key, const char *value, bool duplicates=false);
	void SetValueForKey(const char* key, const FString& value, bool duplicates = false)
	{
		SetValueForKey(key, value.GetChars(), duplicates);
	}

	void EnsureValueForKey (const char *key, const char *value);
	void EnsureValueForKey(const char* key, const FString& value)
	{
		EnsureValueForKey(key, value.GetChars());
	}

	const char *GetPathName () const { return PathName.GetChars(); }
	void ChangePathName (const char *path);

	void LoadConfigFile ();
	bool WriteConfigFile () const;

protected:
	virtual void WriteCommentHeader (FileWriter *file) const;

	uint8_t *ReadLine (TArray<uint8_t> &string, FileReader *file) const;
	bool ReadConfig (FileReader *file);
	static const char *GenerateEndTag(const char *value);
	void RenameSection(const char *oldname, const char *newname) const;

	bool OkayToWrite;
	bool FileExisted;

private:
	struct FConfigEntry
	{
		char *Value;
		FConfigEntry *Next;
		char Key[1];	// + length of key

		void SetValue (const char *val);
	};
	struct FConfigSection
	{
		FString SectionName;
		FConfigEntry *RootEntry;
		FConfigEntry **LastEntryPtr;
		FConfigSection *Next;
		FString Note;
		//char Name[1];	// + length of name
	};

	FConfigSection* Sections = nullptr;
	FConfigSection **LastSectionPtr;
	FConfigSection *CurrentSection;
	FConfigEntry *CurrentEntry;
	FString PathName;

	FConfigSection *FindSection (const char *name) const;
	FConfigEntry *FindEntry (FConfigSection *section, const char *key) const;
	FConfigSection *NewConfigSection (const char *name);
	FConfigEntry *NewConfigEntry (FConfigSection *section, const char *key, const char *value);
	FConfigEntry *ReadMultiLineValue (FileReader *file, FConfigSection *section, const char *key, const char *terminator);
	void SetSectionNote (FConfigSection *section, const char *note);

public:
	class Position
	{
		friend class FConfigFile;

		FConfigSection *Section;
		FConfigEntry *Entry;
	};

	void GetPosition (Position &pos) const;
	void SetPosition (const Position &pos);
};

#endif //__CONFIGFILE_H__
