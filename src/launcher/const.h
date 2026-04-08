/*
** const.h
**
** Defines the consts (and flags) to be used by the launcher
**
**---------------------------------------------------------------------------
**
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#pragma once

#include "gstrings.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// defined directories
inline std::string EXEC_DIR;
inline std::string ROOT_DIR;
inline std::string PROFILE_DIR;

// config file path
inline std::string CONFIG_FILE;

// default language is u.s english
inline std::string_view DEFAULT_LANG = "default";

// default theme is dark imgui
inline std::string_view DEFAULT_THEME = "dark";

inline std::string VERSION = "5.0.0";

// this scales all UI elements too
inline float FONT_SIZE = 22.0f;

// helper to better sort the data
struct WadInfo
{
	std::string title;
	std::string author;
	std::string releaseDate;
	bool        isIWAD = false; // force if wrong detection
};

// list 99% of versions is enough
inline const std::unordered_map<std::string, WadInfo> wadDatabase = {
	// DOOM Shareware
	{"90facab21eede7981be10790e3f82da2",{"Doom (Shareware 1.0)", "id Software", "10-12-1993", true}                                        },
	{"cea4989df52b65f4d481b706234a3dca",
     {"Doom (Shareware 1.1)", "id Software", "15-12-1993", true}                                                      }, //  there are two versions
	{"52cbc8882f445573ce421fa5453513c1",                   {"Doom (Shareware 1.1)", "id Software", "16-12-1993", true}},
	{"2a380f28e813fb0989cae5e4762ebb4c",                   {"Doom (Shareware 1.2)", "id Software", "04-02-1994", true}},
	{"30aa5beb9e5ebfbbe1e1765561c08f38",
     {"Doom (Shareware 1.2)", "id Software", "17-02-1994", true}                                                      }, //  there are two versions
	{"17aebd6b5f2ed8ce07aa526a32af8d99",                  {"Doom (Shareware 1.25)", "id Software", "21-04-1994", true}},
	{"a21ae40c388cb6f2c3cc1b95589ee693",                   {"Doom (Shareware 1.4)", "id Software", "28-06-1994", true}},
	{"e280233d533dcc28c1acd6ccdc7742d4",                   {"Doom (Shareware 1.5)", "id Software", "08-07-1994", true}},
	{"762fd6d4b960d4b759730f01387a50a1",                   {"Doom (Shareware 1.6)", "id Software", "03-08-1994", true}},
	{"c428ea394dc52835f2580d5bfd50d76f",                 {"Doom (Shareware 1.666)", "id Software", "30-08-1994", true}},
	{"5f4eb849b1af12887dec04a2a12e5e62",                   {"Doom (Shareware 1.8)", "id Software", "10-12-1994", true}},
	{"f0cefca49926d00903cf57551d901abe",                   {"Doom (Shareware 1.9)", "id Software", "01-02-1995", true}},

	// DOOM Registered
	{"740901119ba2953e3c7f3764eca6e128",                  {"Doom (Registered 0.2)", "id Software", "04-02-1993", true}},
	{"dae9b1eea1a8e090fdfa5707187f4a43",                  {"Doom (Registered 0.3)", "id Software", "28-02-1993", true}},
	{"b6afa12a8b22e2726a8ff5bd249223de",                  {"Doom (Registered 0.4)", "id Software", "03-04-1993", true}},
	{"9c877480b8ef33b7074f1f0c07ed6487",                  {"Doom (Registered 0.5)", "id Software", "23-05-1993", true}},
	{"049e32f18d9c9529630366cfc72726ea",                 {"Doom (Registered Beta)", "id Software", "04-10-1993", true}},

	{"981b03e6d1dc033301aa3095acc437ce",                  {"Doom (Registered 1.1)", "id Software", "16-12-1993", true}},
	{"792fd1fea023d61210857089a7c1e351",                  {"Doom (Registered 1.2)", "id Software", "17-02-1994", true}},
	{"54978d12de87f162b9bcc011676cb3c0",                {"Doom (Registered 1.666)", "id Software", "01-09-1994", true}},
	{"11e1cd216801ea2657723abc86ecb01f",                  {"Doom (Registered 1.8)", "id Software", "20-01-1995", true}},
	{"1cd63c5ddff1bf8ce844237f580e9cf3",                  {"Doom (Registered 1.9)", "id Software", "01-02-1995", true}},
	{"c4fe9fd920207691a9f493668e0a2083",                      {"The Ultimate Doom", "id Software", "25-05-1995", true}},
	{"fb35c4a5a9fd49ec29ab6e900572c524",                     {"Doom (BFG Edition)", "id Software", "16-10-2012", true}},
	{"8517c4e8f0eef90b82852667d345eb86",    {"The Ultimate Doom (Unity/Bethesda)", "id Software", "09-01-2020	", true}},

	// DOOM II: HELL ON EARTH
	{"d9153ced9fd5b898b36cc5844e35b520",                 {"Doom II (1.666 German)", "id Software", "29-08-1994", true}},
	{"30e3c2d0350b67bfbf47271970b74b2f",                        {"Doom II (1.666)", "id Software", "29-08-1994", true}},
	{"ea74a47a791fdef2e9f2ea8b8a9da13b",                          {"Doom II (1.7)", "id Software", "21-09-1994", true}},
	{"d7a07e5d3f4625074312bc299d7ed33f",                         {"Doom II (1.7a)", "id Software", "18-10-1994", true}},
	{"3cb02349b3df649c86290907eed64e7b",                   {"Doom II (1.8 French)", "id Software", "01-12-1994", true}},
	{"c236745bb01d89bbb866c8fed81b6f8c",                          {"Doom II (1.8)", "id Software", "20-01-1995", true}},
	{"25e1459ca71d321525f84628f45ca8cd",                          {"Doom II (1.9)", "id Software", "01-02-1995", true}},
	{"b96683d113c4f4e9a916e1c7d1d71ffd",                        {"Doom II (PC-98)", "id Software", "01-02-1995", true}},
	{"c3bea40570c23e511a7ed3ebcd9865f7",
     {"Doom II (BFG Edition)", "id Software", "16-10-2012", true}                                                     }, //  appreantly its incorretly marked as PWAD
	{"8ab6d0527a29efdc1ef200e5687b5cae",               {"Doom II (Unity/Bethesda)", "id Software", "09-01-2020", true}},

	// TNT
	{"4e158d9953c79ccf97bd0663244cc6b6",            {"Final Doom: TNT Evilution (1.9)", "TeamTNT", "10-06-1996", true}},
	{"1d39e405bf6ee3df69a8d2646c8d5c49",      {"Final Doom: TNT Evilution (Anthology)", "TeamTNT", "14-11-1996", true}},
	{"f5528f6fd55cf9629141d79eda169630", {"Final Doom: TNT Evilution (Unity/Bethesda)", "TeamTNT", "03-09-2020", true}},

	// Plutonia
	{"75c8cf89566741fa9d22447604053bd7",    {"Final Doom: Plutonia Experiment", "Casali Brothers", "10-06-1996", true}},
	{"3493be7e1e2588bc9c8b31eab2587a04",
     {"Final Doom: Plutonia Experiment (Anthology)", "Casali Brothers", "21-11-1996", true}                           },
	{"ae76c20366ff685d3bb9fab11b148b84",
     {"Final Doom: Plutonia Experiment (Unity/Bethesda)", "Casali Brothers", "03-09-2020", true}                      },

	// Heretic
	{"ae779722390ec32fa37b0d361f7d82f8",             {"Heretic (Shareware 1.2)", "Raven Software", "28-06-1995", true}},
	{"023b52175d2f260c3bdc5528df5d0a8c",             {"Heretic (Shareware 1.0)", "Raven Software", "24-12-1994", true}},
	{"fc7eab659f6ee522bb57acc1a946912f",            {"Heretic (Shareware Beta)", "Raven Software", "23-12-1994", true}},

	{"66d686b1ed6d35ff103f15dbd30e0341",                       {"Heretic (1.3)", "Raven Software", "22-03-1996", true}},
	{"1e4cb4ef075ad344dd63971637307e04",                       {"Heretic (1.2)", "Raven Software", "28-06-1995", true}},
	{"3117e399cdb4298eaa3941625f4b2923",                       {"Heretic (1.0)", "Raven Software", "27-12-1994", true}},

	// Hexen
	{"abb033caf81e26f12a2103e1fa25453f",         {"Hexen: Beyond Heretic (1.1)", "Raven Software", "14-03-1996", true}},
	{"b2543a03521365261d0a0f74d5dd90f0",         {"Hexen: Beyond Heretic (1.0)", "Raven Software", "13-10-1995", true}},
	{"c88a2bb3d783e2ad7b599a8e301e099e",        {"Hexen: Beyond Heretic (Beta)", "Raven Software", "27-09-1995", true}},
	{"876a5a44c7b68f04b3bb9bc7a5bd69d6",        {"Hexen: Beyond Heretic (Demo)", "Raven Software", "18-10-1995", true}},
	{"9178a32a496ff5befebfe6c47dac106c",   {"Hexen: Beyond Heretic (Demo Beta)", "Raven Software", "02-10-1995", true}},

	{"78d5898e99e220e4de64edaa0e479593",
     {"Hexen: Deathkings of the Dark Citadel (1.1)", "Raven Software", "09-05-1996",
     false}																										   }, // Expansion , unrunnable without hexen
	{"1077432e2690d390c256ac908b5f4efa",
     {"Hexen: Deathkings of the Dark Citadel (1.0)", "Raven Software", "22-03-1996",
     false}																										   }, // Expansion , unrunnable without hexen

	// Strife
	{"bb545b9c4eca0ff92c14d466b3294023",            {"Strife (Teaser 1.1)", "Rogue Entertainment", "14-03-1996", true}},
	{"de2c8dcad7cca206292294bdab524292",            {"Strife (Teaser 1.0)", "Rogue Entertainment", "22-02-1996", true}},
	{"2fed2031a5b03892106e0f117f17901f",            {"Strife (1.2 - 1.31)", "Rogue Entertainment", "23-05-1996", true}},
	{"8f2d3a6a289f5d2f2f9c1eec02b47299",                   {"Strife (1.1)", "Rogue Entertainment", "18-04-1996", true}},
	{"082234d6a3f7086424856478b5aa9e95",
     {"Strife (Voices)", "Rogue Entertainment", "18-04-1996", false}                                                  }, //  just voices, needs game

	// Chex Quest 3
	{"bce163d06521f9d15f9686786e64df13",                  {"Chex Quest 3 (1.4)", "Charles Jacobi", "24-06-2009", true}},
	{"148367e53ff7f4f814e54b5ac9ff0ab3",                  {"Chex Quest 3 (1.3)", "Charles Jacobi", "12-06-2009", true}},
	{"26a8998ecdaa983f8e6c363b4b95bf55",                  {"Chex Quest 3 (1.2)", "Charles Jacobi", "02-05-2009", true}},
	{"f85944f55fff094f2ffbd3ecef3fa255",                  {"Chex Quest 3 (1.1)", "Charles Jacobi", "22-04-2009", true}},
	{"cb001c34e424687191f299cc1dff4d68",                  {"Chex Quest 3 (1.0)", "Charles Jacobi", "12-11-2008", true}},
	{"59c985995db55cd2623c1893550d82b3",   {"Chex Quest 3 (1.0 unoffical PWAD)", "Charles Jacobi", "24-06-2009", true}},

	// Action Doom 2: Urban Brawl
	{"1914b280b0a4b517214523bc2270e758",
     {"Action Doom 2: Urban Brawl (1.0)", "Stephen Browning et al.", "??-??-????", true}                              },
	{"c106a4e0a96f299954b073d5f97240be",
     {"Action Doom 2: Urban Brawl (1.1)", "Stephen Browning et al.", "26-12-2013", true}                              },

	// Hacx (Standalone 1.2)
	{"402ca45bb90520bfef0dec6baac5889e",                 {"Hacx (1.0 verified)", "Banjo Software", "08-10-1997", true}},
	{"1511a7032ebc834a3884cf390d7f186e",               {"Hacx (1.0 unverified)", "Banjo Software", "09-10-1997", true}},
	{"b7fd2f43f3382cf012dc6b097a3cb182",                          {"Hacx (1.1)", "Banjo Software", "16-09-1997", true}},
	{"65ed74d522bdf6649c2831b13b9e02b4",                          {"Hacx (1.2)", "Banjo Software", "09-10-2010", true}},
	{"793f07ebadb3d7353ee5b6b6429d9afa",
     {"Hacx (2.0)", "Banjo Software et al.", "09-10-2010", true}                                                      }, //  from build r61

	// Harmony
	{"48ebb49b52f6a3020d174dbcc1b9aeaf",                {"Harmony (1.1)", "Thomas van der Velden", "17-02-2012", true}},
	{"fe2cce6713ddcf6c6d6f0e8154b0cb38",                {"Harmony (1.0)", "Thomas van der Velden", "10-12-2009", true}},

	// The Adventures of Square (Ep 1 & 2)
	{"f4578097c658ad3c813cd5901ec125e2",       {"The Adventures of Square (2.1)", "BigBrik Games", "22-06-2019", true}},

	// Delaweare (this hash is taken from the wad of the standalone, as thats the only one i could find)
	{"a185498bdf721b4c01dc87fa81d1580b",                           {"Delaweare", "Space Is Green", "30-06-2014", true}},

	// Rise of the Wool Ball
	{"9176043468e10eaa471ae556e8e55745",           {"Rise of the Wool Ball (1.3)", "MSPaintR0cks", "21-07-2017", true}},
	{"fb0226e3fed7a3c1e7ea4cd4da905950",           {"Rise of the Wool Ball (1.2)", "MSPaintR0cks", "14-06-2017", true}},

	// Freedoom (MUST BE UPDATE EVERY RELASE)
	{"b93be13d05148dd01614bc205a03648e",          {"Freedoom: Phase 1 (0.13)", "Freedoom Project", "30-01-2024", true}},
	{"cd666466759b5e5f63af93c5f0ffd0a1",          {"Freedoom: Phase 2 (0.13)", "Freedoom Project", "30-01-2024", true}},
	{"908dfd77a14cc490c4cea94b62d13449",                     {"FreeDM (0.13)", "Freedoom Project", "30-01-2024", true}},
};

// Simple struct to hold all data for a single flag
struct FlagInfo
{
	std::string label;
	std::string tooltip;
	int         setIdx; // 0 = dmflags or compatflags, 1 = dmflags2, compatflags2 etc.
	int         bitVal; // The actual value 1 << [flag index starting from 0]
	bool        invert; // true = Unchecking the box adds the flag (because some are default checked)
};

inline std::vector<FlagInfo> getDMFlagsList()
{
	// dynamically apply translated flags here
	std::vector<FlagInfo> dmFlagsList = {

		// DMFLAGS
		{		   GStrings.GetString("DMFLAGS_ALLOWHEALTH"),
	     GStrings.GetString("DMFLAGS_ALLOWHEALTH_TOOLTIP"), 0,  1 << 0,true																									  },
		{		 GStrings.GetString("DMFLAGS_ALLOWPOWERUPS"),
	     GStrings.GetString("DMFLAGS_ALLOWPOWERUPS_TOOLTIP"), 0,  1 << 1,  true         },
		{		   GStrings.GetString("DMFLAGS_WEAPONSSTAY"),
	     GStrings.GetString("DMFLAGS_WEAPONSSTAY_TOOLTIP"), 0,  1 << 2, false           },
		{      GStrings.GetString("DMFLAGS_FALLINGDAMAGEOLD"),
	     GStrings.GetString("DMFLAGS_FALLINGDAMAGEOLD_TOOLTIP"), 0,  1 << 3, false      },
		{    GStrings.GetString("DMFLAGS_FALLINGDAMAGEHEXEN"),
	     GStrings.GetString("DMFLAGS_FALLINGDAMAGEHEXEN_TOOLTIP"), 0,  1 << 4, false    },
		{   GStrings.GetString("DMFLAGS_FALLINGDAMAGESTRIFE"),
	     GStrings.GetString("DMFLAGS_FALLINGDAMAGESTRIFE_TOOLTIP"), 0,      24,
	     false																							  }, // it really is 24 according to ingame
		{			   GStrings.GetString("DMFLAGS_SAMEMAP"),
	     GStrings.GetString("DMFLAGS_SAMEMAP_TOOLTIP"), 0,  1 << 6, false               },
		{		 GStrings.GetString("DMFLAGS_SPAWNFARTHEST"),
	     GStrings.GetString("DMFLAGS_SPAWNFARTHEST_TOOLTIP"), 0,  1 << 7, false         },
		{		  GStrings.GetString("DMFLAGS_FORCERESPAWN"),
	     GStrings.GetString("DMFLAGS_FORCERESPAWN_TOOLTIP"), 0,  1 << 8, false          },
		{			GStrings.GetString("DMFLAGS_ALLOWARMOR"),
	     GStrings.GetString("DMFLAGS_ALLOWARMOR_TOOLTIP"), 0,  1 << 9,  true            },
		{			 GStrings.GetString("DMFLAGS_ALLOWEXIT"),
	     GStrings.GetString("DMFLAGS_ALLOWEXIT_TOOLTIP"), 0, 1 << 10,  true             },
		{		  GStrings.GetString("DMFLAGS_INFINITEAMMO"),
	     GStrings.GetString("DMFLAGS_INFINITEAMMO_TOOLTIP"), 0, 1 << 11, false          },
		{			GStrings.GetString("DMFLAGS_NOMONSTERS"),
	     GStrings.GetString("DMFLAGS_NOMONSTERS_TOOLTIP"), 0, 1 << 12, false            },
		{       GStrings.GetString("DMFLAGS_MONSTERSRESPAWN"),
	     GStrings.GetString("DMFLAGS_MONSTERSRESPAWN_TOOLTIP"), 0, 1 << 13, false       },
		{		  GStrings.GetString("DMFLAGS_ITEMSRESPAWN"),
	     GStrings.GetString("DMFLAGS_ITEMSRESPAWN_TOOLTIP"), 0, 1 << 14, false          },
		{		  GStrings.GetString("DMFLAGS_FASTMONSTERS"),
	     GStrings.GetString("DMFLAGS_FASTMONSTERS_TOOLTIP"), 0, 1 << 15, false          },
		{			 GStrings.GetString("DMFLAGS_ALLOWJUMP"),
	     GStrings.GetString("DMFLAGS_ALLOWJUMP_TOOLTIP"), 0, 1 << 16,  true             },
		{		 GStrings.GetString("DMFLAGS_ALLOWFREELOOK"),
	     GStrings.GetString("DMFLAGS_ALLOWFREELOOK_TOOLTIP"), 0, 1 << 18,  true         },
		{			  GStrings.GetString("DMFLAGS_ALLOWFOV"),
	     GStrings.GetString("DMFLAGS_ALLOWFOV_TOOLTIP"), 0, 1 << 20,  true              },
		{     GStrings.GetString("DMFLAGS_SPAWNMULTIWEAPONS"),
	     GStrings.GetString("DMFLAGS_SPAWNMULTIWEAPONS_TOOLTIP"), 0, 1 << 21,  true     },
		{		   GStrings.GetString("DMFLAGS_ALLOWCROUCH"),
	     GStrings.GetString("DMFLAGS_ALLOWCROUCH_TOOLTIP"), 0, 1 << 22,  true           },
		{		 GStrings.GetString("DMFLAGS_LOSEINVENTORY"),
	     GStrings.GetString("DMFLAGS_LOSEINVENTORY_TOOLTIP"), 0, 1 << 24, false         },
		{			  GStrings.GetString("DMFLAGS_KEEPKEYS"),
	     GStrings.GetString("DMFLAGS_KEEPKEYS_TOOLTIP"), 0, 1 << 25,  true              },
		{		   GStrings.GetString("DMFLAGS_KEEPWEAPONS"),
	     GStrings.GetString("DMFLAGS_KEEPWEAPONS_TOOLTIP"), 0, 1 << 26,  true           },
		{			 GStrings.GetString("DMFLAGS_KEEPARMOR"),
	     GStrings.GetString("DMFLAGS_KEEPARMOR_TOOLTIP"), 0, 1 << 27,  true             },
		{		  GStrings.GetString("DMFLAGS_KEEPPOWERUPS"),
	     GStrings.GetString("DMFLAGS_KEEPPOWERUPS_TOOLTIP"), 0, 1 << 28,  true          },
		{			  GStrings.GetString("DMFLAGS_KEEPAMMO"),
	     GStrings.GetString("DMFLAGS_KEEPAMMO_TOOLTIP"), 0, 1 << 29,  true              },
		{		  GStrings.GetString("DMFLAGS_LOSEHALFAMMO"),
	     GStrings.GetString("DMFLAGS_LOSEHALFAMMO_TOOLTIP"), 0, 1 << 30, false          },

		// DMFLAGS2
		{		   GStrings.GetString("DMFLAGS2_DROPWEAPON"),
	     GStrings.GetString("DMFLAGS2_DROPWEAPON_TOOLTIP"), 1,  1 << 1, false           },
		{       GStrings.GetString("DMFLAGS2_NOTEAMCHANGING"),
	     GStrings.GetString("DMFLAGS2_NOTEAMCHANGING_TOOLTIP"), 1,  1 << 4, false       },
		{		   GStrings.GetString("DMFLAGS2_DOUBLEAMMO"),
	     GStrings.GetString("DMFLAGS2_DOUBLEAMMO_TOOLTIP"), 1,  1 << 6, false           },
		{		 GStrings.GetString("DMFLAGS2_DEGENERATION"),
	     GStrings.GetString("DMFLAGS2_DEGENERATION_TOOLTIP"), 1,  1 << 7, false         },
		{       GStrings.GetString("DMFLAGS2_ALLOWBFGAIMING"),
	     GStrings.GetString("DMFLAGS2_ALLOWBFGAIMING_TOOLTIP"), 1,  1 << 8,  true       },
		{       GStrings.GetString("DMFLAGS2_BARRELSRESPAWN"),
	     GStrings.GetString("DMFLAGS2_BARRELSRESPAWN_TOOLTIP"), 1,  1 << 9, false       },
		{    GStrings.GetString("DMFLAGS2_RESPAWNPROTECTION"),
	     GStrings.GetString("DMFLAGS2_RESPAWNPROTECTION_TOOLTIP"), 1, 1 << 10, false    },
		{       GStrings.GetString("DMFLAGS2_SPAWNWHEREDIED"),
	     GStrings.GetString("DMFLAGS2_SPAWNWHEREDIED_TOOLTIP"), 1, 1 << 12, false       },
		{      GStrings.GetString("DMFLAGS2_KEEPFRAGSGAINED"),
	     GStrings.GetString("DMFLAGS2_KEEPFRAGSGAINED_TOOLTIP"), 1, 1 << 13, false      },
		{			GStrings.GetString("DMFLAGS2_NORESPAWN"),
	     GStrings.GetString("DMFLAGS2_NORESPAWN_TOOLTIP"), 1, 1 << 14, false            },
		{      GStrings.GetString("DMFLAGS2_LOSEFRAGONDEATH"),
	     GStrings.GetString("DMFLAGS2_LOSEFRAGONDEATH_TOOLTIP"), 1, 1 << 15, false      },
		{    GStrings.GetString("DMFLAGS2_INFINITEINVENTORY"),
	     GStrings.GetString("DMFLAGS2_INFINITEINVENTORY_TOOLTIP"), 1, 1 << 16, false    },
		{     GStrings.GetString("DMFLAGS2_NOMONSTERSTOEXIT"),
	     GStrings.GetString("DMFLAGS2_NOMONSTERSTOEXIT_TOOLTIP"), 1, 1 << 17, false     },
		{		 GStrings.GetString("DMFLAGS2_ALLOWAUTOMAP"),
	     GStrings.GetString("DMFLAGS2_ALLOWAUTOMAP_TOOLTIP"), 1, 1 << 18,  true         },
		{        GStrings.GetString("DMFLAGS2_AUTOMAPALLIES"),
	     GStrings.GetString("DMFLAGS2_AUTOMAPALLIES_TOOLTIP"), 1, 1 << 19,  true        },
		{		  GStrings.GetString("DMFLAGS2_ALLOWSPYING"),
	     GStrings.GetString("DMFLAGS2_ALLOWSPYING_TOOLTIP"), 1, 1 << 20,  true          },
		{        GStrings.GetString("DMFLAGS2_CHASECAMCHEAT"),
	     GStrings.GetString("DMFLAGS2_CHASECAMCHEAT_TOOLTIP"), 1, 1 << 21, false        },
		{      GStrings.GetString("DMFLAGS2_DISALLOWSUICIDE"),
	     GStrings.GetString("DMFLAGS2_DISALLOWSUICIDE_TOOLTIP"), 1, 1 << 22, false      },
		{		 GStrings.GetString("DMFLAGS2_ALLOWAUTOAIM"),
	     GStrings.GetString("DMFLAGS2_ALLOWAUTOAIM_TOOLTIP"), 1, 1 << 23,  true         },
		{      GStrings.GetString("DMFLAGS2_CHECKAMMOSWITCH"),
	     GStrings.GetString("DMFLAGS2_CHECKAMMOSWITCH_TOOLTIP"), 1, 1 << 24,  true      },
		{  GStrings.GetString("DMFLAGS2_IOSDEATHKILLSSPAWNS"),
	     GStrings.GetString("DMFLAGS2_IOSDEATHKILLSSPAWNS_TOOLTIP"), 1, 1 << 25,  true  },
		{        GStrings.GetString("DMFLAGS2_ENDSECTORKILL"),
	     GStrings.GetString("DMFLAGS2_ENDSECTORKILL_TOOLTIP"), 1, 1 << 26,  true        },
		{   GStrings.GetString("DMFLAGS2_BIGPOWERUPSRESPAWN"),
	     GStrings.GetString("DMFLAGS2_BIGPOWERUPSRESPAWN_TOOLTIP"), 1, 1 << 27, false   },
		{  GStrings.GetString("DMFLAGS2_ALLOWVERTICALSPREAD"),
	     GStrings.GetString("DMFLAGS2_ALLOWVERTICALSPREAD_TOOLTIP"), 1, 1 << 30, false  },

		// DMFLAGS3
		{     GStrings.GetString("DMFLAGS3_NOPLAYERCLIPPING"),
	     GStrings.GetString("DMFLAGS3_NOPLAYERCLIPPING_TOOLTIP"), 2,  1 << 0, false     },
		{			GStrings.GetString("DMFLAGS3_SHAREKEYS"),
	     GStrings.GetString("DMFLAGS3_SHAREKEYS_TOOLTIP"), 2,  1 << 1, false            },
		{		 GStrings.GetString("DMFLAGS3_LOCALPICKUPS"),
	     GStrings.GetString("DMFLAGS3_LOCALPICKUPS_TOOLTIP"), 2,  1 << 2, false         },
		{GStrings.GetString("DMFLAGS3_NOLOCALPICKUPSDROPPED"),
	     GStrings.GetString("DMFLAGS3_NOLOCALPICKUPSDROPPED_TOOLTIP"), 2,  1 << 3, false},
		{      GStrings.GetString("DMFLAGS3_NOCOOPONLYITEMS"),
	     GStrings.GetString("DMFLAGS3_NOCOOPONLYITEMS_TOOLTIP"), 2,  1 << 4, false      },
		{     GStrings.GetString("DMFLAGS3_NOCOOPONLYTHINGS"),
	     GStrings.GetString("DMFLAGS3_NOCOOPONLYTHINGS_TOOLTIP"), 2,  1 << 5, false     },
		{   GStrings.GetString("DMFLAGS3_REMEMBERLASTWEAPON"),
	     GStrings.GetString("DMFLAGS3_REMEMBERLASTWEAPON_TOOLTIP"), 2,  1 << 6, false   },
		{		  GStrings.GetString("DMFLAGS3_PISTOLSTART"),
	     GStrings.GetString("DMFLAGS3_PISTOLSTART_TOOLTIP"), 2,  1 << 7, false          }
    };

	return dmFlagsList;
}

inline std::vector<FlagInfo> getCompatFlagsList()
{
	// dynamically apply translated flags here

	std::vector<FlagInfo> compatFlagsList = {

		// compatflags
		{		   GStrings.GetString("COMPAT_SHORTTEX"),
	     GStrings.GetString("COMPAT_SHORTTEX_TOOLTIP"), 0,          1 << 0, false           },
		{			 GStrings.GetString("COMPAT_STAIRS"),
	     GStrings.GetString("COMPAT_STAIRS_TOOLTIP"), 0,          1 << 1, false             },
		{		  GStrings.GetString("COMPAT_LIMITPAIN"),
	     GStrings.GetString("COMPAT_LIMITPAIN_TOOLTIP"), 0,          1 << 2, false          },
		{       GStrings.GetString("COMPAT_SILENTPICKUP"),
	     GStrings.GetString("COMPAT_SILENTPICKUP_TOOLTIP"), 0,          1 << 3, false       },
		{GStrings.GetString("COMPAT_NOVERTICALCOLLISION"),
	     GStrings.GetString("COMPAT_NOVERTICALCOLLISION_TOOLTIP"), 0,          1 << 4, false},
		{		 GStrings.GetString("COMPAT_SILENT_BFG"),
	     GStrings.GetString("COMPAT_SILENT_BFG_TOOLTIP"), 0,          1 << 5, false         },
		{			GStrings.GetString("COMPAT_WALLRUN"),
	     GStrings.GetString("COMPAT_WALLRUN_TOOLTIP"), 0,          1 << 6, false            },
		{        GStrings.GetString("COMPAT_NOTOSSDROPS"),
	     GStrings.GetString("COMPAT_NOTOSSDROPS_TOOLTIP"), 0,          1 << 7, false        },
		{        GStrings.GetString("COMPAT_USEBLOCKING"),
	     GStrings.GetString("COMPAT_USEBLOCKING_TOOLTIP"), 0,          1 << 8, false        },
		{        GStrings.GetString("COMPAT_NODOORLIGHT"),
	     GStrings.GetString("COMPAT_NODOORLIGHT_TOOLTIP"), 0,          1 << 9, false        },
		{        GStrings.GetString("COMPAT_RAVENSCROLL"),
	     GStrings.GetString("COMPAT_RAVENSCROLL_TOOLTIP"), 0,         1 << 10, false        },
		{        GStrings.GetString("COMPAT_SOUNDTARGET"),
	     GStrings.GetString("COMPAT_SOUNDTARGET_TOOLTIP"), 0,         1 << 11, false        },
		{		  GStrings.GetString("COMPAT_DEHHEALTH"),
	     GStrings.GetString("COMPAT_DEHHEALTH_TOOLTIP"), 0,         1 << 12, false          },
		{			  GStrings.GetString("COMPAT_TRACE"),
	     GStrings.GetString("COMPAT_TRACE_TOOLTIP"), 0,         1 << 13, false              },
		{			GStrings.GetString("COMPAT_DROPOFF"),
	     GStrings.GetString("COMPAT_DROPOFF_TOOLTIP"), 0,         1 << 14, false            },
		{		 GStrings.GetString("COMPAT_BOOMSCROLL"),
	     GStrings.GetString("COMPAT_BOOMSCROLL_TOOLTIP"), 0,         1 << 15, false         },
		{       GStrings.GetString("COMPAT_INVISIBILITY"),
	     GStrings.GetString("COMPAT_INVISIBILITY_TOOLTIP"), 0,         1 << 16, false       },
		{GStrings.GetString("COMPAT_SILENTINSTANTFLOORS"),
	     GStrings.GetString("COMPAT_SILENTINSTANTFLOORS_TOOLTIP"), 0,         1 << 17, false},
		{       GStrings.GetString("COMPAT_SECTORSOUNDS"),
	     GStrings.GetString("COMPAT_SECTORSOUNDS_TOOLTIP"), 0,         1 << 18, false       },
		{        GStrings.GetString("COMPAT_MISSILECLIP"),
	     GStrings.GetString("COMPAT_MISSILECLIP_TOOLTIP"), 0,         1 << 19, false        },
		{       GStrings.GetString("COMPAT_CROSSDROPOFF"),
	     GStrings.GetString("COMPAT_CROSSDROPOFF_TOOLTIP"), 0,         1 << 20, false       },
		{       GStrings.GetString("COMPAT_ANYBOSSDEATH"),
	     GStrings.GetString("COMPAT_ANYBOSSDEATH_TOOLTIP"), 0,         1 << 21, false       },
		{		   GStrings.GetString("COMPAT_MINOTAUR"),
	     GStrings.GetString("COMPAT_MINOTAUR_TOOLTIP"), 0,         1 << 22, false           },
		{		   GStrings.GetString("COMPAT_MUSHROOM"),
	     GStrings.GetString("COMPAT_MUSHROOM_TOOLTIP"), 0,         1 << 23, false           },
		{     GStrings.GetString("COMPAT_MBFMONSTERMOVE"),
	     GStrings.GetString("COMPAT_MBFMONSTERMOVE_TOOLTIP"), 0,         1 << 24, false     },
		{		 GStrings.GetString("COMPAT_CORPSEGIBS"),
	     GStrings.GetString("COMPAT_CORPSEGIBS_TOOLTIP"), 0,         1 << 25, false         },
		{	 GStrings.GetString("COMPAT_NOBLOCKFRIENDS"),
	     GStrings.GetString("COMPAT_NOBLOCKFRIENDS_TOOLTIP"), 0,         1 << 26, false     },
		{		 GStrings.GetString("COMPAT_SPRITESORT"),
	     GStrings.GetString("COMPAT_SPRITESORT_TOOLTIP"), 0,         1 << 27, false         },
		{			GStrings.GetString("COMPAT_HITSCAN"),
	     GStrings.GetString("COMPAT_HITSCAN_TOOLTIP"), 0,         1 << 28, false            },
		{			  GStrings.GetString("COMPAT_LIGHT"),
	     GStrings.GetString("COMPAT_LIGHT_TOOLTIP"), 0,         1 << 29, false              },
		{			GStrings.GetString("COMPAT_POLYOBJ"),
	     GStrings.GetString("COMPAT_POLYOBJ_TOOLTIP"), 0,         1 << 30, false            },
		{       GStrings.GetString("COMPAT_MASKEDMIDTEX"),
	     GStrings.GetString("COMPAT_MASKEDMIDTEX_TOOLTIP"), 0, (int)2147483648, false       },

		// compatflags2
		{		 GStrings.GetString("COMPAT2_BADANGLES"),
	     GStrings.GetString("COMPAT2_BADANGLES_TOOLTIP"), 1,          1 << 0, false         },
		{		 GStrings.GetString("COMPAT2_FLOORMOVE"),
	     GStrings.GetString("COMPAT2_FLOORMOVE_TOOLTIP"), 1,          1 << 1, false         },
		{       GStrings.GetString("COMPAT2_SOUNDCUTOFF"),
	     GStrings.GetString("COMPAT2_SOUNDCUTOFF_TOOLTIP"), 1,          1 << 2, false       },
		{       GStrings.GetString("COMPAT2_POINTONLINE"),
	     GStrings.GetString("COMPAT2_POINTONLINE_TOOLTIP"), 1,          1 << 3, false       },
		{		 GStrings.GetString("COMPAT2_MULTIEXIT"),
	     GStrings.GetString("COMPAT2_MULTIEXIT_TOOLTIP"), 1,          1 << 4, false         }
    };

	return compatFlagsList;
}
