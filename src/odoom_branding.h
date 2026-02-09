/**
 * ODOOM branding for OASIS STAR API builds.
 * Included from version.h when OASIS_STAR_API is defined.
 * Overrides game name and version so loading, title, HUD, and console show "ODOOM" and build.
 * Bottom-right (and version display) shows ODOOM version with UZDoom version in brackets, e.g. "ODOOM 1.0 (Build 1) (UZDoom 1.2.3)".
 */
#ifndef ODOOM_BRANDING_H
#define ODOOM_BRANDING_H

#define ODOOM_VERSION "1.0"
#define ODOOM_BUILD "1"
#define ODOOM_VERSION_STR ODOOM_VERSION " (Build " ODOOM_BUILD ")"

/* Save UZDoom's version string before we override VERSIONSTR (version.h must define it before including us). */
#define ODOOM_UZDOOM_VER VERSIONSTR

#undef GAMENAME
#undef WGAMENAME
#undef GAMENAMELOWERCASE
#undef VERSIONSTR

#define GAMENAME "ODOOM"
#define WGAMENAME L"ODOOM"
#define GAMENAMELOWERCASE "odoom"
#define VERSIONSTR ODOOM_VERSION_STR " (UZDoom " ODOOM_UZDOOM_VER ")"

#endif
