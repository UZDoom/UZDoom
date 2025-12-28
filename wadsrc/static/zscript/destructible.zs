/*
** destructible.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025 UZDoom Maintainers and Contributors
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

struct HealthGroup native play
{
    deprecated("3.8", "Use Level.FindHealthGroup() instead") static clearscope HealthGroup Find(int id)
	{
		return level.FindHealthGroup(id);
	}
    
    readonly int id;
    readonly int health;
    readonly Array<Sector> sectors;
    readonly Array<Line> lines;
    
    native void SetHealth(int newhealth);
}

enum SectorPart
{
    SECPART_None = -1,
    SECPART_Floor = 0,
    SECPART_Ceiling = 1,
    SECPART_3D = 2
}

struct Destructible native play
{
   
    static native void DamageSector(Sector sec, Actor source, int damage, Name damagetype, SectorPart part, vector3 position, bool isradius);
    static native void DamageLinedef(Line def, Actor source, int damage, Name damagetype, int side, vector3 position, bool isradius);
    
    static native void GeometryLineAttack(TraceResults trace, Actor thing, int damage, Name damagetype);
    static native void GeometryRadiusAttack(Actor bombspot, Actor bombsource, int bombdamage, double bombdistance, Name damagetype, double fulldamagedistance);
    static native bool ProjectileHitLinedef(Actor projectile, Line def);
    static native bool ProjectileHitPlane(Actor projectile, SectorPart part);
    
    static clearscope native bool CheckLinedefVulnerable(Line def, int side, SectorPart part);
    static clearscope native bool CheckSectorVulnerable(Sector sec, SectorPart part);
}
