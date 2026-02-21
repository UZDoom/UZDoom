/**
 * OASIS STAR API - Generic game integration layer implementation.
 * Async auth, async inventory (with optional local-item sync), single-item sync.
 * Compiles on Windows (Win32 threads) and elsewhere (pthreads).
 */

#include "star_sync.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

/* Safe copy; always null-terminates, truncates to size-1 */
static void str_copy(char* dst, const char* src, size_t size) {
    if (!size) return;
    if (!src) { dst[0] = '\0'; return; }
    size_t n = 0;
    while (n + 1 < size && src[n]) {
        dst[n] = src[n];
        n++;
    }
    dst[n] = '\0';
}

/* ---------------------------------------------------------------------------
 * Auth state
 * --------------------------------------------------------------------------- */
#define AUTH_USERNAME_SIZE  64
#define AUTH_AVATAR_SIZE    64
#define AUTH_ERROR_SIZE     256

static char g_auth_username_buf[AUTH_USERNAME_SIZE];
static char g_auth_password_buf[64];
static int  g_auth_in_progress = 0;
static int  g_auth_has_result = 0;
static int  g_auth_success = 0;
static char g_auth_username_out[AUTH_USERNAME_SIZE];
static char g_auth_avatar_id_out[AUTH_AVATAR_SIZE];
static char g_auth_error_msg[AUTH_ERROR_SIZE];

#ifdef _WIN32
static CRITICAL_SECTION g_auth_lock;
static HANDLE g_auth_thread = NULL;
#else
static pthread_mutex_t g_auth_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_auth_thread = 0;
#endif

#ifdef _WIN32
static DWORD WINAPI auth_thread_proc(LPVOID param) {
#else
static void* auth_thread_proc(void* param) {
#endif
    char user[AUTH_USERNAME_SIZE], pass[64];
    star_api_result_t auth_result = STAR_API_ERROR_NOT_INITIALIZED;
    star_api_result_t avatar_result = STAR_API_ERROR_NOT_INITIALIZED;
    char avatar_id[AUTH_AVATAR_SIZE] = {0};
    const char* err = NULL;

    (void)param;
#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#endif
    str_copy(user, g_auth_username_buf, sizeof(user));
    str_copy(pass, g_auth_password_buf, sizeof(pass));
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
#else
    pthread_mutex_unlock(&g_auth_lock);
#endif

    auth_result = star_api_authenticate(user, pass);
    if (auth_result == STAR_API_SUCCESS) {
        avatar_result = star_api_get_avatar_id(avatar_id, sizeof(avatar_id));
        if (avatar_result != STAR_API_SUCCESS)
            err = star_api_get_last_error();
    } else {
        err = star_api_get_last_error();
    }

#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#else
    pthread_mutex_lock(&g_auth_lock);
#endif
    g_auth_in_progress = 0;
    g_auth_has_result = 1;
    g_auth_success = (auth_result == STAR_API_SUCCESS && avatar_result == STAR_API_SUCCESS) ? 1 : 0;
    str_copy(g_auth_username_out, user, sizeof(g_auth_username_out));
    str_copy(g_auth_avatar_id_out, avatar_id, sizeof(g_auth_avatar_id_out));
    str_copy(g_auth_error_msg, err ? err : "", sizeof(g_auth_error_msg));
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
    return 0;
#else
    pthread_mutex_unlock(&g_auth_lock);
    return NULL;
#endif
}

void star_sync_auth_start(const char* username, const char* password) {
#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#else
    pthread_mutex_lock(&g_auth_lock);
#endif
    if (g_auth_in_progress) {
#ifdef _WIN32
        LeaveCriticalSection(&g_auth_lock);
#else
        pthread_mutex_unlock(&g_auth_lock);
#endif
        return;
    }
    g_auth_has_result = 0;
    str_copy(g_auth_username_buf, username ? username : "", sizeof(g_auth_username_buf));
    str_copy(g_auth_password_buf, password ? password : "", sizeof(g_auth_password_buf));
    g_auth_in_progress = 1;
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
    g_auth_thread = CreateThread(NULL, 0, auth_thread_proc, NULL, 0, NULL);
#else
    pthread_mutex_unlock(&g_auth_lock);
    pthread_create(&g_auth_thread, NULL, auth_thread_proc, NULL);
#endif
}

int star_sync_auth_poll(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#else
    pthread_mutex_lock(&g_auth_lock);
#endif
    if (g_auth_in_progress) {
#ifdef _WIN32
        LeaveCriticalSection(&g_auth_lock);
#else
        pthread_mutex_unlock(&g_auth_lock);
#endif
        return 0;
    }
    if (g_auth_has_result) {
#ifdef _WIN32
        LeaveCriticalSection(&g_auth_lock);
#else
        pthread_mutex_unlock(&g_auth_lock);
#endif
        return 1;
    }
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
#else
    pthread_mutex_unlock(&g_auth_lock);
#endif
    return -1;
}

int star_sync_auth_get_result(int* success_out,
    char* username_buf, size_t username_size,
    char* avatar_id_buf, size_t avatar_id_size,
    char* error_msg_buf, size_t error_msg_size) {
#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#else
    pthread_mutex_lock(&g_auth_lock);
#endif
    if (!g_auth_has_result) {
#ifdef _WIN32
        LeaveCriticalSection(&g_auth_lock);
#else
        pthread_mutex_unlock(&g_auth_lock);
#endif
        return 0;
    }
    if (success_out) *success_out = g_auth_success;
    if (username_buf && username_size) str_copy(username_buf, g_auth_username_out, username_size);
    if (avatar_id_buf && avatar_id_size) str_copy(avatar_id_buf, g_auth_avatar_id_out, avatar_id_size);
    if (error_msg_buf && error_msg_size) str_copy(error_msg_buf, g_auth_error_msg, error_msg_size);
    g_auth_has_result = 0;
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
#else
    pthread_mutex_unlock(&g_auth_lock);
#endif
    return 1;
}

int star_sync_auth_in_progress(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_auth_lock);
#else
    pthread_mutex_lock(&g_auth_lock);
#endif
    int in_progress = g_auth_in_progress;
#ifdef _WIN32
    LeaveCriticalSection(&g_auth_lock);
#else
    pthread_mutex_unlock(&g_auth_lock);
#endif
    return in_progress;
}

