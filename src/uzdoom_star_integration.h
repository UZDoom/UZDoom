/**
 * UZDoom - OASIS STAR API Integration
 *
 * Provides integration hooks for UZDoom (GZDoom fork) to connect with the
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
 * otherwise returns 0. Use the return value in UZDoom_STAR_PostTouchSpecial after CallTouch.
 */
int UZDoom_STAR_PreTouchSpecial(struct AActor* special);

/** Call after CallTouch with the key number from PreTouchSpecial (0 = no key). */
void UZDoom_STAR_PostTouchSpecial(int keynum);

/**
 * If local key check failed, try cross-game inventory. Returns true if door/lock
 * should be opened (STAR API had the key and used it).
 */
int UZDoom_STAR_CheckDoorAccess(struct AActor* owner, int keynum, int remote);

#ifdef __cplusplus
}
#endif

#endif /* UZDOOM_STAR_INTEGRATION_H */
