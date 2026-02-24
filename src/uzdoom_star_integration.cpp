/**
 * ODOOM - OASIS STAR API Integration Implementation
 *
 * Build this file as part of ODOOM (UZDoom) with STAR API from STARAPIClient (not NativeWrapper).
 * Keycard pickups are reported to STAR; door/lock checks can use cross-game inventory.
 * In-game console: "star" command for testing (star version, star inventory, star add, etc.).
 *
 * Threading: star_sync uses background threads for auth and for inventory (like OQuake).
 * Pickups and inventory fetch run on a background thread via star_sync_inventory_start;
 * the main thread only polls for completion. This keeps the game responsive.
 */

#include "uzdoom_star_integration.h"
#include "star_api.h"
#ifndef STAR_API_HAS_SEND_ITEM
/* Forward declare send-item API when using an older star_api.h (e.g. in UZDoom tree). Link with updated star_api.lib. */
extern "C" {
star_api_result_t star_api_send_item_to_avatar(const char* target_username_or_avatar_id, const char* item_name, int quantity, const char* item_id);
star_api_result_t star_api_send_item_to_clan(const char* clan_name_or_target, const char* item_name, int quantity, const char* item_id);
}
#endif
#include "star_sync.h"
#include "odoom_branding.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <string>
#include <algorithm>
#include <cctype>
#include <thread>
#include <mutex>
#include <atomic>

/* ODOOM (UZDoom) headers for key detection */
#include "gamedata/a_keys.h"
#include "playsim/actor.h"
#include "gamedata/info.h"
#include "vm.h"
#include "c_dispatch.h"
#include "c_console.h"
#include "c_cvars.h"
#include "m_argv.h"
#include "printf.h"

#ifdef _WIN32
#include <windows.h>
#include <wincred.h>
#pragma comment(lib, "Credui.lib")
#endif

static star_api_config_t g_star_config;
static bool g_star_initialized = false;
static bool g_star_client_ready = false;
static bool g_star_debug_logging = true;
static bool g_star_logged_runtime_auth_failure = false;
static bool g_star_logged_missing_auth_config = false;
static bool g_star_cli_loaded = false;
static std::string g_star_override_username;
static std::string g_star_override_password;
static std::string g_star_override_jwt;
static std::string g_star_override_api_key;
static std::string g_star_override_avatar_id;
static std::string g_star_effective_api_key;
static std::string g_star_effective_avatar_id;
static std::string g_star_effective_username;
static std::string g_star_effective_password;
static const int STAR_PICKUP_OQUAKE_GOLD_KEY = 5005;
static const int STAR_PICKUP_OQUAKE_SILVER_KEY = 5013;
static const int STAR_PICKUP_GENERIC_ITEM = 9001;
static std::string g_star_pending_item_name;
static std::string g_star_pending_item_desc;
static std::string g_star_pending_item_type;
static bool g_star_has_pending_item = false;
static std::string g_star_last_pickup_name;
static std::string g_star_last_pickup_type;
static std::string g_star_last_pickup_desc;
static bool g_star_has_last_pickup = false;
static bool g_star_face_suppressed_for_session = false;
/** Single source of truth for status bar face; only set by star face on/off and beam-in/out. */
static bool g_star_show_anorak_face = false;

/** True when we started async SSO auth so beamin command can show "Authenticating..." instead of "Beam-in failed". */
static bool g_star_async_auth_pending = false;
static bool StarInitialized(void);
CVAR(Bool, oasis_star_anorak_face, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, oasis_star_beam_face, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, odoom_star_api_url, "https://star-api.oasisplatform.world/api", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, odoom_oq_monster_yoffset, -50, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_global, 1.0f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_dog, 0.50f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_zombie, 0.33f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_demon, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_shambler, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_grunt, 0.40f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_fish, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_ogre, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_enforcer, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_spawn, 1.00f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Float, odoom_oq_monster_scale_knight, 0.60f, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, odoom_star_username, "", 0)
CVAR(String, odoom_oasis_api_url, "https://api.oasisplatform.world", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
/* Stack (1) = each pickup adds quantity; Unlock (0) = one per type. Ammo always stacks. Shared with OQuake; sigils are OQuake-only. */
CVAR(Int, odoom_star_stack_armor, 1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, odoom_star_stack_weapons, 1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, odoom_star_stack_powerups, 1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, odoom_star_stack_keys, 1, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

/* Config: ODOOM stores STAR options in the engine config. Typical path: Documents\\My Games\\UZDoom
 * (or OneDrive\\Documents\\My Games\\UZDoom) - ini file there is written on exit. STAR cvars use
 * CVAR_ARCHIVE so they should appear in that ini; if not visible, use oasisstar.json (loaded/saved
 * when found) for parity with OQuake and cross-game config sharing. */
static std::string g_odoom_json_config_path;

/** When init (e.g. star_api_init) has failed, we skip retrying until user runs beamin again to avoid spamming "couldn't find the host". */
static bool g_star_init_failed_this_session = false;

/* Async inventory (background thread, like OQuake). Pending pickups are synced then inventory is fetched. */
#define ODOOM_SYNC_PENDING_MAX 64
static star_sync_local_item_t g_odoom_pending_sync[ODOOM_SYNC_PENDING_MAX];
static int g_odoom_pending_sync_count = 0;
static star_sync_local_item_t g_odoom_in_flight_sync[ODOOM_SYNC_PENDING_MAX];
static int g_odoom_in_flight_count = 0;
static star_item_list_t* g_odoom_cached_inventory = nullptr;

/* Inventory overlay: when open, temporarily clear key bindings (OQuake-style) so arrows/keys only drive the popup.
 * We read raw key state here and set odoom_key_* CVars so ZScript can drive selection/use/send/tabs. */
static bool g_odoom_inventory_bindings_captured = false;

/* Send popup (OQuake-style): text input buffer for username/clan name */
static const int ODOOM_SEND_INPUT_MAX = 64;
static std::string g_odoom_send_input_buffer;
static bool g_odoom_send_key_was_down[256];
static bool g_odoom_send_popup_was_open = false;
/* Last send item name/qty so we can update local cache on success without hitting the API */
static std::string g_odoom_last_sent_item_name;
static int g_odoom_last_sent_qty = 0;

static void StarApplyBeamFacePreference(void);

/*-----------------------------------------------------------------------------
 * OASIS STAR Config - oasisstar.json (parity with OQuake)
 *-----------------------------------------------------------------------------*/
static bool ODOOM_FindConfigFile(const char* filename, std::string& out_path) {
	FILE* test = fopen(filename, "r");
	if (test) { fclose(test); out_path = filename; return true; }
	const char* locations[] = { "build/", "../build/", "OASIS Omniverse/ODOOM/build/", nullptr };
	for (int i = 0; locations[i]; i++) {
		char buf[512];
		snprintf(buf, sizeof(buf), "%s%s", locations[i], filename);
		test = fopen(buf, "r");
		if (test) { fclose(test); out_path = buf; return true; }
	}
#ifdef _WIN32
	char exe_path[MAX_PATH] = {0};
	if (GetModuleFileNameA(nullptr, exe_path, sizeof(exe_path))) {
		std::string exe_dir(exe_path);
		size_t last = exe_dir.find_last_of("\\/");
		if (last != std::string::npos) {
			exe_dir.resize(last);
			out_path = exe_dir + "\\"; out_path += filename;
			test = fopen(out_path.c_str(), "r");
			if (test) { fclose(test); return true; }
			out_path = exe_dir + "\\build\\"; out_path += filename;
			test = fopen(out_path.c_str(), "r");
			if (test) { fclose(test); return true; }
		}
	}
#endif
	return false;
}

static bool ODOOM_ExtractJsonValue(const char* json, const char* key, char* value, int maxlen) {
	char search[128];
	snprintf(search, sizeof(search), "\"%s\"", key);
	const char* pos = strstr(json, search);
	if (!pos) return false;
	pos += strlen(search);
	while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
	if (*pos == '"') {
		pos++;
		int n = 0;
		while (*pos && *pos != '"' && *pos != '\n' && *pos != '\r' && n < maxlen - 1) {
			if (*pos == '\\' && pos[1]) { pos++; if (*pos == 'n') value[n++] = '\n'; else if (*pos == 't') value[n++] = '\t'; else if (*pos == '\\') value[n++] = '\\'; else if (*pos == '"') value[n++] = '"'; else value[n++] = *pos; }
			else value[n++] = *pos;
			pos++;
		}
		value[n] = '\0';
		return n > 0;
	}
	int n = 0;
	while (*pos && *pos != ',' && *pos != '}' && *pos != '\n' && *pos != '\r' && *pos != ' ' && n < maxlen - 1)
		value[n++] = *pos++;
	value[n] = '\0';
	return n > 0;
}

static bool ODOOM_LoadJsonConfig(const char* json_path) {
	FILE* f = fopen(json_path, "r");
	if (!f) return false;
	char json[4096] = {0};
	size_t len = fread(json, 1, sizeof(json) - 1, f);
	fclose(f);
	if (len == 0) return false;
	json[len] = '\0';
	bool loaded = false;
	char value[256];
	if (ODOOM_ExtractJsonValue(json, "star_api_url", value, (int)sizeof(value))) {
		odoom_star_api_url = value;
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "oasis_api_url", value, (int)sizeof(value))) {
		odoom_oasis_api_url = value;
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "beam_face", value, (int)sizeof(value))) {
		oasis_star_beam_face = (atoi(value) != 0);
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "stack_armor", value, (int)sizeof(value))) {
		odoom_star_stack_armor = (atoi(value) != 0) ? 1 : 0;
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "stack_weapons", value, (int)sizeof(value))) {
		odoom_star_stack_weapons = (atoi(value) != 0) ? 1 : 0;
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "stack_powerups", value, (int)sizeof(value))) {
		odoom_star_stack_powerups = (atoi(value) != 0) ? 1 : 0;
		loaded = true;
	}
	if (ODOOM_ExtractJsonValue(json, "stack_keys", value, (int)sizeof(value))) {
		odoom_star_stack_keys = (atoi(value) != 0) ? 1 : 0;
		loaded = true;
	}
	return loaded;
}

