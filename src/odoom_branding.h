/**
 * ODOOM branding for OASIS STAR API builds.
 * Included from version.h when OASIS_STAR_API is defined.
 * Overrides game name and version so loading, title, HUD, and console show "ODOOM" and build.
 */
#ifndef ODOOM_BRANDING_H
#define ODOOM_BRANDING_H

#define ODOOM_VERSION "1.0"
#define ODOOM_BUILD "1"
#define ODOOM_VERSION_STR ODOOM_VERSION " (Build " ODOOM_BUILD ")"

#undef GAMENAME
#undef WGAMENAME
#undef GAMENAMELOWERCASE
#undef VERSIONSTR

#define GAMENAME "ODOOM"
#define WGAMENAME L"ODOOM"
#define GAMENAMELOWERCASE "odoom"
#define VERSIONSTR ODOOM_VERSION_STR

#endif