/* ---------------------------------------------------------------------------
 * Inventory state
 * --------------------------------------------------------------------------- */
#define INV_ERROR_SIZE 256

static star_sync_local_item_t* g_inv_local_items = NULL;
static int g_inv_local_count = 0;
static char g_inv_default_game_source[64] = {0};
static int g_inv_in_progress = 0;
static int g_inv_has_result = 0;
static star_item_list_t* g_inv_list = NULL;
static star_api_result_t g_inv_result = STAR_API_ERROR_NOT_INITIALIZED;
static char g_inv_error_msg[INV_ERROR_SIZE] = {0};

#ifdef _WIN32
static CRITICAL_SECTION g_inv_lock;
static HANDLE g_inv_thread = NULL;
#else
static pthread_mutex_t g_inv_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_inv_thread = 0;
#endif

static int g_sync_initialized = 0;

void star_sync_init(void) {
    if (g_sync_initialized) return;
#ifdef _WIN32
    InitializeCriticalSection(&g_auth_lock);
    InitializeCriticalSection(&g_inv_lock);
#endif
    g_sync_initialized = 1;
}

void star_sync_cleanup(void) {
    if (!g_sync_initialized) return;
    star_sync_inventory_clear_result();
#ifdef _WIN32
    DeleteCriticalSection(&g_inv_lock);
    DeleteCriticalSection(&g_auth_lock);
#endif
    g_sync_initialized = 0;
}