static bool ODOOM_SaveJsonConfig(const char* json_path) {
	FILE* f = fopen(json_path, "w");
	if (!f) return false;
	const char* star_url = (const char*)odoom_star_api_url;
	const char* oasis_url = (const char*)odoom_oasis_api_url;
	fprintf(f, "{\n");
	fprintf(f, "  \"star_api_url\": \"%s\",\n", star_url ? star_url : "");
	fprintf(f, "  \"oasis_api_url\": \"%s\",\n", oasis_url ? oasis_url : "");
	fprintf(f, "  \"beam_face\": %d,\n", oasis_star_beam_face ? 1 : 0);
	fprintf(f, "  \"stack_armor\": %d,\n", odoom_star_stack_armor ? 1 : 0);
	fprintf(f, "  \"stack_weapons\": %d,\n", odoom_star_stack_weapons ? 1 : 0);
	fprintf(f, "  \"stack_powerups\": %d,\n", odoom_star_stack_powerups ? 1 : 0);
	fprintf(f, "  \"stack_keys\": %d\n", odoom_star_stack_keys ? 1 : 0);
	fprintf(f, "}\n");
	fclose(f);
	return true;
}

/** Write current STAR cvars to oasisstar.json. Used on exit, star config save, star seturl/setoasisurl, star face. */
static void ODOOM_SaveStarConfigToFiles(void) {
	std::string path;
	if (!g_odoom_json_config_path.empty())
		path = g_odoom_json_config_path;
	else if (ODOOM_FindConfigFile("oasisstar.json", path))
		g_odoom_json_config_path = path;
	else {
		/* Prefer exe dir for new file */
#ifdef _WIN32
		char exe_path[MAX_PATH] = {0};
		if (GetModuleFileNameA(nullptr, exe_path, sizeof(exe_path))) {
			std::string exe_dir(exe_path);
			size_t last = exe_dir.find_last_of("\\/");
			if (last != std::string::npos) {
				exe_dir.resize(last);
				path = exe_dir + "\\oasisstar.json";
			}
		}
#endif
		if (path.empty()) path = "oasisstar.json";
	}
	if (!path.empty() && ODOOM_SaveJsonConfig(path.c_str()))
		g_odoom_json_config_path = path;
}

/** Return 1 if key is currently down, 0 otherwise. Uses platform API when available. */
static int ODOOM_GetRawKeyDown(int vk_or_ascii)
{
#ifdef _WIN32
	SHORT s = GetAsyncKeyState(vk_or_ascii);
	return (s & 0x8000) ? 1 : 0;
#else
	(void)vk_or_ascii;
	return 0;
#endif
}

/** Push g_odoom_cached_inventory to CVars so ZScript overlay can display it (same data as "star inventory" command). */
static void ODOOM_UpdateStarInventoryCVars(void) {
	static char listBuf[24576];  /* one line per item: name\tdesc\ttype\tgame\n, max 64 items */
	FBaseCVar* countVar = FindCVar("odoom_star_inventory_count", nullptr);
	FBaseCVar* listVar = FindCVar("odoom_star_inventory_list", nullptr);
	if (!countVar || !listVar) return;

	if (!g_odoom_cached_inventory || g_odoom_cached_inventory->count == 0) {
		UCVarValue u;
		u.Int = 0;
		countVar->SetGenericRep(u, CVAR_Int);
		UCVarValue v;
		v.String = (char*)("");
		listVar->SetGenericRep(v, CVAR_String);
		return;
	}

	size_t n = g_odoom_cached_inventory->count;
	if (n > 64) n = 64;
	size_t off = 0;
	for (size_t i = 0; i < n && off < sizeof(listBuf) - 320; i++) {
		const star_item_t* it = &g_odoom_cached_inventory->items[i];
		char name[256], desc[256], type[64], game[64];
		auto copySafe = [](char* dst, const char* src, int maxLen) {
			int j = 0;
			while (src[j] && j < maxLen - 1) {
				char c = src[j];
				if (c == '\t' || c == '\n' || c == '\r') c = ' ';
				dst[j++] = c;
			}
			dst[j] = '\0';
		};
		copySafe(name, it->name, 256);
		copySafe(desc, it->description, 256);
		copySafe(type, it->item_type, 64);
		copySafe(game, it->game_source, 64);
		int wr = snprintf(listBuf + off, (size_t)(sizeof(listBuf) - off), "%s\t%s\t%s\t%s\n", name, desc, type, game);
		if (wr > 0 && (size_t)wr < sizeof(listBuf) - off) off += (size_t)wr;
		else break;
	}
	listBuf[off] = '\0';

	UCVarValue u;
	u.Int = (int)n;
	countVar->SetGenericRep(u, CVAR_Int);
	UCVarValue v;
	v.String = listBuf;
	listVar->SetGenericRep(v, CVAR_String);
}

static void ODOOM_OnAuthDone(void* user_data);
static void ODOOM_OnInventoryDone(void* user_data);
static void ODOOM_OnSendItemDone(void* user_data);

/** Start background inventory sync if we have pending items and no sync in progress. */
static void ODOOM_StartInventorySyncIfNeeded(void) {
	if (!g_star_initialized || star_sync_inventory_in_progress())
		return;
	if (g_odoom_pending_sync_count <= 0) return;
	int n = g_odoom_pending_sync_count;
	if (n > ODOOM_SYNC_PENDING_MAX) n = ODOOM_SYNC_PENDING_MAX;
	for (int i = 0; i < n; i++) {
		g_odoom_in_flight_sync[i] = g_odoom_pending_sync[i];
		g_odoom_in_flight_sync[i].synced = 0;
	}
	g_odoom_in_flight_count = n;
	g_odoom_pending_sync_count = 0;
	star_sync_inventory_start(g_odoom_in_flight_sync, n, "ODOOM", ODOOM_OnInventoryDone, nullptr);
}

/** Called from main thread by star_sync_pump() when auth completes. */
static void ODOOM_OnAuthDone(void* user_data) {
	(void)user_data;
	int success = 0;
	char username_buf[64] = {};
	char avatar_id_buf[64] = {};
	char error_buf[256] = {};
	if (!star_sync_auth_get_result(&success, username_buf, sizeof(username_buf), avatar_id_buf, sizeof(avatar_id_buf), error_buf, sizeof(error_buf)))
		return;
	g_star_async_auth_pending = false;
	if (success) {
		g_star_initialized = true;
		g_star_logged_runtime_auth_failure = false;
		g_star_logged_missing_auth_config = false;
		g_star_effective_username = username_buf;
		g_star_effective_avatar_id = avatar_id_buf;
		g_star_config.avatar_id = g_star_effective_avatar_id.empty() ? nullptr : g_star_effective_avatar_id.c_str();
		odoom_star_username = g_star_effective_username.c_str();
		StarApplyBeamFacePreference();
		/* Start inventory fetch in background so popup has data when opened (same as OQuake). */
		if (!star_sync_inventory_in_progress())
			star_sync_inventory_start(nullptr, 0, "ODOOM", ODOOM_OnInventoryDone, nullptr);
		Printf(PRINT_NONOTIFY, "Beam-in successful. Cross-game features enabled.\n");
	} else {
		Printf(PRINT_NONOTIFY, "Beam-in failed: %s\n", error_buf[0] ? error_buf : star_api_get_last_error());
	}
}

/** Called from main thread by star_sync_pump() when inventory refresh completes. */
static void ODOOM_OnInventoryDone(void* user_data) {
	(void)user_data;
	star_item_list_t* list = nullptr;
	star_api_result_t res = STAR_API_ERROR_API_ERROR;
	char err_buf[256] = {};
	if (!star_sync_inventory_get_result(&list, &res, err_buf, sizeof(err_buf)))
		return;
	if (g_odoom_cached_inventory)
		star_api_free_item_list(g_odoom_cached_inventory);
	g_odoom_cached_inventory = list;
	ODOOM_UpdateStarInventoryCVars();
	star_sync_inventory_clear_result();
	g_odoom_in_flight_count = 0;
	/* Do NOT chain another inventory sync here; load once after beam-in only. */
}

/** Return true if the item exists in the local cached inventory (no API call). Use for door/lock checks to avoid redundant API hits. */
static bool ODOOM_HasItemInCache(const char* item_name) {
	if (!item_name || !g_odoom_cached_inventory || !g_odoom_cached_inventory->items) return false;
	for (size_t i = 0; i < g_odoom_cached_inventory->count; i++) {
#ifdef _WIN32
		if (_stricmp(g_odoom_cached_inventory->items[i].name, item_name) == 0) return true;
#else
		if (strcasecmp(g_odoom_cached_inventory->items[i].name, item_name) == 0) return true;
#endif
	}
	return false;
}

/** Remove up to 'remove_qty' items matching 'item_name' from the cached list (in-place). Case-insensitive name match. */
static void ODOOM_RemoveFromCachedInventory(const char* item_name, int remove_qty) {
	if (!g_odoom_cached_inventory || !item_name || remove_qty <= 0) return;
	size_t count = g_odoom_cached_inventory->count;
	star_item_t* items = g_odoom_cached_inventory->items;
	if (!items) return;
	size_t write = 0;
	int removed = 0;
	for (size_t i = 0; i < count; i++) {
		if (removed < remove_qty &&
#ifdef _WIN32
		    _stricmp(items[i].name, item_name) == 0
#else
		    strcasecmp(items[i].name, item_name) == 0
#endif
		) {
			removed++;
			continue;
		}
		if (write != i)
			items[write] = items[i];
		write++;
	}
	g_odoom_cached_inventory->count = write;
}

