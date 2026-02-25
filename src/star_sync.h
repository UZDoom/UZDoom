/**
 * OASIS STAR API - Generic game integration layer (async auth, async inventory, local-item sync)
 *
 * Use this from OQUAKE, ODOOM, or any game that links against star_api. It provides:
 * - Async authentication (background thread; completion via callback from main thread)
 * - Async inventory refresh (background thread; completion via callback from main thread)
 * - Async send item (background thread; completion via callback from main thread)
 * - Reusable sync logic so games don't duplicate threading/sync code
 *
 * All completion callbacks are invoked on the main thread when you call star_sync_pump().
 * Call star_sync_pump() once per frame; no per-frame polling of individual operations.
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

/**
 * Call once per frame from the main thread. Runs any pending auth/inventory/send_item completion
 * callbacks on the main thread. More efficient than polling each operation every frame.
 */
void star_sync_pump(void);

/* ---------------------------------------------------------------------------
 * Local item entry: one item to sync to remote (has_item then add_item if missing).
 * name, description, game_source, item_type are inputs; synced is output (1 when synced).
 * nft_id: optional; if set, add_item is called with this NFT ID (item is linked to NFTHolon).
 * Game allocates an array of these and passes to star_sync_inventory_start.
 * --------------------------------------------------------------------------- */
typedef struct star_sync_local_item {
    char name[256];
    char description[512];
    char game_source[64];
    char item_type[64];
    char nft_id[128];  /* optional; empty = no NFT. When set, add_item stores NFTId in item MetaData. */
    int  synced;  /* output: set to 1 by sync layer when item is on remote */
} star_sync_local_item_t;

/* ---------------------------------------------------------------------------
 * Async authentication
 * --------------------------------------------------------------------------- */

/** Optional completion callback: invoked from main thread when star_sync_pump() sees auth finished. In the callback, call star_sync_auth_get_result() to get the result. */
typedef void (*star_sync_auth_on_done_fn)(void* user_data);

/** Start authentication on a background thread. When done, on_done(user_data) is invoked from main thread when you call star_sync_pump(). Pass NULL to use polling (star_sync_auth_poll/get_result). */
void star_sync_auth_start(const char* username, const char* password, star_sync_auth_on_done_fn on_done, void* user_data);

/** Returns: 0 = still in progress, 1 = finished (call star_sync_auth_get_result), -1 = not started / no result. Only needed if not using callbacks. */
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

/** Optional completion callback: invoked from main thread when star_sync_pump() sees inventory finished. In the callback, call star_sync_inventory_get_result() then star_sync_inventory_clear_result() when done. Pass NULL to use polling. */
typedef void (*star_sync_inventory_on_done_fn)(void* user_data);

/** Start inventory refresh on a background thread. Syncs local_items (has_item/add_item) then get_inventory.
 *  local_items may be NULL (or count 0) to only fetch inventory.
 *  on_done and on_done_user: optional; if non-NULL, on_done(user_data) is called from main thread in star_sync_pump(). Pass NULL, NULL to use polling. */
void star_sync_inventory_start(
    star_sync_local_item_t* local_items,
    int local_count,
    const char* default_game_source,
    star_sync_inventory_on_done_fn on_done,
    void* on_done_user
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
    const char* item_type,
    const char* nft_id  /* NULL or empty for non-NFT items */
);

/* ---------------------------------------------------------------------------
 * Async send item (to avatar or clan) - same background-thread pattern as auth/inventory.
 * --------------------------------------------------------------------------- */

/** Optional completion callback: invoked from main thread when star_sync_pump() sees send finished. In the callback, call star_sync_send_item_get_result() to get success/error. */
typedef void (*star_sync_send_item_on_done_fn)(void* user_data);

/** Start send-item on a background thread. to_clan: 1 = send to clan, 0 = send to avatar. item_id can be NULL. When done, on_done(user_data) is invoked from main thread in star_sync_pump(). Pass NULL to use polling. */
void star_sync_send_item_start(const char* target, const char* item_name, int quantity, int to_clan, const char* item_id, star_sync_send_item_on_done_fn on_done, void* user_data);

/** Returns: 0 = in progress, 1 = finished (call star_sync_send_item_get_result), -1 = not started / no result. Only needed if not using callbacks. */
int star_sync_send_item_poll(void);

/** Get result after poll returned 1. success_out: 1 = success, 0 = failure. Returns 1 if result was consumed. */
int star_sync_send_item_get_result(int* success_out, char* error_msg_buf, size_t error_msg_size);

/** Non-zero if a send is currently in progress */
int star_sync_send_item_in_progress(void);

/* ---------------------------------------------------------------------------
 * Async use item (e.g. door/key) - runs on background thread so API is off main thread.
 * --------------------------------------------------------------------------- */

/** Optional completion callback: invoked from main thread when star_sync_pump() sees use-item finished. */
typedef void (*star_sync_use_item_on_done_fn)(void* user_data);

/** Start use-item on a background thread. When done, on_done(user_data) is invoked from main thread in star_sync_pump(). Pass NULL for polling. */
void star_sync_use_item_start(const char* item_name, const char* context, star_sync_use_item_on_done_fn on_done, void* user_data);

/** Get result after use-item finished. success_out: 1 = success, 0 = failure. Returns 1 if result was consumed. */
int star_sync_use_item_get_result(int* success_out, char* error_msg_buf, size_t error_msg_size);

/** Non-zero if a use-item is currently in progress */
int star_sync_use_item_in_progress(void);

#ifdef __cplusplus
}
#endif

#endif /* STAR_SYNC_H */