#ifdef _WIN32
static DWORD WINAPI inventory_thread_proc(LPVOID param) {
#else
static void* inventory_thread_proc(void* param) {
#endif
    star_sync_local_item_t* local;
    int local_count;
    char default_src[64];
    star_item_list_t* list = NULL;
    star_api_result_t result = STAR_API_ERROR_NOT_INITIALIZED;
    const char* err = NULL;

    (void)param;
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#endif
#ifndef _WIN32
    pthread_mutex_lock(&g_inv_lock);
#endif
    local = g_inv_local_items;
    local_count = g_inv_local_count;
    str_copy(default_src, g_inv_default_game_source, sizeof(default_src));
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
#else
    pthread_mutex_unlock(&g_inv_lock);
#endif

    /* Sync local items first */
    if (local && local_count > 0 && default_src[0]) {
        int i;
        for (i = 0; i < local_count; i++) {
            if (local[i].synced) continue;
            if (star_api_has_item(local[i].name)) {
                local[i].synced = 1;
            } else {
                star_api_result_t add_res = star_api_add_item(
                    local[i].name,
                    local[i].description,
                    local[i].game_source[0] ? local[i].game_source : default_src,
                    local[i].item_type);
                if (add_res == STAR_API_SUCCESS)
                    local[i].synced = 1;
            }
        }
    }

    result = star_api_get_inventory(&list);
    if (result != STAR_API_SUCCESS) {
        err = star_api_get_last_error();
        if (!err || !err[0]) err = "Unknown error";
    } else if (!list) {
        result = STAR_API_ERROR_API_ERROR;
        err = "Inventory API returned success but no data";
    }

#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    g_inv_in_progress = 0;
    g_inv_has_result = 1;
    if (g_inv_list)
        star_api_free_item_list(g_inv_list);
    g_inv_list = list;
    g_inv_result = result;
    str_copy(g_inv_error_msg, err ? err : "", sizeof(g_inv_error_msg));
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
    return 0;
#else
    pthread_mutex_unlock(&g_inv_lock);
    return NULL;
#endif
}

void star_sync_inventory_start(star_sync_local_item_t* local_items,
    int local_count,
    const char* default_game_source) {
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    if (g_inv_in_progress) {
#ifdef _WIN32
        LeaveCriticalSection(&g_inv_lock);
#else
        pthread_mutex_unlock(&g_inv_lock);
#endif
        return;
    }
    if (g_inv_list) {
        star_api_free_item_list(g_inv_list);
        g_inv_list = NULL;
    }
    g_inv_has_result = 0;
    g_inv_local_items = local_items;
    g_inv_local_count = local_count < 0 ? 0 : local_count;
    str_copy(g_inv_default_game_source, default_game_source ? default_game_source : "", sizeof(g_inv_default_game_source));
    g_inv_in_progress = 1;
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
    g_inv_thread = CreateThread(NULL, 0, inventory_thread_proc, NULL, 0, NULL);
#else
    pthread_mutex_unlock(&g_inv_lock);
    pthread_create(&g_inv_thread, NULL, inventory_thread_proc, NULL);
#endif
}

int star_sync_inventory_poll(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    if (g_inv_in_progress) {
#ifdef _WIN32
        LeaveCriticalSection(&g_inv_lock);
#else
        pthread_mutex_unlock(&g_inv_lock);
#endif
        return 0;
    }
    if (g_inv_has_result) {
#ifdef _WIN32
        LeaveCriticalSection(&g_inv_lock);
#else
        pthread_mutex_unlock(&g_inv_lock);
#endif
        return 1;
    }
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
#else
    pthread_mutex_unlock(&g_inv_lock);
#endif
    return -1;
}

int star_sync_inventory_get_result(star_item_list_t** list_out,
    star_api_result_t* result_out,
    char* error_msg_buf, size_t error_msg_size) {
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    if (!g_inv_has_result) {
#ifdef _WIN32
        LeaveCriticalSection(&g_inv_lock);
#else
        pthread_mutex_unlock(&g_inv_lock);
#endif
        return 0;
    }
    if (list_out) *list_out = g_inv_list;
    if (result_out) *result_out = g_inv_result;
    if (error_msg_buf && error_msg_size) str_copy(error_msg_buf, g_inv_error_msg, error_msg_size);
    g_inv_has_result = 0;
    g_inv_list = NULL; /* ownership transferred to caller */
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
#else
    pthread_mutex_unlock(&g_inv_lock);
#endif
    return 1;
}

void star_sync_inventory_clear_result(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    if (g_inv_list) {
        star_api_free_item_list(g_inv_list);
        g_inv_list = NULL;
    }
    g_inv_has_result = 0;
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
#else
    pthread_mutex_unlock(&g_inv_lock);
#endif
}

int star_sync_inventory_in_progress(void) {
#ifdef _WIN32
    EnterCriticalSection(&g_inv_lock);
#else
    pthread_mutex_lock(&g_inv_lock);
#endif
    int in_progress = g_inv_in_progress;
#ifdef _WIN32
    LeaveCriticalSection(&g_inv_lock);
#else
    pthread_mutex_unlock(&g_inv_lock);
#endif
    return in_progress;
}

star_api_result_t star_sync_single_item(const char* name,
    const char* description,
    const char* game_source,
    const char* item_type) {
    if (!name || !name[0]) return STAR_API_ERROR_INVALID_PARAM;
    if (star_api_has_item(name))
        return STAR_API_SUCCESS;
    return star_api_add_item(
        name,
        description ? description : "",
        game_source ? game_source : "",
        item_type ? item_type : "");
}