/** Called from main thread by star_sync_pump() when send-item completes (same pattern as Quake). */
static void ODOOM_OnSendItemDone(void* user_data) {
	(void)user_data;
	int success = 0;
	char err_buf[384] = {};
	if (!star_sync_send_item_get_result(&success, err_buf, sizeof(err_buf)))
		return;
	static char s_send_status_buf[384];
	if (success) {
		std::strncpy(s_send_status_buf, "Item sent.", sizeof(s_send_status_buf) - 1);
		s_send_status_buf[sizeof(s_send_status_buf) - 1] = '\0';
		Printf(PRINT_NONOTIFY, "Item sent.\n");
		/* Update local cache so UI reflects sent items without hitting the API */
		if (!g_odoom_last_sent_item_name.empty() && g_odoom_last_sent_qty > 0) {
			ODOOM_RemoveFromCachedInventory(g_odoom_last_sent_item_name.c_str(), g_odoom_last_sent_qty);
			ODOOM_UpdateStarInventoryCVars();
			g_odoom_last_sent_item_name.clear();
			g_odoom_last_sent_qty = 0;
		}
	} else {
		std::snprintf(s_send_status_buf, sizeof(s_send_status_buf), "Send failed: %s", err_buf[0] ? err_buf : "Unknown error");
		Printf(PRINT_NONOTIFY, "Send failed: %s\n", err_buf[0] ? err_buf : "Unknown error");
		g_odoom_last_sent_item_name.clear();
		g_odoom_last_sent_qty = 0;
	}
	FBaseCVar* statusVar = FindCVar("odoom_send_status", nullptr);
	if (statusVar && statusVar->GetRealType() == CVAR_String) {
		UCVarValue val; val.String = s_send_status_buf;
		statusVar->SetGenericRep(val, CVAR_String);
	}
	/* Do NOT refetch inventory here; we updated the cache above. Keeps API hits to minimum. */
}

void ODOOM_InventoryInputCaptureFrame(void)
{
	star_sync_pump();

	FBaseCVar* openVar = FindCVar("odoom_inventory_open", nullptr);
	const bool open = (openVar && openVar->GetRealType() == CVAR_Int && openVar->GetGenericRep(CVAR_Int).Int != 0);

	if (open && !g_odoom_inventory_bindings_captured)
	{
		/* Clear arrow, movement, and inventory key bindings so game doesn't receive them (OQuake-style). */
		C_DoCommand("bind uparrow \"\"");
		C_DoCommand("bind downarrow \"\"");
		C_DoCommand("bind leftarrow \"\"");
		C_DoCommand("bind rightarrow \"\"");
		C_DoCommand("bind W \"\"");
		C_DoCommand("bind S \"\"");
		C_DoCommand("bind A \"\"");
		C_DoCommand("bind D \"\"");
		C_DoCommand("bind E \"\"");
		C_DoCommand("bind C \"\"");
		C_DoCommand("bind Z \"\"");
		C_DoCommand("bind X \"\"");
		/* I, O, P cleared so they only affect popup; ZScript will read odoom_key_i/o/p from raw state */
		C_DoCommand("bind I \"\"");
		C_DoCommand("bind O \"\"");
		C_DoCommand("bind P \"\"");
		C_DoCommand("bind enter \"\"");
		C_DoCommand("bind \"KP-Enter\" \"\"");
		g_odoom_inventory_bindings_captured = true;
	}
	else if (!open && g_odoom_inventory_bindings_captured)
	{
		/* Restore default bindings (user can rebind in options). */
		C_DoCommand("bind uparrow \"+forward\"");
		C_DoCommand("bind downarrow \"+back\"");
		C_DoCommand("bind leftarrow \"+left\"");
		C_DoCommand("bind rightarrow \"+right\"");
		C_DoCommand("bind W \"+forward\"");
		C_DoCommand("bind S \"+back\"");
		C_DoCommand("bind A \"+moveleft\"");
		C_DoCommand("bind D \"+moveright\"");
		C_DoCommand("bind E \"+use\"");
		C_DoCommand("bind A \"+moveleft\"");
		C_DoCommand("bind C \"+crouch\"");
		C_DoCommand("bind Z \"+user4\"");
		C_DoCommand("bind X \"+reload\"");
		C_DoCommand("bind I \"+user1\"");
		C_DoCommand("bind O \"+user2\"");
		C_DoCommand("bind P \"+user3\"");
		C_DoCommand("bind enter \"+use\"");
		C_DoCommand("bind \"KP-Enter\" \"+use\"");
		g_odoom_inventory_bindings_captured = false;
	}

	/* Always feed raw key state into CVars so ZScript can open inventory with I (keyIPressed) when closed and drive popup when open. */
	{
		int up   = ODOOM_GetRawKeyDown(VK_UP);
		int down = ODOOM_GetRawKeyDown(VK_DOWN);
		int left = ODOOM_GetRawKeyDown(VK_LEFT);
		int right= ODOOM_GetRawKeyDown(VK_RIGHT);
		int use  = ODOOM_GetRawKeyDown('E');
		int a    = ODOOM_GetRawKeyDown('A');
		int c    = ODOOM_GetRawKeyDown('C');
		int z    = ODOOM_GetRawKeyDown('Z');
		int x    = ODOOM_GetRawKeyDown('X');
		int i    = ODOOM_GetRawKeyDown('I');
		int o    = ODOOM_GetRawKeyDown('O');
		int p    = ODOOM_GetRawKeyDown('P');
		int enter= ODOOM_GetRawKeyDown(VK_RETURN);
		/* Merge Enter into use so ZScript sees keyUsePressed for both E and Enter (confirm/close) */
		use = (use || enter) ? 1 : 0;
		ODOOM_InventorySetKeyState(up, down, left, right, use, a, c, z, x, i, o, p, enter);
	}

	/* Send popup: text input buffer (OQuake-style) and execute send when ZScript requests */
	FBaseCVar* sendOpenVar = FindCVar("odoom_send_popup_open", nullptr);
	/* Capture typed name only while send popup is open. */
	bool sendOpen = false;
	if (sendOpenVar && sendOpenVar->GetRealType() == CVAR_Int)
		sendOpen = (sendOpenVar->GetGenericRep(CVAR_Int).Int != 0);
	if (sendOpen && !g_odoom_send_popup_was_open)
	{
		g_odoom_send_input_buffer.clear();
		for (int i = 0; i < 256; i++) g_odoom_send_key_was_down[i] = false;
		/* Suppress the opener key so A/C/Z/X don't get typed into name on popup open. */
		g_odoom_send_key_was_down['A'] = (ODOOM_GetRawKeyDown('A') != 0);
		g_odoom_send_key_was_down['C'] = (ODOOM_GetRawKeyDown('C') != 0);
		g_odoom_send_key_was_down['Z'] = (ODOOM_GetRawKeyDown('Z') != 0);
		g_odoom_send_key_was_down['X'] = (ODOOM_GetRawKeyDown('X') != 0);
		g_odoom_send_popup_was_open = true;
	}
	if (!sendOpen) {
		g_odoom_send_popup_was_open = false;
		g_odoom_send_input_buffer.clear();
	}

	/* Send status for ZScript: "Sending...", "Item sent.", or "Send failed: ...". Clear when popup closed. */
	FBaseCVar* statusVar = FindCVar("odoom_send_status", nullptr);
	if (statusVar && statusVar->GetRealType() == CVAR_String) {
		static char s_send_status_buf[384];
		if (!sendOpen) {
			s_send_status_buf[0] = '\0';
			UCVarValue val; val.String = s_send_status_buf;
			statusVar->SetGenericRep(val, CVAR_String);
		} else if (star_sync_send_item_in_progress()) {
			std::strncpy(s_send_status_buf, "Sending...", sizeof(s_send_status_buf) - 1);
			s_send_status_buf[sizeof(s_send_status_buf) - 1] = '\0';
			UCVarValue val; val.String = s_send_status_buf;
			statusVar->SetGenericRep(val, CVAR_String);
		}
	}

	if (sendOpen)
	{
		int lastChar = 0;  /* one newly pressed char per frame for ZScript (0=none, 8=backspace, else ASCII) */
#ifdef _WIN32
		/* Backspace */
		{
			int vk = VK_BACK;
			int down = ODOOM_GetRawKeyDown(vk);
			if (down && !g_odoom_send_key_was_down[vk & 0xFF])
			{
				lastChar = 8;
				if (!g_odoom_send_input_buffer.empty())
					g_odoom_send_input_buffer.pop_back();
			}
			g_odoom_send_key_was_down[vk & 0xFF] = (down != 0);
		}
		if (lastChar == 0) {
		/* A-Z (lowercase) */
		for (int vk = 'A'; vk <= 'Z'; vk++)
		{
			int down = ODOOM_GetRawKeyDown(vk);
			if (down && !g_odoom_send_key_was_down[vk & 0xFF] && (int)g_odoom_send_input_buffer.size() < ODOOM_SEND_INPUT_MAX)
			{
				lastChar = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? vk : (vk - 'A' + 'a');
				g_odoom_send_input_buffer += (char)lastChar;
			}
			g_odoom_send_key_was_down[vk & 0xFF] = (down != 0);
		}
		}
		if (lastChar == 0) {
		/* 0-9 */
		for (int vk = '0'; vk <= '9'; vk++)
		{
			int down = ODOOM_GetRawKeyDown(vk);
			if (down && !g_odoom_send_key_was_down[vk & 0xFF] && (int)g_odoom_send_input_buffer.size() < ODOOM_SEND_INPUT_MAX)
			{
				lastChar = vk;
				g_odoom_send_input_buffer += (char)vk;
			}
			g_odoom_send_key_was_down[vk & 0xFF] = (down != 0);
		}
		}
		if (lastChar == 0) {
		/* Space */
		{
			int vk = VK_SPACE;
			int down = ODOOM_GetRawKeyDown(vk);
			if (down && !g_odoom_send_key_was_down[vk & 0xFF] && (int)g_odoom_send_input_buffer.size() < ODOOM_SEND_INPUT_MAX)
			{
				lastChar = ' ';
				g_odoom_send_input_buffer += ' ';
			}
			g_odoom_send_key_was_down[vk & 0xFF] = (down != 0);
		}
		}
		if (lastChar == 0) {
		/* - . */
		{
			int vk_minus = VK_OEM_MINUS;
			int vk_period = VK_OEM_PERIOD;
			int dmin = ODOOM_GetRawKeyDown(vk_minus);
			int dper = ODOOM_GetRawKeyDown(vk_period);
			if (dmin && !g_odoom_send_key_was_down[vk_minus & 0xFF] && (int)g_odoom_send_input_buffer.size() < ODOOM_SEND_INPUT_MAX)
			{
				lastChar = '-';
				g_odoom_send_input_buffer += '-';
			}
			else if (dper && !g_odoom_send_key_was_down[vk_period & 0xFF] && (int)g_odoom_send_input_buffer.size() < ODOOM_SEND_INPUT_MAX)
			{
				lastChar = '.';
				g_odoom_send_input_buffer += '.';
			}
			g_odoom_send_key_was_down[vk_minus & 0xFF] = (dmin != 0);
			g_odoom_send_key_was_down[vk_period & 0xFF] = (dper != 0);
		}
		}
#endif
		/* ZScript reads this and appends to its display string (string CVar may not work in all builds) */
		FBaseCVar* lastCharVar = FindCVar("odoom_send_last_char", nullptr);
		if (lastCharVar && lastCharVar->GetRealType() == CVAR_Int)
		{
			UCVarValue u; u.Int = lastChar;
			lastCharVar->SetGenericRep(u, CVAR_Int);
		}
		/* Also write full line to string CVar for display/send */
		FBaseCVar* lineVar = FindCVar("odoom_send_input_line", nullptr);
		if (lineVar && lineVar->GetRealType() == CVAR_String)
		{
			static char s_send_line_buf[ODOOM_SEND_INPUT_MAX + 1];
			size_t len = g_odoom_send_input_buffer.size();
			if (len > (size_t)ODOOM_SEND_INPUT_MAX) len = (size_t)ODOOM_SEND_INPUT_MAX;
			std::memcpy(s_send_line_buf, g_odoom_send_input_buffer.c_str(), len);
			s_send_line_buf[len] = '\0';
			UCVarValue val;
			val.String = s_send_line_buf;
			lineVar->SetGenericRep(val, CVAR_String);
		}
	}

	/* Execute send when ZScript set odoom_send_do_it=1 */
	FBaseCVar* doItVar = FindCVar("odoom_send_do_it", nullptr);
	if (doItVar && doItVar->GetRealType() == CVAR_Int && doItVar->GetGenericRep(CVAR_Int).Int != 0)
	{
		FBaseCVar* targetVar = FindCVar("odoom_send_target", nullptr);
		FBaseCVar* classVar = FindCVar("odoom_send_item_class", nullptr);
		FBaseCVar* qtyVar = FindCVar("odoom_send_quantity", nullptr);
		FBaseCVar* toClanVar = FindCVar("odoom_send_to_clan", nullptr);
		const char* target = (targetVar && targetVar->GetRealType() == CVAR_String) ? targetVar->GetGenericRep(CVAR_String).String : "";
		const char* itemClass = (classVar && classVar->GetRealType() == CVAR_String) ? classVar->GetGenericRep(CVAR_String).String : "";
		int qty = (qtyVar && qtyVar->GetRealType() == CVAR_Int) ? qtyVar->GetGenericRep(CVAR_Int).Int : 1;
		int toClan = (toClanVar && toClanVar->GetRealType() == CVAR_Int) ? toClanVar->GetGenericRep(CVAR_Int).Int : 0;
		if (qty < 1) qty = 1;
		if (target && target[0] && itemClass && itemClass[0])
		{
			const char* starItemName = (std::strncmp(itemClass, "STAR:", 5) == 0) ? (itemClass + 5) : nullptr;
			if (starItemName && starItemName[0])
			{
				/* STAR item send: use star_sync (background thread + callback via pump, same as Quake). */
				if (StarInitialized())
				{
					if (star_sync_send_item_in_progress())
						Printf("Send already in progress; try again shortly.\n");
					else
					{
						g_odoom_last_sent_item_name = starItemName;
						g_odoom_last_sent_qty = qty;
						star_sync_send_item_start(target, starItemName, qty, toClan ? 1 : 0, nullptr, ODOOM_OnSendItemDone, nullptr);
						/* Show "Sending..." in popup; keep popup open until callback sets result (ZScript shows status). */
						FBaseCVar* sv = FindCVar("odoom_send_status", nullptr);
						if (sv && sv->GetRealType() == CVAR_String) {
							static const char sending[] = "Sending...";
							UCVarValue val; val.String = (char*)sending;
							sv->SetGenericRep(val, CVAR_String);
						}
					}
				}
				else
					Printf("STAR API not initialized; cannot send item. Beam in first.\n");
			}
			else
			{
				/* Local (Doom) item send */
				if (toClan)
					Printf("Send to clan: \"%s\" item \"%s\" x%d (local send not yet implemented).\n", target, itemClass, qty);
				else
					Printf("Send to avatar: \"%s\" item \"%s\" x%d (local send not yet implemented).\n", target, itemClass, qty);
			}
		}
		{ UCVarValue u; u.Int = 0; doItVar->SetGenericRep(u, CVAR_Int); }
		/* Do not close send popup here; ZScript keeps it open and shows Sending.../result, then user closes. */
		g_odoom_send_popup_was_open = false;
	}
}

