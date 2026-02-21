/**
 * OASIS STAR API - Generic game integration layer (async auth, async inventory, local-item sync)
 *
 * Use this from OQUAKE, ODOOM, or any game that links against star_api. It provides:
 * - Async authentication (start on background thread, poll from main thread)
 * - Async inventory refresh (optional sync of local items first, then get_inventory)
 * - Reusable sync logic so games don't duplicate threading/sync code
 *
 * Include star_api.h before this header. Link with star_sync.c (or build as lib) and star_api.
 */

#ifndef STAR_SYNC_H
#define STAR_SYNC_H

#include "star_api.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Call once at game startup (e.g. from OQuake_STAR_Init). Required on Windows to initialize locks. */
void star_sync_init(void);

/** Call at game shutdown (e.g. from OQuake_STAR_Cleanup). Frees any pending inventory result and tears down locks. */
void star_sync_cleanup(void);

/* ---------------------------------------------------------------------------
 * Local item entry: one item to sync to remote (has_item then add_item if missing).
 * name, description, game_source, item_type are inputs; synced is output (1 when synced).
 * Game allocates an array of these and passes to star_sync_inventory_start.
 * --------------------------------------------------------------------------- */
typedef struct star_sync_local_item {
    char name[256];
    char description[512];
    char game_source[64];
    char item_type[64];
    int  synced;  /* output: set to 1 by sync layer when item is on remote */
} star_sync_local_item_t;

/* ---------------------------------------------------------------------------
 * Async authentication
 * --------------------------------------------------------------------------- */

/** Start authentication on a background thread. Call star_sync_auth_poll() from main thread. */
void star_sync_auth_start(const char* username, const char* password);

/** Returns: 0 = still in progress, 1 = finished (call star_sync_auth_get_result), -1 = not started / no result */
int star_sync_auth_poll(void);

/** Get result after poll returned 1. Copies username/avatar_id/error into buffers. Returns 1 on success, 0 on failure. */
int star_sync_auth_get_result(
    int* success_out,
    char* username_buf, size_t username_size,
    char* avatar_id_buf, size_t avatar_id_size,
    char* error_msg_buf, size_t error_msg_size
);

/** Non-zero if an auth is currently in progress */
int star_sync_auth_in_progress(void);

/* ---------------------------------------------------------------------------
 * Async inventory refresh (optionally sync local items first, then get_inventory)
 * --------------------------------------------------------------------------- */

/** Start inventory refresh on a background thread. Syncs local_items (has_item/add_item) then get_inventory.
 *  local_items may be NULL (or count 0) to only fetch inventory. synced_flags may be NULL if no local items. */
void star_sync_inventory_start(
    star_sync_local_item_t* local_items,
    int local_count,
    const char* default_game_source
);

/** Returns: 0 = in progress, 1 = finished (call star_sync_inventory_get_result and free the list), -1 = not started / no result */
int star_sync_inventory_poll(void);

/** Get result after poll returned 1. *list_out is valid until next star_sync_inventory_start or star_sync_inventory_clear_result.
 *  Caller must call star_api_free_item_list() when done. */
int star_sync_inventory_get_result(
    star_item_list_t** list_out,
    star_api_result_t* result_out,
    char* error_msg_buf, size_t error_msg_size
);

/** Clear the stored result (frees the list). Call after you've copied or used the list. */
void star_sync_inventory_clear_result(void);

/** Non-zero if an inventory refresh is currently in progress */
int star_sync_inventory_in_progress(void);

/* ---------------------------------------------------------------------------
 * One-shot sync of a single local item (has_item then add_item if missing).
 * Can be called from main thread; use for immediate sync when e.g. player picks up a key.
 * --------------------------------------------------------------------------- */
star_api_result_t star_sync_single_item(
    const char* name,
    const char* description,
    const char* game_source,
    const char* item_type
);

#ifdef __cplusplus
}
#endif

#endif /* STAR_SYNC_H */
