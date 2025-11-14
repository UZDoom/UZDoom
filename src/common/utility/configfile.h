/*
** configfile.h
**
**---------------------------------------------------------------------------
** Copyright 1998-2008 Randy Heit
** All rights reserved.
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

	class Tokens
	{
	public:
		static const inline FString path = FString("Path");
		static const inline FString recursive_path = FString("RecursivePath");
		static const inline FString name = FString("Name");
		static const inline FString command = FString("Command");
		static const inline FString empty = FString("");
		static const inline FString iwad_search_directories = FString("IWADSearch.Directories");
	};

	bool HaveSections() const { return Sections != NULL; }
	void CreateSectionAtStart (const FString& name);
	void MoveSectionToStart (const FString& name);
	void SetSectionNote (const FString& section, const FString& note);
	void SetSectionNote (const FString& note);
	bool SetSection (const FString& section, bool allowCreate=false);
	bool SetFirstSection ();
	bool SetNextSection ();
	const char *GetCurrentSection () const;
	void ClearCurrentSection ();
	bool DeleteCurrentSection ();
	void ClearKey (const FString& key);

	bool SectionIsEmpty ();
	bool NextInSection (FString& key, FString& value);
	const FString& GetValueForKey (const FString& key) const;
	void SetValueForKey(const FString& key, const FString& value, bool duplicates = false);

	const FString& GetPathName () const { return PathName; }
	void ChangePathName (const FString& path);

	void LoadConfigFile ();
	bool WriteConfigFile () const;

protected:
	virtual void WriteCommentHeader (FileWriter *file) const;

	uint8_t *ReadLine (TArray<uint8_t> &string, FileReader *file) const;
	bool ReadConfig (FileReader *file);
	static FString GenerateEndTag(const FString& value);
	void RenameSection(const FString& oldname, const FString& newname) const;

	bool OkayToWrite;
	bool FileExisted;

private:
	struct FConfigEntry final
	{
		FConfigEntry* Next = nullptr;

		FString Key;
		FString Value;

		void SetValue (const FString& val);
		FConfigEntry() = default;
		~FConfigEntry() = default;
	};
	struct FConfigSection
	{
		FString SectionName;
		FConfigEntry* RootEntry = nullptr;
		FConfigEntry** LastEntryPtr = nullptr;
		FConfigSection* Next = nullptr;
		FString Note;
		//char Name[1];	// + length of name
	};

	FConfigSection* Sections = nullptr;
	FConfigSection **LastSectionPtr;
	FConfigSection *CurrentSection = nullptr;
	FConfigEntry *CurrentEntry = nullptr;
	FString PathName;

	FConfigSection *FindSection (const FString& name) const;
	FConfigEntry *FindEntry (FConfigSection* section, const FString& key) const;
	FConfigSection *NewConfigSection (const FString& name);
	FConfigEntry *NewConfigEntry (FConfigSection* section, const FString& key, const FString& value);
	FConfigEntry *ReadMultiLineValue (FileReader* file, FConfigSection *section, const FString& key, const FString& terminator);
	void SetSectionNote (FConfigSection* section, const FString& note);

public:
	class Position
	{
		friend class FConfigFile;

		FConfigSection* Section = nullptr;
		FConfigEntry* Entry = nullptr;
	};

	void GetPosition (Position &pos) const;
	void SetPosition (const Position &pos);
};

#endif //__CONFIGFILE_H__