/** Called from engine input code when building ticcmd: set key state CVars for ZScript. */
void ODOOM_InventorySetKeyState(int up, int down, int left, int right, int use, int a, int c, int z, int x, int i, int o, int p, int enter)
{
	UCVarValue val;
	FBaseCVar* v;
#define SET_KEY_CVAR(name, vint) do { val.Int = (vint); v = FindCVar(name, nullptr); if (v && v->GetRealType() == CVAR_Int) v->SetGenericRep(val, CVAR_Int); } while(0)
	SET_KEY_CVAR("odoom_key_up", up);
	SET_KEY_CVAR("odoom_key_down", down);
	SET_KEY_CVAR("odoom_key_left", left);
	SET_KEY_CVAR("odoom_key_right", right);
	SET_KEY_CVAR("odoom_key_use", use);
	SET_KEY_CVAR("odoom_key_a", a);
	SET_KEY_CVAR("odoom_key_c", c);
	SET_KEY_CVAR("odoom_key_z", z);
	SET_KEY_CVAR("odoom_key_x", x);
	SET_KEY_CVAR("odoom_key_i", i);
	SET_KEY_CVAR("odoom_key_o", o);
	SET_KEY_CVAR("odoom_key_p", p);
	SET_KEY_CVAR("odoom_key_enter", enter);
#undef SET_KEY_CVAR
}

int UZDoom_STAR_GetShowAnorakFace(void)
{
	return g_star_show_anorak_face ? 1 : 0;
}

static std::string TrimAscii(const std::string& s) {
	size_t start = 0;
	while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
	size_t end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
	return s.substr(start, end - start);
}

static bool EqualsNoCase(const std::string& a, const std::string& b) {
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); i++) {
		unsigned char ca = static_cast<unsigned char>(a[i]);
		unsigned char cb = static_cast<unsigned char>(b[i]);
		if (std::tolower(ca) != std::tolower(cb)) return false;
	}
	return true;
}

static std::string ToStarItemName(const char* className) {
	if (!className || !className[0]) return "pickup_item";
	std::string out;
	out.reserve(std::strlen(className) + 8);
	for (const char* p = className; *p; ++p) {
		unsigned char c = static_cast<unsigned char>(*p);
		if (std::isalnum(c)) out.push_back(static_cast<char>(std::tolower(c)));
		else out.push_back('_');
	}
	while (!out.empty() && out.front() == '_') out.erase(out.begin());
	while (!out.empty() && out.back() == '_') out.pop_back();
	if (out.empty()) out = "pickup_item";
	return out;
}

static bool IsMockAnorakCredentials(const std::string& username, const std::string& password) {
	return EqualsNoCase(TrimAscii(username), "anorak") && TrimAscii(password) == "test!";
}

static const char* StarAuthSourceLabel(void) {
	if (!g_star_override_username.empty() || !g_star_override_password.empty() ||
		!g_star_override_jwt.empty() || !g_star_override_api_key.empty() || !g_star_override_avatar_id.empty()) {
		return "cli";
	}
	return "env";
}

static bool ArgEquals(const char* a, const char* b) {
	if (!a || !b) return false;
#ifdef _WIN32
	return _stricmp(a, b) == 0;
#else
	return strcasecmp(a, b) == 0;
#endif
}

#ifdef _WIN32
static std::string WideToUtf8(const wchar_t* w) {
	if (!w || !w[0]) return std::string();
	int size = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
	if (size <= 1) return std::string();
	std::string out;
	out.resize(static_cast<size_t>(size - 1));
	WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], size, nullptr, nullptr);
	return out;
}

static bool PromptForStarCredentials(std::string& usernameOut, std::string& passwordOut) {
	CREDUI_INFOW ui = {};
	ui.cbSize = sizeof(ui);
	ui.pszCaptionText = L"OASIS STAR Beam In";
	ui.pszMessageText = L"Enter your STAR username and password";

	wchar_t username[512] = {};
	wchar_t password[512] = {};
	BOOL save = FALSE;
	DWORD rc = CredUIPromptForCredentialsW(
		&ui,
		L"OASIS_STAR_API",
		nullptr,
		0,
		username,
		static_cast<ULONG>(sizeof(username) / sizeof(username[0])),
		password,
		static_cast<ULONG>(sizeof(password) / sizeof(password[0])),
		&save,
		CREDUI_FLAGS_GENERIC_CREDENTIALS |
		CREDUI_FLAGS_ALWAYS_SHOW_UI |
		CREDUI_FLAGS_DO_NOT_PERSIST |
		CREDUI_FLAGS_EXCLUDE_CERTIFICATES
	);

	if (rc != NO_ERROR) return false;

	usernameOut = WideToUtf8(username);
	passwordOut = WideToUtf8(password);
	SecureZeroMemory(password, sizeof(password));
	return !usernameOut.empty() && !passwordOut.empty();
}
#endif

static const char* GetCliOptionValue(const char* opt) {
	if (!Args || !opt) return nullptr;
	for (int i = 1; i < Args->NumArgs() - 1; i++) {
		const char* arg = Args->GetArg(i);
		if (!ArgEquals(arg, opt)) continue;
		const char* val = Args->GetArg(i + 1);
		if (!val || val[0] == '\0') return nullptr;
		return val;
	}
	return nullptr;
}

static void RefreshStarCliOverridesFromExeArgs(void) {
	if (g_star_cli_loaded) return;
	g_star_cli_loaded = true;

	const char* v = nullptr;
	v = GetCliOptionValue("-star_username"); if (!v) v = GetCliOptionValue("-star_user");
	if (v) g_star_override_username = v;
	v = GetCliOptionValue("-star_password"); if (!v) v = GetCliOptionValue("-star_pass");
	if (v) g_star_override_password = v;
	v = GetCliOptionValue("-star_jwt"); if (!v) v = GetCliOptionValue("-star_token");
	if (v) g_star_override_jwt = v;
	v = GetCliOptionValue("-star_api_key");
	if (v) g_star_override_api_key = v;
	v = GetCliOptionValue("-star_avatar_id");
	if (v) g_star_override_avatar_id = v;
}

