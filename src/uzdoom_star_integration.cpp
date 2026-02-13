/**
 * ODOOM - OASIS STAR API Integration Implementation
 *
 * Build this file as part of ODOOM (UZDoom) with STAR_API_DIR pointing to OASIS NativeWrapper.
 * Keycard pickups are reported to STAR; door/lock checks can use cross-game inventory.
 * In-game console: "star" command for testing (star version, star inventory, star add, etc.).
 */

#include "uzdoom_star_integration.h"
#include "star_api.h"
#include "odoom_branding.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <string>

/* ODOOM (UZDoom) headers for key detection */
#include "gamedata/a_keys.h"
#include "playsim/actor.h"
#include "gamedata/info.h"
#include "vm.h"
#include "c_dispatch.h"
#include "c_console.h"
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
CVAR(Bool, oasis_star_anorak_face, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, odoom_oq_monster_yoffset, -50, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

static bool IsMockAnorakCredentials(const std::string& username, const std::string& password) {
	return username == "anorak" && password == "test!";
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

static bool StarTryInitializeAndAuthenticate(bool verbose) {
	const bool logVerbose = verbose;
	if (g_star_initialized) return true;

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

	g_star_config.base_url = "https://star-api.oasisplatform.world/api";
	g_star_config.api_key = g_star_effective_api_key.empty() ? nullptr : g_star_effective_api_key.c_str();
	g_star_config.avatar_id = g_star_effective_avatar_id.empty() ? nullptr : g_star_effective_avatar_id.c_str();
	g_star_config.timeout_seconds = 10;
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
			if (logVerbose) StarLogError("star_api_init failed: %s", star_api_get_last_error());
			return false;
		}
		g_star_client_ready = true;
		if (logVerbose) StarLogInfo("star_api_init succeeded (interop DLL/API ready).");
	}

	const char* username = g_star_effective_username.empty() ? nullptr : g_star_effective_username.c_str();
	const char* password = g_star_effective_password.empty() ? nullptr : g_star_effective_password.c_str();
	if (HasValue(username) && HasValue(password)) {
		if (logVerbose) StarLogInfo("Beaming in... authenticating avatar via SSO.");
		star_api_result_t auth = star_api_authenticate(username, password);
		if (auth == STAR_API_SUCCESS) {
			g_star_initialized = true;
			g_star_logged_runtime_auth_failure = false;
			g_star_logged_missing_auth_config = false;
			if (logVerbose) StarLogInfo("Beam-in successful (SSO). Cross-game features enabled.");
			return true;
		}
		if (logVerbose) StarLogError("Beam-in failed (SSO): %s", star_api_get_last_error());
	}

	// API key + avatar mode: verify we can access inventory for this avatar.
	if (HasValue(g_star_config.api_key) && HasValue(g_star_config.avatar_id)) {
		if (logVerbose) StarLogInfo("Beaming in... verifying API key/avatar via star_api_get_inventory.");
		star_item_list_t* list = nullptr;
		star_api_result_t inv = star_api_get_inventory(&list);
		if (inv == STAR_API_SUCCESS) {
			g_star_initialized = true;
			g_star_logged_runtime_auth_failure = false;
			g_star_logged_missing_auth_config = false;
			size_t count = list ? list->count : 0;
			if (list) star_api_free_item_list(list);
			if (logVerbose) StarLogInfo("Beam-in successful (API key/avatar). Inventory items=%zu. Cross-game features enabled.", count);
			return true;
		}
		if (list) star_api_free_item_list(list);
		if (logVerbose) StarLogError("Beam-in failed (API key/avatar verify): %s", star_api_get_last_error());
		return false;
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
	// Safe default bind for ODOOM inventory popup toggle/tab controls.
	// defaultbind does not override user-customized bindings.
	C_DoCommand("defaultbind i +user1");
	C_DoCommand("defaultbind o +user2");
	C_DoCommand("defaultbind p +user3");

	StarLogInfo("STAR bootstrap: Beaming in...");
	StarTryInitializeAndAuthenticate(true);
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
	if (!kt || !special->IsKindOf(kt)) return 0;

	AActor* def = GetDefaultByType(special->GetClass());
	if (!def) return 0;

	int keynum = def->special1;
	if (keynum <= 0 || keynum > 4) return 0;
	StarLogInfo("Pickup detected: Doom key special1=%d.", keynum);

	return keynum;
}

void UZDoom_STAR_PostTouchSpecial(int keynum) {
	if (keynum <= 0) return;
	if (!StarTryInitializeAndAuthenticate(false)) {
		StarLogRuntimeAuthFailureOnce(star_api_get_last_error());
		return;
	}

	const char* name = nullptr;
	const char* desc = nullptr;
	if (keynum == STAR_PICKUP_OQUAKE_GOLD_KEY) {
		name = "gold_key";
		desc = "Gold key - Opens gold doors in OQuake";
	} else if (keynum == STAR_PICKUP_OQUAKE_SILVER_KEY) {
		name = "silver_key";
		desc = "Silver key - Opens silver doors in OQuake";
	} else if (keynum >= 1 && keynum <= 4) {
		name = GetKeycardName(keynum);
		desc = GetKeycardDescription(keynum);
	}
	if (!name || !desc) return;

	StarLogInfo("Calling star_api_add_item(name=%s, game=ODOOM, type=KeyItem)...", name);
	star_api_result_t result = star_api_add_item(name, desc, "ODOOM", "KeyItem");
	if (result == STAR_API_SUCCESS) {
		StarLogInfo("star_api_add_item success: %s added to shared inventory.", name);
	} else {
		StarLogError("star_api_add_item failed for %s: %s", name, star_api_get_last_error());
	}
}

int UZDoom_STAR_CheckDoorAccess(struct AActor* owner, int keynum, int remote) {
	if (!owner || keynum <= 0) return 0;
	if (!StarTryInitializeAndAuthenticate(false)) {
		StarLogRuntimeAuthFailureOnce(star_api_get_last_error());
		return 0;
	}

	/* 1) Check Doom keycard in cross-game inventory */
	const char* keyname = GetKeycardName(keynum);
	if (keyname && star_api_has_item(keyname)) {
		StarLogInfo("Door access granted via shared inventory key: %s", keyname);
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
		Printf("  star beamin [user pass]|[jwt <token> [avatar]] - Beam in/authenticate\n");
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
	if (strcmp(sub, "inventory") == 0) {
		Printf("\n");
		if (!StarInitialized()) { Printf("STAR API not initialized. %s\n", star_api_get_last_error()); Printf("\n"); return; }
		star_item_list_t* list = nullptr;
		star_api_result_t r = star_api_get_inventory(&list);
		if (r != STAR_API_SUCCESS) {
			Printf("Failed to get inventory: %s\n", star_api_get_last_error());
			Printf("\n");
			return;
		}
		if (!list || list->count == 0) { Printf("Inventory is empty.\n"); if (list) star_api_free_item_list(list); Printf("\n"); return; }
		Printf("STAR inventory (%zu items):\n", list->count);
		for (size_t i = 0; i < list->count; i++) {
			Printf("  %s - %s (%s, %s)\n", list->items[i].name, list->items[i].description, list->items[i].game_source, list->items[i].item_type);
		}
		star_api_free_item_list(list);
		Printf("\n");
		return;
	}
	if (strcmp(sub, "has") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star has <item_name>\n"); return; }
		bool has = star_api_has_item(argv[2]);
		Printf("Has '%s': %s\n", argv[2], has ? "yes" : "no");
		return;
	}
	if (strcmp(sub, "add") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star add <item_name> [description] [item_type]\n"); return; }
		const char* name = argv[2];
		const char* desc = argv.argc() > 3 ? argv[3] : "Added from console";
		const char* type = argv.argc() > 4 ? argv[4] : "Miscellaneous";
		star_api_result_t r = star_api_add_item(name, desc, "ODOOM", type);
		if (r == STAR_API_SUCCESS) Printf("Added '%s' to STAR inventory.\n", name);
		else Printf("Failed to add '%s': %s\n", name, star_api_get_last_error());
		return;
	}
	if (strcmp(sub, "use") == 0) {
		if (argv.argc() < 3) { Printf("Usage: star use <item_name> [context]\n"); return; }
		const char* ctx = argv.argc() > 3 ? argv[3] : "console";
		bool ok = star_api_use_item(argv[2], ctx);
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
		if (StarInitialized()) {
			if (argv.argc() >= 4 && strcmp(argv[2], "jwt") != 0) {
				std::string user = argv[2];
				std::string pass = argv[3];
				if (IsMockAnorakCredentials(user, pass)) {
					oasis_star_anorak_face = true;
					Printf("Beam-in face profile applied (mock anorak).\n");
					Printf("\n");
					return;
				}
			}
			Printf("You are already beamed in. Use 'star beamout' first if you want to switch account.\n");
			Printf("\n");
			return;
		}
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
				StarLogInfo("Using credentials from secure beam-in dialog.");
			} else {
				Printf("Beam-in cancelled.\n");
				Printf("\n");
				return;
			}
#else
			Printf("Provide credentials to beam in:\n");
			Printf("  star beamin <username> <password>\n");
			Printf("  star beamin jwt <token> [avatar_id]\n");
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
			StarLogInfo("Using runtime credentials from console command.");
		} else if (argv.argc() >= 4 && strcmp(argv[2], "jwt") == 0) {
			g_star_override_jwt = argv[3];
			if (argv.argc() >= 5) g_star_override_avatar_id = argv[4];
			StarLogInfo("Using runtime JWT token from console command.");
		}

		// Temporary local mock path (no STAR API call):
		// username=anorak password=test! enables custom HUD face.
		if (IsMockAnorakCredentials(g_star_override_username, g_star_override_password)) {
			g_star_initialized = true;
			g_star_logged_runtime_auth_failure = false;
			g_star_logged_missing_auth_config = false;
			oasis_star_anorak_face = true;
			Printf("Beam-in successful (mock). Welcome, anorak.\n");
			Printf("\n");
			return;
		}

		oasis_star_anorak_face = false;
		StarLogInfo("Beaming in...");
		if (StarTryInitializeAndAuthenticate(true)) {
			Printf("Beam-in successful. Cross-game features enabled.\n");
			Printf("\n");
			return;
		}
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
		oasis_star_anorak_face = false;
		Printf("Beam-out successful. Use 'star beamin' to beam in again.\n");
		Printf("\n");
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
