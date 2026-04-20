/*
** loadsavemenu.cpp
**
** The load game and save game menus
**
**---------------------------------------------------------------------------
**
** Copyright 2001-2016 Marisa Heit
** Copyright 2010-2020 Christoph Oelckers
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

#include "doommenu.h"
#include "version.h"
#include "g_game.h"
#include "m_png.h"
#include "filesystem.h"
#include "v_text.h"
#include "gstrings.h"
#include "serializer.h"
#include "vm.h"
#include "i_system.h"
#include "v_video.h"
#include "fs_findfile.h"
#include "v_draw.h"

// Save name length limit for old binary formats.
#define OLDSAVESTRINGSIZE		24

EXTERN_CVAR(Int, save_sort_order)

//=============================================================================
//
// M_ReadSaveStrings
//
// Find savegames and read their titles
//
//=============================================================================

bool CheckGZDoomSaveCompat(FString &engine, FString &software)
{
	if(software.IndexOf("GZDoom g") == 0)
	{
		if(software.Len() < 11) return false; // GZDoom g????????

		// GZDoom g4
		int v0 = software[8] - '0';

		//GZDoom g4.
		// minsavever is 4556, so gzdoom 4.0.0, don't allow gzdoom g3.x, g5.x or g4x.y
		if(v0 != 4 || software[9] != '.')
		{
			return false;
		}

		//GZDoom g4.#
		int v1 = software[10] - '0';

		if(software[11] >= '0' && software[11] <= '9')
		{
			//GZDoom g4.##
			v1 = (v1 * 10) + (software[11] - '0');
		}

		if(v1 > 14)
		{
			return false; // GZDoom 4.15+, don't allow save import
		}

		if(v1 == 14)
		{
			if(software.Len() > 13)
			{
				//GZDoom g4.14.#
				int v2 = software[13] - '0';
				if(software.Len() > 14 && software[14] >= '0' && software[14] <= '9')
				{
					//GZDoom g4.14.##, don't allow
					return false;
				}
				else if(v2 > 2)
				{
					//GZDoom g4.14.3+, don't allow
					return false;
				}
				/*
				else
				{
					//GZDoom g4.14.0 / GZDoom g4.14.1 / GZDoom g4.14.2, allow
				}
				*/
			}
			/*
			else
			{
				//GZDoom g4.14.0, allow
			}
			*/
		}
		/*
		else
		{
		//GZDoom g4.0.0 - GZDoom g4.13.2, allow
		}
		*/
	}
	else
	{
		return false;
	}

	return true;
}

void FSavegameManager::ReadSaveStrings()
{
	// re-read list if forced to sort again
	static int old_save_sort_order = 0;
	if (old_save_sort_order != save_sort_order)
	{
		ClearSaveGames();
		old_save_sort_order = save_sort_order;
	}

	if (SaveGames.Size() == 0)
	{
		FString filter;

		LastSaved = LastAccessed = -1;
		quickSaveSlot = nullptr;
		FileSys::FileList list;
		if (FileSys::ScanDirectory(list, G_GetSavegamesFolder().GetChars(), "*." SAVEGAME_EXT, true))
		{
			for (auto& entry : list)
			{
				std::unique_ptr<FResourceFile> savegame(FResourceFile::OpenResourceFile(entry.FilePath.c_str(), true));
				if (savegame != nullptr)
				{
					bool oldVer = false;
					bool missing = false;
					auto info = savegame->FindEntry("info.json");
					if (info < 0)
					{
						// savegame info not found. This is not a savegame so leave it alone.
						continue;
					}
					auto data = savegame->Read(info);
					FSerializer arc;
					if (arc.OpenReader(data.string(), data.size()))
					{
						int savever = 0;
						arc("Save Version", savever);
						FString engine = arc.GetString("Engine");
						FString iwad = arc.GetString("Game WAD");
						FString title = arc.GetString("Title");
						FString creationtime = arc.GetString("Creation Time");
						FString uuid = arc.GetString("GameUUID");

						TArray<FString> allowLoadIn;

						if(arc.HasKey("AllowLoadIn"))
						{
							arc("AllowLoadIn", allowLoadIn);
						}

						#if LOAD_GZDOOM_4142_SAVES
						FString software = arc.GetString("Software");

						if(engine.Compare("GZDOOM") == 0)
						{
							if(!CheckGZDoomSaveCompat(engine, software))
							{
								continue;
							}
						}
						else
						#endif
						if ((engine.CompareNoCase(GAMESIG) != 0 && allowLoadIn.FindNoCase(GAMESIG) == allowLoadIn.Size()) || savever > SAVEVER)
						{
							// different engine or newer version:
							// not our business. Leave it alone.
							continue;
						}

						if (savever < MINSAVEVER)
						{
							// old, incompatible savegame. List as not usable.
							oldVer = true;
						}
						else if (iwad.CompareNoCase(fileSystem.GetResourceFileName(fileSystem.GetIwadNum())) == 0)
						{
							missing = !G_CheckSaveGameWads(arc, false);
						}
						else
						{
							// different game. Skip this.
							continue;
						}

						FSaveGameNode *node = new FSaveGameNode;
						node->Filename = entry.FilePath.c_str();
						node->UUID = uuid;
						node->bOldVersion = oldVer;
						node->bMissingWads = missing;
						node->SaveTitle = title;
						node->CreationTime = creationtime;
						InsertSaveNode(node);
					}
				}
			}
		}
	}
}

//=============================================================================
//
// Loads the savegame
//
//=============================================================================

void FSavegameManager::PerformLoadGame(const char *fn, bool flag)
{
	G_LoadGame(fn, flag);
}

//=============================================================================
//
// 
//
//=============================================================================

void FSavegameManager::PerformSaveGame(const char *fn, const char *savegamestring)
{
	G_SaveGame(fn, savegamestring);
}

//=============================================================================
//
//
//
//=============================================================================

FString FSavegameManager::ExtractSaveComment(FSerializer &arc)
{
	FString comment;

	FString time = arc.GetString("Creation Time");
	FString pcomment = arc.GetString("Comment");

	comment = time;
	if (time.Len() > 0) comment += "\n";
	comment += pcomment;
	return comment;
}

FString FSavegameManager::BuildSaveName(const char* prefix, int slot)
{
	return G_BuildSaveName(FStringf("%s%02d", prefix, slot).GetChars());
}

//=============================================================================
//
//
//
//=============================================================================

FSavegameManager savegameManager;

DEFINE_ACTION_FUNCTION(FSavegameManager, GetManager)
{
	PARAM_PROLOGUE;
	ACTION_RETURN_POINTER(&savegameManager);
}