static void StarLogInfo(const char* fmt, ...) {
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);
	std::printf("STAR API: %s\n", msg);
	Printf(PRINT_NONOTIFY, TEXTCOLOR_GREEN "STAR API: %s\n", msg);
}

static void StarLogError(const char* fmt, ...) {
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);
	std::printf("STAR API ERROR: %s\n", msg);
	Printf(PRINT_NONOTIFY, TEXTCOLOR_RED "STAR API ERROR: %s\n", msg);
}

static void StarLogRuntimeAuthFailureOnce(const char* reason) {
	if (g_star_logged_runtime_auth_failure) return;
	g_star_logged_runtime_auth_failure = true;
	StarLogError("Not authenticated. STAR sync disabled until beam-in succeeds. Reason: %s", reason ? reason : "(unknown)");
}

static bool HasValue(const char* s) {
	return s && s[0] != '\0';
}

static bool StarShouldUseAnorakFace(void) {
	const char* activeName = HasValue(odoom_star_username) ? odoom_star_username : "";
	std::string u = TrimAscii(activeName);
	return oasis_star_beam_face && !g_star_face_suppressed_for_session && (EqualsNoCase(u, "anorak") || EqualsNoCase(u, "dellams"));
}

/** True if current user is dellams or anorak (for add, bossnft, deploynft). */
static bool StarAllowPrivilegedCommands(void) {
	std::string u = HasValue((const char*)odoom_star_username) ? std::string((const char*)odoom_star_username) : "";
	if (u.empty() && !g_star_effective_username.empty()) u = g_star_effective_username;
	u = TrimAscii(u);
	return EqualsNoCase(u, "dellams") || EqualsNoCase(u, "anorak");
}

static void StarApplyBeamFacePreference(void) {
	g_star_show_anorak_face = g_star_initialized && StarShouldUseAnorakFace();
	oasis_star_anorak_face = g_star_show_anorak_face;
}

static bool StarTryInitializeAndAuthenticate(bool verbose) {
	const bool logVerbose = verbose;
	if (g_star_initialized)
		return true;
	/* Avoid retrying init every touch/door when host is unreachable - only retry when user explicitly runs beamin. */
	if (!logVerbose && g_star_init_failed_this_session && !g_star_client_ready) {
		return false;
	}
	if (logVerbose)
		g_star_init_failed_this_session = false;

	RefreshStarCliOverridesFromExeArgs();

	const char* env_username = getenv("STAR_USERNAME");
	const char* env_password = getenv("STAR_PASSWORD");
	const char* env_api_key = getenv("STAR_API_KEY");
	const char* env_jwt = getenv("STAR_JWT_TOKEN");
	if (!HasValue(env_jwt)) env_jwt = getenv("STAR_JWT");
	const char* env_avatar_id = getenv("STAR_AVATAR_ID");

	g_star_effective_username =
		!g_star_override_username.empty() ? g_star_override_username :
		(HasValue(env_username) ? env_username : "");
	g_star_effective_password =
		!g_star_override_password.empty() ? g_star_override_password :
		(HasValue(env_password) ? env_password : "");
	g_star_effective_api_key =
		!g_star_override_api_key.empty() ? g_star_override_api_key :
		(!g_star_override_jwt.empty() ? g_star_override_jwt :
		(HasValue(env_api_key) ? env_api_key :
		(HasValue(env_jwt) ? env_jwt : "")));
	g_star_effective_avatar_id =
		!g_star_override_avatar_id.empty() ? g_star_override_avatar_id :
		(HasValue(env_avatar_id) ? env_avatar_id : "");

	const char* configuredBaseUrl = odoom_star_api_url;
	g_star_config.base_url = HasValue(configuredBaseUrl) ? configuredBaseUrl : "https://star-api.oasisplatform.world/api";
	g_star_config.api_key = g_star_effective_api_key.empty() ? nullptr : g_star_effective_api_key.c_str();
	g_star_config.avatar_id = g_star_effective_avatar_id.empty() ? nullptr : g_star_effective_avatar_id.c_str();
	g_star_config.timeout_seconds = 30;
	if (logVerbose) {
		StarLogInfo(
			"Init/auth start (base_url=%s, has_api_key=%s, has_avatar_id=%s, has_username=%s, has_password=%s)",
			g_star_config.base_url ? g_star_config.base_url : "(null)",
			HasValue(g_star_config.api_key) ? "yes" : "no",
			HasValue(g_star_config.avatar_id) ? "yes" : "no",
			!g_star_effective_username.empty() ? "yes" : "no",
			!g_star_effective_password.empty() ? "yes" : "no"
		);
	}

	if (!g_star_client_ready) {
		if (logVerbose) StarLogInfo("Calling star_api_init...");
		star_api_result_t init_result = star_api_init(&g_star_config);
		if (init_result != STAR_API_SUCCESS) {
			g_star_init_failed_this_session = true;
			if (logVerbose) StarLogError("star_api_init failed: %s", star_api_get_last_error());
			return false;
		}
		g_star_client_ready = true;
		g_star_init_failed_this_session = false;
		if (logVerbose) StarLogInfo("star_api_init succeeded (interop DLL/API ready).");
	}

	const char* username = g_star_effective_username.empty() ? nullptr : g_star_effective_username.c_str();
	const char* password = g_star_effective_password.empty() ? nullptr : g_star_effective_password.c_str();
	if (HasValue(username) && HasValue(password)) {
		if (star_sync_auth_in_progress()) {
			if (logVerbose) StarLogInfo("SSO auth already in progress.");
			return false;
		}
		if (logVerbose) StarLogInfo("Beaming in... starting async SSO authentication.");
		star_sync_auth_start(username, password, ODOOM_OnAuthDone, nullptr);
		g_star_async_auth_pending = true;
		return false; /* Result will be applied in ODOOM_STAR_PollAsyncAuth. */
	}

	// API key + avatar mode: accept credentials and start inventory in background (no blocking call).
	if (HasValue(g_star_config.api_key) && HasValue(g_star_config.avatar_id)) {
		g_star_initialized = true;
		g_star_init_failed_this_session = false;
		g_star_logged_runtime_auth_failure = false;
		g_star_logged_missing_auth_config = false;
		if (!g_star_effective_username.empty())
			odoom_star_username = g_star_effective_username.c_str();
		else if (!g_star_effective_avatar_id.empty())
			odoom_star_username = g_star_effective_avatar_id.c_str();
		else
			odoom_star_username = "Avatar";
		StarApplyBeamFacePreference();
		if (logVerbose) StarLogInfo("Beam-in (API key/avatar). Starting inventory sync in background.");
		if (!star_sync_inventory_in_progress())
			star_sync_inventory_start(nullptr, 0, "ODOOM", ODOOM_OnInventoryDone, nullptr);
		return true;
	}

	if (logVerbose && !g_star_logged_missing_auth_config) {
		g_star_logged_missing_auth_config = true;
		StarLogError("No authentication configured. Set STAR_USERNAME/STAR_PASSWORD or STAR_API_KEY/STAR_AVATAR_ID.");
	}
	return false;
}

static const char* GetKeycardName(int keynum) {
	switch (keynum) {
		case 1: return "red_keycard";
		case 2: return "blue_keycard";
		case 3: return "yellow_keycard";
		case 4: return "skull_key";
		default: return nullptr;
	}
}

static const char* GetKeycardDescription(int keynum) {
	switch (keynum) {
		case 1: return "Red Keycard - Opens red doors";
		case 2: return "Blue Keycard - Opens blue doors";
		case 3: return "Yellow Keycard - Opens yellow doors";
		case 4: return "Skull Key - Opens skull-marked doors";
		default: return "Key";
	}
}

void UZDoom_STAR_Init(void) {
	star_sync_init();
	/* Load STAR options from oasisstar.json if present (parity with OQuake). */
	{
		std::string path;
		if (ODOOM_FindConfigFile("oasisstar.json", path) && ODOOM_LoadJsonConfig(path.c_str())) {
			g_odoom_json_config_path = path;
			StarLogInfo("Loaded STAR config from: %s", path.c_str());
		}
	}
	/* Always start with default Doom face until explicit beam-in. */
	g_star_show_anorak_face = false;
	oasis_star_anorak_face = false;
	// Safe default bind for ODOOM inventory popup toggle/tab controls.
	// defaultbind does not override user-customized bindings.
	C_DoCommand("defaultbind i +user1");
	C_DoCommand("defaultbind o +user2");
	C_DoCommand("defaultbind p +user3");

	StarLogInfo("STAR bootstrap: Beaming in...");
	if (StarTryInitializeAndAuthenticate(true) && !star_sync_inventory_in_progress())
		star_sync_inventory_start(nullptr, 0, "ODOOM", ODOOM_OnInventoryDone, nullptr);
	Printf(PRINT_NONOTIFY, "STAR: debug=%s auth_source=%s initialized=%s\n",
		g_star_debug_logging ? "on" : "off",
		StarAuthSourceLabel(),
		g_star_initialized ? "yes" : "no");

	/* OASIS / ODOOM welcome splash in console (same style as OQuake) */
	Printf("\n");
	Printf("  ================================================\n");
	Printf("            O A S I S   O D O O M  " ODOOM_VERSION_STR "\n");
	Printf("               By NextGen World Ltd\n");
	Printf("  ================================================\n");
	Printf("\n");
	Printf("  " GAMENAME " " ODOOM_VERSION_STR "\n");
	Printf("  STAR API - Enabling full interoperable games across the OASIS Omniverse!\n");
	Printf("  Type 'star' in console for STAR commands.\n");
	Printf("\n");
	Printf("  Welcome to ODOOM!\n");
	Printf("\n");
}

void UZDoom_STAR_Cleanup(void) {
	ODOOM_SaveStarConfigToFiles(); /* persist STAR options to oasisstar.json on exit */
	if (g_odoom_cached_inventory) {
		star_api_free_item_list(g_odoom_cached_inventory);
		g_odoom_cached_inventory = nullptr;
		ODOOM_UpdateStarInventoryCVars();
	}
	star_sync_cleanup();
	g_star_async_auth_pending = false;
	if (g_star_client_ready) {
		star_api_cleanup();
		g_star_client_ready = false;
		g_star_initialized = false;
		StarLogInfo("Cleaned up STAR API client.");
	}
}

