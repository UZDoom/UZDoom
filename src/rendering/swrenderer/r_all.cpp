/*
** r_all.cpp
**
** Just a bunch of imports
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
*/

#include "textures/r_swtexture.h"
#include "r_renderthread.cpp"
#include "r_swrenderer.cpp"
#include "r_swcolormaps.cpp"
#include "drawers/r_draw.cpp"
#include "drawers/r_draw_pal.cpp"
#include "drawers/r_draw_rgba.cpp"
#include "line/r_fogboundary.cpp"
#include "line/r_line.cpp"
#include "line/r_farclip_line.cpp"
#include "line/r_renderdrawsegment.cpp"
#include "line/r_walldraw.cpp"
#include "line/r_wallsetup.cpp"
#include "plane/r_flatplane.cpp"
#include "plane/r_planerenderer.cpp"
#include "plane/r_skyplane.cpp"
#include "plane/r_slopeplane.cpp"
#include "plane/r_visibleplane.cpp"
#include "plane/r_visibleplanelist.cpp"
#include "scene/r_3dfloors.cpp"
#include "scene/r_light.cpp"
#include "scene/r_opaque_pass.cpp"
#include "scene/r_portal.cpp"
#include "scene/r_scene.cpp"
#include "scene/r_translucent_pass.cpp"
#include "segments/r_clipsegment.cpp"
#include "segments/r_drawsegment.cpp"
#include "segments/r_portalsegment.cpp"
#include "things/r_decal.cpp"
#include "things/r_particle.cpp"
#include "things/r_playersprite.cpp"
#include "things/r_sprite.cpp"
#include "things/r_visiblesprite.cpp"
#include "things/r_visiblespritelist.cpp"
#include "things/r_voxel.cpp"
#include "things/r_wallsprite.cpp"
#include "viewport/r_drawerargs.cpp"
#include "viewport/r_skydrawer.cpp"
#include "viewport/r_spandrawer.cpp"
#include "viewport/r_spritedrawer.cpp"
#include "viewport/r_viewport.cpp"
#include "viewport/r_walldrawer.cpp"
