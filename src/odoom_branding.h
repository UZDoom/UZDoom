/**
 * ODOOM branding for OASIS STAR API builds.
 * Included from version.h when OASIS_STAR_API is defined.
 * Version from odoom_version_generated.h (generated from ODOOM/odoom_version.txt).
 */
#ifndef ODOOM_BRANDING_H
#define ODOOM_BRANDING_H

#include "odoom_version_generated.h"

/* Full string for HUD/loading/title: "ODOOM 1.0 (Build 1) (UZDoom x.x.x)". */
#define ODOOM_FULL_VERSION_STR ODOOM_VERSION_STR " (UZDoom " VERSIONSTR ")"

#undef GAMENAME
#undef WGAMENAME
#undef GAMENAMELOWERCASE

#define GAMENAME "ODOOM"
#define WGAMENAME L"ODOOM"
#define GAMENAMELOWERCASE "odoom"

#endif
