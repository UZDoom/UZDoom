/*
** p_setup.h
**
** Setup a game, startup stuff.
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1994-1996 Raven Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2002-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#ifndef __P_SETUP__
#define __P_SETUP__

#include "cmdlib.h"
#include "doomdata.h"
#include "filesystem.h"
#include "nodebuild.h"
#include "r_defs.h"

struct MapData
{
private:
	struct ResourceHolder
	{
		FResourceFile *data = nullptr;

		~ResourceHolder()
		{
			delete data;
		}

		ResourceHolder &operator=(FResourceFile *other) { data = other; return *this; }
		FResourceFile *operator->() { return data; }
		operator FResourceFile *() const { return data; }
	};

	// The order of members here is important
	// Resource should be destructed after MapLumps as readers may share FResourceLump objects
	// For example, this is the case when map .wad is loaded from .pk3 file
	ResourceHolder resource;

	struct MapLump
	{
		char Name[8] = { 0 };
		FileReader Reader;
	} MapLumps[ML_MAX];
	FileReader nofile;
public:
	bool HasBehavior = false;
	bool isText = false;
	bool InWad = false;
	int lumpnum = -1;

	/*
	void Seek(unsigned int lumpindex)
	{
		if (lumpindex<countof(MapLumps))
		{
			file = &MapLumps[lumpindex].Reader;
			file->Seek(0, FileReader::SeekSet);
		}
	}
	*/

	FileReader &Reader(unsigned int lumpindex)
	{
		if (lumpindex < countof(MapLumps))
		{
			auto &file = MapLumps[lumpindex].Reader;
			file.Seek(0, FileReader::SeekSet);
			return file;
		}
		return nofile;
	}

	void Read(unsigned int lumpindex, void * buffer, int size = -1)
	{
		if (lumpindex<countof(MapLumps))
		{
			if (size == -1) size = Size(lumpindex);
			if (size > 0)
			{
				auto &file = MapLumps[lumpindex].Reader;
				file.Seek(0, FileReader::SeekSet);
				file.Read(buffer, size);
			}
		}
	}

	TArray<uint8_t> Read(unsigned lumpindex)
	{
		TArray<uint8_t> buffer(Size(lumpindex), true);
 		Read(lumpindex, buffer.Data(), (int)buffer.Size());
		return buffer;
	}

	uint32_t Size(unsigned int lumpindex)
	{
		if (lumpindex<countof(MapLumps) && MapLumps[lumpindex].Reader.isOpen())
		{
			return (uint32_t)MapLumps[lumpindex].Reader.GetLength();
		}
		return 0;
	}

	bool CheckName(unsigned int lumpindex, const char *name)
	{
		if (lumpindex < countof(MapLumps))
		{
			return !strnicmp(MapLumps[lumpindex].Name, name, 8);
		}
		return false;
	}

	void GetChecksum(uint8_t cksum[16]);

	friend class MapLoader;
	friend MapData *P_OpenMapData(const char * mapname, bool justcheck);

};

MapData * P_OpenMapData(const char * mapname, bool justcheck);
bool P_CheckMapData(const char * mapname);

void P_SetupLevel (FLevelLocals *Level, int position, bool newGame);
void P_LoadLightmap(MapData *map);

void P_FreeLevelData(bool fullgc = true);

// Called by startup code.
void P_Init (void);

struct line_t;
struct maplinedef_t;

int GetUDMFInt(FLevelLocals *Level, int type, int index, FName key);
double GetUDMFFloat(FLevelLocals *Level, int type, int index, FName key);
FString GetUDMFString(FLevelLocals *Level, int type, int index, FName key);

void FixMinisegReferences();
void FixHoles();
void ReportUnpairedMinisegs();

#endif