int UZDoom_STAR_PreTouchSpecial(struct AActor* special) {
	if (!special) return 0;
	if (!StarTryInitializeAndAuthenticate(false)) {
		StarLogRuntimeAuthFailureOnce(star_api_get_last_error());
		return 0;
	}

	// OQUAKE runtime key actors in ODOOM: report exact shared-key ids.
	static PClassActor* oqGoldKeyClass = nullptr;
	static PClassActor* oqSilverKeyClass = nullptr;
	if (!oqGoldKeyClass) oqGoldKeyClass = PClass::FindActor("OQGoldKey");
	if (!oqSilverKeyClass) oqSilverKeyClass = PClass::FindActor("OQSilverKey");
	if (oqGoldKeyClass && special->IsKindOf(oqGoldKeyClass)) {
		StarLogInfo("Pickup detected: OQGoldKey (id=%d).", STAR_PICKUP_OQUAKE_GOLD_KEY);
		return STAR_PICKUP_OQUAKE_GOLD_KEY;
	}
	if (oqSilverKeyClass && special->IsKindOf(oqSilverKeyClass)) {
		StarLogInfo("Pickup detected: OQSilverKey (id=%d).", STAR_PICKUP_OQUAKE_SILVER_KEY);
		return STAR_PICKUP_OQUAKE_SILVER_KEY;
	}

	auto kt = PClass::FindActor(NAME_Key);
	if (kt && special->IsKindOf(kt)) {
		AActor* def = GetDefaultByType(special->GetClass());
		if (!def) return 0;

		int keynum = def->special1;
		if (keynum <= 0 || keynum > 4) return 0;
		StarLogInfo("Pickup detected: Doom key special1=%d.", keynum);
		return keynum;
	}

	// Generic inventory sync path for non-key pickups (weapons/ammo/armor/items).
	auto invType = PClass::FindActor(NAME_Inventory);
	if (invType && special->IsKindOf(invType)) {
		const char* cls = special->GetClass()->TypeName.GetChars();
		const char* type = "Item";
		auto weaponType = PClass::FindActor(NAME_Weapon);
		auto ammoType = PClass::FindActor(NAME_Ammo);
		if (weaponType && special->IsKindOf(weaponType)) type = "Weapon";
		else if (ammoType && special->IsKindOf(ammoType)) type = "Ammo";
		else if (cls && (strstr(cls, "Armor") || strstr(cls, "armor"))) type = "Armor";
		else if (cls && (strstr(cls, "Health") || strstr(cls, "health") || strstr(cls, "Medikit") || strstr(cls, "Stimpack"))) type = "Health";

		g_star_pending_item_name = ToStarItemName(cls);
		g_star_pending_item_desc = std::string("Picked up ") + (cls ? cls : "Item");
		g_star_pending_item_type = type;
		g_star_has_pending_item = true;
		StarLogInfo("Pickup detected: %s (type=%s).", cls ? cls : "Inventory", type);
		return STAR_PICKUP_GENERIC_ITEM;
	}

	return 0;
}

void UZDoom_STAR_PostTouchSpecial(int keynum) {
	if (keynum <= 0) return;
	if (!StarTryInitializeAndAuthenticate(false)) {
		StarLogRuntimeAuthFailureOnce(star_api_get_last_error());
		return;
	}

	const char* name = nullptr;
	const char* desc = nullptr;
	const char* itemType = "KeyItem";
	if (keynum == STAR_PICKUP_OQUAKE_GOLD_KEY) {
		name = "gold_key";
		desc = "Gold key - Opens gold doors in OQuake";
	} else if (keynum == STAR_PICKUP_OQUAKE_SILVER_KEY) {
		name = "silver_key";
		desc = "Silver key - Opens silver doors in OQuake";
	} else if (keynum >= 1 && keynum <= 4) {
		name = GetKeycardName(keynum);
		desc = GetKeycardDescription(keynum);
	} else if (keynum == STAR_PICKUP_GENERIC_ITEM && g_star_has_pending_item) {
		name = g_star_pending_item_name.c_str();
		desc = g_star_pending_item_desc.c_str();
		itemType = g_star_pending_item_type.empty() ? "Item" : g_star_pending_item_type.c_str();
	}
	if (!name || !desc) return;

	/* Queue for background sync (like OQuake) so main thread doesn't block. */
	if (g_odoom_pending_sync_count < ODOOM_SYNC_PENDING_MAX) {
		star_sync_local_item_t* p = &g_odoom_pending_sync[g_odoom_pending_sync_count++];
		strncpy(p->name, name, sizeof(p->name) - 1); p->name[sizeof(p->name) - 1] = '\0';
		strncpy(p->description, desc, sizeof(p->description) - 1); p->description[sizeof(p->description) - 1] = '\0';
		strncpy(p->game_source, "ODOOM", sizeof(p->game_source) - 1); p->game_source[sizeof(p->game_source) - 1] = '\0';
		strncpy(p->item_type, itemType, sizeof(p->item_type) - 1); p->item_type[sizeof(p->item_type) - 1] = '\0';
		p->synced = 0;
		g_star_last_pickup_name = name;
		g_star_last_pickup_type = itemType;
		g_star_last_pickup_desc = desc;
		g_star_has_last_pickup = true;
		/* Do not start sync on every pickup; inventory loads once after beam-in only. */
	} else {
		StarLogError("Pending sync queue full; pickup %s not synced.", name);
	}
	if (keynum == STAR_PICKUP_GENERIC_ITEM) {
		g_star_has_pending_item = false;
		g_star_pending_item_name.clear();
		g_star_pending_item_desc.clear();
		g_star_pending_item_type.clear();
	}
}

int UZDoom_STAR_CheckDoorAccess(struct AActor* owner, int keynum, int remote) {
	if (!owner || keynum <= 0) return 0;
	if (!StarTryInitializeAndAuthenticate(false)) {
		StarLogRuntimeAuthFailureOnce(star_api_get_last_error());
		return 0;
	}

	const char* keyname = GetKeycardName(keynum);
	if (!keyname) return 0;

	/* Prefer local cached inventory (loaded once after beam-in) to avoid API calls on every door touch. */
	if (ODOOM_HasItemInCache(keyname)) {
		StarLogInfo("Door access granted via shared inventory key (cache): %s", keyname);
		return 1;
	}

	/* Fallback: cache empty or not loaded yet (edge case) – hit API as last resort. */
	if (star_api_has_item(keyname)) {
		StarLogInfo("Door access granted via shared inventory key (API): %s", keyname);
		bool used = star_api_use_item(keyname, "odoom_door");
		if (!used) {
			StarLogError("star_api_use_item failed for %s: %s", keyname, star_api_get_last_error());
		}
		return 1;
	}

	// IMPORTANT: OQuake keys are intentionally NOT valid for ODOOM doors.
	// Gold/silver keys only open their matching doors in OQuake.

	return 0;
}

//-----------------------------------------------------------------------------
// STAR console command (star <subcmd> [args...]) for testing the STAR API
//-----------------------------------------------------------------------------
static bool StarInitialized(void) {
	return g_star_initialized;
}

