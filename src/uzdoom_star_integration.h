/**
 * ODOOM - OASIS STAR API Integration
 *
 * Provides integration hooks for ODOOM (UZDoom-based, GZDoom fork) to connect with the
 * OASIS STAR API for cross-game item sharing (keycards, doors, quests).
 *
 * Integration points:
 * - Keycard collection (P_TouchSpecialThing)
 * - Door/lock checks (P_CheckKeys)
 * - Init at startup, cleanup at shutdown
 */

#ifndef UZDOOM_STAR_INTEGRATION_H
#define UZDOOM_STAR_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

struct AActor;

/** Initialize STAR API integration. Call once at game startup (e.g. in D_DoomMain). */
void UZDoom_STAR_Init(void);

/** Cleanup STAR API. Call at game shutdown (e.g. in D_Cleanup). */
void UZDoom_STAR_Cleanup(void);

/**
 * Call before special->CallTouch(toucher). If special is a keycard, returns key number (1-4);
 * otherwise returns 0. Use the return value in UZDoom_STAR_PostTouchSpecial after CallTouch. (Symbol names stay UZDoom_* for engine compatibility.)
 */
int UZDoom_STAR_PreTouchSpecial(struct AActor* special);

/** Call after CallTouch with the key number from PreTouchSpecial (0 = no key). */
void UZDoom_STAR_PostTouchSpecial(int keynum);

/**
 * If local key check failed, try cross-game inventory. Returns true if door/lock
 * should be opened (STAR API had the key and used it).
 */
int UZDoom_STAR_CheckDoorAccess(struct AActor* owner, int keynum, int remote);

/** Call every frame from status bar (when OASIS_STAR_API): when inventory open, clear key bindings (OQuake-style). */
void ODOOM_InventoryInputCaptureFrame(void);

/** Call from engine input when building ticcmd: set odoom_key_* CVars from raw key state (for ZScript). */
void ODOOM_InventorySetKeyState(int up, int down, int left, int right, int use, int a, int c, int z, int x, int i, int o, int p, int enter);

#ifdef __cplusplus
}
#endif

#endif /* UZDOOM_STAR_INTEGRATION_H */