CCMD(star)
{
	if (argv.argc() < 2) {
		Printf("\n");
		Printf("  Welcome to ODOOM!\n");
		Printf("\n");
		Printf(TEXTCOLOR_GREEN "STAR API console commands (ODOOM):\n");
		Printf("\n");
		Printf("  star version        - Show integration and API status\n");
		Printf("  star status         - Show init state and last error\n");
		Printf("  star inventory      - List items in STAR inventory\n");
		Printf("  star lastpickup     - Show most recent synced pickup\n");
		Printf("  star has <item>     - Check if you have an item (e.g. red_keycard)\n");
		Printf("  star add <item> [desc] [type] - Add item (e.g. star add red_keycard)\n");
		Printf("  star use <item> [context]     - Use item (e.g. star use red_keycard door)\n");
		Printf("  star quest start <id>        - Start a quest\n");
		Printf("  star quest objective <quest> <obj> - Complete quest objective\n");
		Printf("  star quest complete <id>    - Complete a quest\n");
		Printf("  star bossnft <name> [desc]   - Create boss NFT\n");
		Printf("  star deploynft <nft_id> <game> [loc] - Deploy boss NFT\n");
		Printf("  star pickup keycard <red|blue|yellow|skull> - Add keycard (convenience)\n");
		Printf("  star debug on|off|status - Toggle STAR debug logging in console\n");
		Printf("  star face on|off|status - Toggle beamed-in face switch (default on)\n");
		Printf("  star config        - Show current STAR config (URLs, beam face, stack options)\n");
		Printf("  star config save   - Write config to oasisstar.json now (also saved on exit)\n");
		Printf("  star stack <armor|weapons|powerups|keys> <0|1> - Stack (1) or unlock (0) per category\n");
		Printf("  star seturl <url>       - Set STAR API URL (saved to config)\n");
		Printf("  star setoasisurl <url>  - Set OASIS API URL (saved to config)\n");
		Printf("  star reloadconfig  - Reload from oasisstar.json\n");
		Printf("  star beamin [user pass]|[jwt <token> [avatar]] [-noface] - Beam in/authenticate\n");
		Printf("  star beamout  - Log out / disconnect from STAR\n");
		Printf("\n");
		return;
	}
	const char* sub = argv[1];
	if (strcmp(sub, "pickup") == 0) {
		Printf("\n");
		if (argv.argc() < 4 || strcmp(argv[2], "keycard") != 0) {
			Printf("Usage: star pickup keycard <red|blue|yellow|skull>\n");
			Printf("\n");
			return;
		}
		if (!StarAllowPrivilegedCommands()) { Printf("Only dellams or anorak can use star pickup keycard.\n"); Printf("\n"); return; }
		const char* color = argv[3];
		const char* name = nullptr;
		const char* desc = nullptr;
		if (strcmp(color, "red") == 0)    { name = "red_keycard";  desc = "Red Keycard - Opens red doors"; }
		else if (strcmp(color, "blue") == 0)   { name = "blue_keycard";  desc = "Blue Keycard - Opens blue doors"; }
		else if (strcmp(color, "yellow") == 0) { name = "yellow_keycard"; desc = "Yellow Keycard - Opens yellow doors"; }
		else if (strcmp(color, "skull") == 0)  { name = "skull_key";      desc = "Skull Key - Opens skull-marked doors"; }
		else { Printf("Unknown keycard: %s. Use red|blue|yellow|skull.\n", color); Printf("\n"); return; }
		star_api_result_t r = star_api_add_item(name, desc, "ODOOM", "KeyItem");
		if (r == STAR_API_SUCCESS) Printf("Added %s to STAR inventory.\n", name);
		else Printf("Failed: %s\n", star_api_get_last_error());
		Printf("\n");
		return;
	}
	if (strcmp(sub, "version") == 0) {
		Printf("\n");
		Printf(TEXTCOLOR_GREEN "STAR API integration 1.0 (ODOOM)\n");
		Printf("  Initialized: %s\n", StarInitialized() ? "yes" : "no");
		Printf("  Auth source: %s\n", StarAuthSourceLabel());
		if (!StarInitialized()) Printf("  Last error: %s\n", star_api_get_last_error());
		Printf("\n");
		return;
	}
	if (strcmp(sub, "status") == 0) {
		Printf("\n");
		Printf("STAR API initialized: %s\n", StarInitialized() ? "yes" : "no");
		Printf("STAR API client ready: %s\n", g_star_client_ready ? "yes" : "no");
		Printf("STAR debug logging: %s\n", g_star_debug_logging ? "on" : "off");
		Printf("STAR auth source: %s\n", StarAuthSourceLabel());
		Printf("Last error: %s\n", star_api_get_last_error());
		Printf("\n");
		return;
	}
	if (strcmp(sub, "debug") == 0) {
		Printf("\n");
		if (argv.argc() < 3 || strcmp(argv[2], "status") == 0) {
			Printf("STAR debug logging is %s\n", g_star_debug_logging ? "on" : "off");
			Printf("Usage: star debug on|off|status\n");
			Printf("\n");
			return;
		}
		if (strcmp(argv[2], "on") == 0) {
			g_star_debug_logging = true;
			StarLogInfo("Debug logging enabled.");
			Printf("\n");
			return;
		}
		if (strcmp(argv[2], "off") == 0) {
			g_star_debug_logging = false;
			Printf("STAR API: Debug logging disabled.\n");
			Printf("\n");
			return;
		}
		Printf("Unknown debug option: %s. Use on|off|status.\n", argv[2]);
		Printf("\n");
		return;
	}
	if (strcmp(sub, "face") == 0) {
		Printf("\n");
		if (argv.argc() < 3 || strcmp(argv[2], "status") == 0) {
			Printf("Beam-in face switch is %s\n", oasis_star_beam_face ? "on" : "off");
			Printf("Usage: star face on|off|status\n");
			Printf("\n");
			return;
		}
		if (strcmp(argv[2], "on") == 0) {
			oasis_star_beam_face = true;
			g_star_face_suppressed_for_session = false;
			g_star_show_anorak_face = (g_star_initialized && StarShouldUseAnorakFace());
			oasis_star_anorak_face = g_star_show_anorak_face;
			ODOOM_SaveStarConfigToFiles();
			Printf("Beam-in face switch enabled.\n");
			Printf("\n");
			return;
		}
		if (strcmp(argv[2], "off") == 0) {
			oasis_star_beam_face = false;
			g_star_show_anorak_face = false;
			oasis_star_anorak_face = false;
			ODOOM_SaveStarConfigToFiles();
			Printf("Beam-in face switch disabled.\n");
			Printf("\n");
			return;
		}
		Printf("Unknown face option: %s. Use on|off|status.\n", argv[2]);
		Printf("\n");
		return;
	}
	if (strcmp(sub, "inventory") == 0) {
		Printf("\n");
		if (!StarInitialized()) { Printf("STAR API not initialized. %s\n", star_api_get_last_error()); Printf("\n"); return; }
		star_sync_pump();
		if (g_odoom_cached_inventory) {
			size_t count = g_odoom_cached_inventory->count;
			if (count == 0) { Printf("Inventory is empty.\n"); Printf("\n"); return; }
			Printf("STAR inventory (%zu items):\n", count);
			for (size_t i = 0; i < count; i++) {
				Printf("  %s - %s (%s, %s)\n", g_odoom_cached_inventory->items[i].name, g_odoom_cached_inventory->items[i].description, g_odoom_cached_inventory->items[i].game_source, g_odoom_cached_inventory->items[i].item_type);
			}
			Printf("\n");
			return;
		}
		if (star_sync_inventory_in_progress()) {
			Printf("Syncing... (run 'star inventory' again in a moment)\n");
			Printf("\n");
			return;
		}
		/* Do not start a fetch here; inventory loads once after beam-in only to minimize API hits. */
		Printf("No inventory loaded. Beam in first.\n");
		Printf("\n");
		return;
	}
	if (strcmp(sub, "lastpickup") == 0) {
		Printf("\n");
		if (!g_star_has_last_pickup) {
			Printf("No pickup has been synced to STAR yet in this session.\n");
			Printf("\n");
			return;
		}
		Printf("Last STAR-synced pickup:\n");
		Printf("  name: %s\n", g_star_last_pickup_name.c_str());
		Printf("  type: %s\n", g_star_last_pickup_type.c_str());
		Printf("  desc: %s\n", g_star_last_pickup_desc.c_str());
		Printf("\n");
		return;
	}
	if (strcmp(sub, "has") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star has <item_name>\n"); return; }
		/* Check local cache first (no API hit); fall back to API only if cache not available. */
		bool has = ODOOM_HasItemInCache(argv[2]);
		if (!has && (!g_odoom_cached_inventory || g_odoom_cached_inventory->count == 0))
			has = star_api_has_item(argv[2]);
		Printf("Has '%s': %s\n", argv[2], has ? "yes" : "no");
		return;
	}
	if (strcmp(sub, "add") == 0) {
		if (!StarAllowPrivilegedCommands()) { Printf("Only dellams or anorak can use star add.\n"); return; }
		if (argv.argc() < 3) { Printf("Usage: star add <item_name> [description] [item_type]\n"); return; }
		const char* name = argv[2];
		const char* desc = argv.argc() > 3 ? argv[3] : "Added from console";
		const char* type = argv.argc() > 4 ? argv[4] : "Miscellaneous";
		if (g_odoom_pending_sync_count < ODOOM_SYNC_PENDING_MAX) {
			star_sync_local_item_t* p = &g_odoom_pending_sync[g_odoom_pending_sync_count++];
			strncpy(p->name, name, sizeof(p->name) - 1); p->name[sizeof(p->name) - 1] = '\0';
			strncpy(p->description, desc, sizeof(p->description) - 1); p->description[sizeof(p->description) - 1] = '\0';
			strncpy(p->game_source, "ODOOM", sizeof(p->game_source) - 1); p->game_source[sizeof(p->game_source) - 1] = '\0';
			strncpy(p->item_type, type, sizeof(p->item_type) - 1); p->item_type[sizeof(p->item_type) - 1] = '\0';
			p->synced = 0;
			/* Do not auto-start sync; keep API hits to minimum. */
			Printf("Queued '%s' for sync. Inventory loads once after beam-in.\n", name);
		} else {
			Printf("Sync queue full; try again later.\n");
		}
		return;
	}
	if (strcmp(sub, "use") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star use <item_name> [context]\n"); return; }
		const char* ctx = argv.argc() > 3 ? argv[3] : "console";
		/* If we have it in cache we know we have it; still call API to record use. Fall back to API has+use if cache empty. */
		bool have = ODOOM_HasItemInCache(argv[2]);
		if (!have && (!g_odoom_cached_inventory || g_odoom_cached_inventory->count == 0))
			have = star_api_has_item(argv[2]);
		bool ok = have && star_api_use_item(argv[2], ctx);
		Printf("Use '%s' (context %s): %s\n", argv[2], ctx, ok ? "ok" : "failed");
		if (!ok) Printf("  %s\n", star_api_get_last_error());
		return;
	}
	if (strcmp(sub, "quest") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star quest start|objective|complete ...\n"); return; }
		const char* qsub = argv[2];
		if (strcmp(qsub, "start") == 0) {
			if (argv.argc() < 4) { Printf("Usage: star quest start <quest_id>\n"); return; }
			star_api_result_t r = star_api_start_quest(argv[3]);
			Printf(r == STAR_API_SUCCESS ? "Quest started.\n" : "Failed: %s\n", star_api_get_last_error());
			return;
		}
		if (strcmp(qsub, "objective") == 0) {
			if (argv.argc() < 5) { Printf("Usage: star quest objective <quest_id> <objective_id>\n"); return; }
			star_api_result_t r = star_api_complete_quest_objective(argv[3], argv[4], "ODOOM");
			Printf(r == STAR_API_SUCCESS ? "Objective completed.\n" : "Failed: %s\n", star_api_get_last_error());
			return;
		}
		if (strcmp(qsub, "complete") == 0) {
			if (argv.argc() < 4) { Printf("Usage: star quest complete <quest_id>\n"); return; }
			star_api_result_t r = star_api_complete_quest(argv[3]);
			Printf(r == STAR_API_SUCCESS ? "Quest completed.\n" : "Failed: %s\n", star_api_get_last_error());
			return;
		}
		Printf("Unknown: star quest %s. Use start|objective|complete.\n", qsub);
		return;
	}
	if (strcmp(sub, "bossnft") == 0) {
		if (!StarAllowPrivilegedCommands()) { Printf("Only dellams or anorak can use star bossnft.\n"); return; }
		if (argv.argc() < 3) { Printf("Usage: star bossnft <boss_name> [description]\n"); return; }
		const char* name = argv[2];
		const char* desc = argv.argc() > 3 ? argv[3] : "Boss from UZDoom";
		char nft_id[64] = {};
		star_api_result_t r = star_api_create_boss_nft(name, desc, "ODOOM", "{}", nft_id);
		if (r == STAR_API_SUCCESS) Printf("Boss NFT created. ID: %s\n", nft_id[0] ? nft_id : "(none)");
		else Printf("Failed: %s\n", star_api_get_last_error());
		return;
	}
	if (strcmp(sub, "deploynft") == 0) {
		if (argv.argc() < 4) { Printf("Usage: star deploynft <nft_id> <target_game> [location]\n"); return; }
		const char* loc = argv.argc() > 4 ? argv[4] : "";
		star_api_result_t r = star_api_deploy_boss_nft(argv[2], argv[3], loc);
		Printf(r == STAR_API_SUCCESS ? "NFT deploy requested.\n" : "Failed: %s\n", star_api_get_last_error());
		return;
	}
	if (strcmp(sub, "beamin") == 0) {
		Printf("\n");
		bool hasRuntimeCredentials = false;
		bool usingJwt = false;
		bool noFaceThisLogin = false;
		if (argv.argc() == 2) {
#ifdef _WIN32
			std::string promptUser;
			std::string promptPass;
			if (PromptForStarCredentials(promptUser, promptPass)) {
				// Explicitly prefer secure popup credentials for this beam-in attempt.
				g_star_override_jwt.clear();
				g_star_override_api_key.clear();
				g_star_override_username = promptUser;
				g_star_override_password = promptPass;
				hasRuntimeCredentials = true;
				StarLogInfo("Using credentials from secure beam-in dialog.");
			} else {
				Printf("Beam-in cancelled.\n");
				Printf("\n");
				return;
			}
#else
			Printf("Provide credentials to beam in:\n");
			Printf("  star beamin <username> <password> [-noface]\n");
			Printf("  star beamin jwt <token> [avatar_id] [-noface]\n");
			Printf("\n");
			return;
#endif
		}
		// Optional runtime overrides:
		// - star beamin <username> <password>
		// - star beamin jwt <token> [avatar_id]
		if (argv.argc() >= 4 && strcmp(argv[2], "jwt") != 0) {
			g_star_override_username = argv[2];
			g_star_override_password = argv[3];
			hasRuntimeCredentials = true;
			StarLogInfo("Using runtime credentials from console command.");
		} else if (argv.argc() >= 4 && strcmp(argv[2], "jwt") == 0) {
			g_star_override_jwt = argv[3];
			if (argv.argc() >= 5 && argv[4][0] != '-') g_star_override_avatar_id = argv[4];
			usingJwt = true;
			StarLogInfo("Using runtime JWT token from console command.");
		}
		for (int i = 2; i < argv.argc(); i++) {
			if (ArgEquals(argv[i], "-noface") || ArgEquals(argv[i], "noface")) {
				noFaceThisLogin = true;
				break;
			}
		}
		const bool applyFaceThisLogin = oasis_star_beam_face && !noFaceThisLogin;
		g_star_face_suppressed_for_session = noFaceThisLogin;

		// Temporary local mock path (no STAR API call):
		// username=anorak password=test! enables custom HUD face.
		if (IsMockAnorakCredentials(g_star_override_username, g_star_override_password)) {
			g_star_initialized = true;
			g_star_logged_runtime_auth_failure = false;
			g_star_logged_missing_auth_config = false;
			g_star_effective_username = "anorak";
			odoom_star_username = "anorak";
			StarApplyBeamFacePreference();
			if (!applyFaceThisLogin) {
				g_star_show_anorak_face = false;
				oasis_star_anorak_face = false;
			}
			Printf("Beam-in successful (mock). Welcome, anorak.\n");
			Printf("\n");
			return;
		}

		if (StarInitialized() && !hasRuntimeCredentials && !usingJwt) {
			Printf("You are already beamed in. Use 'star beamout' first if you want to switch account.\n");
			Printf("\n");
			return;
		}

		g_star_show_anorak_face = false;
		oasis_star_anorak_face = false;
		if (hasRuntimeCredentials || usingJwt) {
			// Force a fresh authentication attempt for account switching.
			g_star_initialized = false;
		}
		StarLogInfo("Beaming in...");
		if (StarTryInitializeAndAuthenticate(true)) {
			if (!applyFaceThisLogin) {
				g_star_show_anorak_face = false;
				oasis_star_anorak_face = false;
			}
			Printf("Beam-in successful. Cross-game features enabled.\n");
			Printf("\n");
			return;
		}
		if (g_star_async_auth_pending) {
			Printf("Authenticating... Please wait.\n");
			Printf("\n");
			return;
		}
		g_star_face_suppressed_for_session = false;
		Printf("Beam-in failed: %s\n", star_api_get_last_error());
		Printf("\n");
		return;
	}
	if (strcmp(sub, "beamout") == 0) {
		Printf("\n");
		if (!StarInitialized() && !g_star_client_ready) {
			Printf("You are not beamed in.\n");
			Printf("\n");
			return;
		}
		if (g_star_client_ready) {
			star_api_cleanup();
		}
		g_star_client_ready = false;
		g_star_initialized = false;
		g_star_init_failed_this_session = false;
		g_star_async_auth_pending = false;
		g_star_face_suppressed_for_session = false;
		g_star_effective_username.clear();
		g_star_show_anorak_face = false;
		oasis_star_anorak_face = false;
		odoom_star_username = "";
		Printf("Beam-out successful. Use 'star beamin' to beam in again.\n");
		Printf("\n");
		return;
	}
	if (strcmp(sub, "config") == 0) {
		const char* save_arg = (argv.argc() >= 3) ? argv[2] : nullptr;
		if (save_arg && strcmp(save_arg, "save") == 0) {
			ODOOM_SaveStarConfigToFiles();
			Printf("Config saved to oasisstar.json (if path found). Also saved on exit.\n");
			return;
		}
		const char* star_url = (const char*)odoom_star_api_url;
		const char* oasis_url = (const char*)odoom_oasis_api_url;
		Printf("\n");
		Printf("ODOOM STAR Configuration:\n");
		Printf("  STAR API URL: %s\n", star_url && star_url[0] ? star_url : "(default: https://star-api.oasisplatform.world/api)");
		Printf("  OASIS API URL: %s\n", oasis_url && oasis_url[0] ? oasis_url : "(default: https://api.oasisplatform.world)");
		Printf("  Beam face: %s\n", oasis_star_beam_face ? "on" : "off");
		Printf("  Stack (1) / Unlock (0) - ammo always stacks:\n");
		Printf("    stack_armor:    %s\n", odoom_star_stack_armor ? "1 (stack)" : "0 (unlock)");
		Printf("    stack_weapons:  %s\n", odoom_star_stack_weapons ? "1 (stack)" : "0 (unlock)");
		Printf("    stack_powerups: %s\n", odoom_star_stack_powerups ? "1 (stack)" : "0 (unlock)");
		Printf("    stack_keys:     %s\n", odoom_star_stack_keys ? "1 (stack)" : "0 (unlock)");
		Printf("\n");
		Printf("To set: star seturl <url>   star setoasisurl <url>\n");
		Printf("        star stack <armor|weapons|powerups|keys> <0|1>\n");
		Printf("To save now: star config save (also saved on exit)\n");
		Printf("\n");
		return;
	}
	if (strcmp(sub, "stack") == 0) {
		if (argv.argc() < 4) {
			Printf("Usage: star stack <armor|weapons|powerups|keys> <0|1>\n");
			Printf("  1 = stack (each pickup adds quantity), 0 = unlock (one per type). Ammo always stacks.\n");
			return;
		}
		const char* cat = argv[2];
		const char* val = argv[3];
		int on = (val[0] == '1' && val[1] == '\0') ? 1 : 0;
		if (strcmp(cat, "armor") == 0) { odoom_star_stack_armor = on; }
		else if (strcmp(cat, "weapons") == 0) { odoom_star_stack_weapons = on; }
		else if (strcmp(cat, "powerups") == 0) { odoom_star_stack_powerups = on; }
		else if (strcmp(cat, "keys") == 0) { odoom_star_stack_keys = on; }
		else {
			Printf("Unknown category: %s. Use armor|weapons|powerups|keys (sigils are OQuake-only).\n", cat);
			return;
		}
		ODOOM_SaveStarConfigToFiles();
		Printf("%s set to %s (%s). Config saved.\n", cat, on ? "1" : "0", on ? "stack" : "unlock");
		return;
	}
	if (strcmp(sub, "seturl") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star seturl <star_api_url>\n"); return; }
		odoom_star_api_url = argv[2];
		ODOOM_SaveStarConfigToFiles();
		Printf("STAR API URL set to: %s. Config saved.\n", argv[2]);
		return;
	}
	if (strcmp(sub, "setoasisurl") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star setoasisurl <oasis_api_url>\n"); return; }
		odoom_oasis_api_url = argv[2];
		ODOOM_SaveStarConfigToFiles();
		Printf("OASIS API URL set to: %s. Config saved.\n", argv[2]);
		return;
	}
	if (strcmp(sub, "reloadconfig") == 0) {
		if (!g_odoom_json_config_path.empty() && ODOOM_LoadJsonConfig(g_odoom_json_config_path.c_str())) {
			Printf("Reloaded config from: %s\n", g_odoom_json_config_path.c_str());
			return;
		}
		std::string path;
		if (ODOOM_FindConfigFile("oasisstar.json", path) && ODOOM_LoadJsonConfig(path.c_str())) {
			g_odoom_json_config_path = path;
			Printf("Reloaded config from: %s\n", path.c_str());
			return;
		}
		Printf("Could not find or load oasisstar.json.\n");
		return;
	}
	if (strcmp(sub, "send_avatar") == 0) {
		if (argv.argc() < 4) { Printf("Usage: star send_avatar <username> <item_class>\n"); return; }
		const char* username = argv[2];
		const char* itemClass = argv[3];
		Printf("Send to avatar: \"%s\" item \"%s\" (STAR send API not yet implemented).\n", username, itemClass);
		return;
	}
	if (strcmp(sub, "send_clan") == 0) {
		if (argv.argc() < 4) { Printf("Usage: star send_clan <clan_name> <item_class>\n"); return; }
		const char* clanName = argv[2];
		const char* itemClass = argv[3];
		Printf("Send to clan: \"%s\" item \"%s\" (STAR send API not yet implemented).\n", clanName, itemClass);
		return;
	}
	Printf("Unknown STAR subcommand: %s. Type 'star' for list.\n", sub);
}

CCMD(stam)
{
	// Convenience alias for common typo.
	if (argv.argc() < 2) {
		Printf("Usage: stam beamin|beamout\n");
		return;
	}
	if (strcmp(argv[1], "beamin") == 0) {
		C_DoCommand("star beamin");
		return;
	}
	if (strcmp(argv[1], "beamout") == 0) {
		C_DoCommand("star beamout");
		return;
	}
	Printf("Unknown STAM subcommand: %s. Use beamin|beamout.\n", argv[1]);
}
