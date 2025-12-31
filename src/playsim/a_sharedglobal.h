/*
** a_sharedglobal.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2005-2016 Christoph Oelckers
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
** Code written prior to 2026 is also licensed under:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**---------------------------------------------------------------------------
**
*/

#ifndef __A_SHAREDGLOBAL_H__
#define __A_SHAREDGLOBAL_H__

#include "info.h"
#include "actor.h"

class FDecalTemplate;
struct vertex_t;
struct side_t;
struct F3DFloor;
class DBaseDecal;
struct SpreadInfo;

DBaseDecal *ShootDecal(FLevelLocals *Level, const FDecalTemplate *tpl, sector_t *sec, double x, double y, double z, DAngle angle, double tracedist, bool permanent);
void SprayDecal(AActor *shooter, const char *name,double distance = 172., DVector3 offset = DVector3(0., 0., 0.), DVector3 direction = DVector3(0., 0., 0.), bool useBloodColor = false, uint32_t decalColor = 0, FTranslationID translation = NO_TRANSLATION);

class DBaseDecal : public DThinker
{
	DECLARE_CLASS (DBaseDecal, DThinker)
	HAS_OBJECT_POINTERS
public:
	static const int DEFAULT_STAT = STAT_DECAL;
	void Construct(double z = 0);
	void Construct(const AActor *actor);
	void Construct(const DBaseDecal *basis);

	void Serialize(FSerializer &arc);
	void OnDestroy() override;
	virtual void Expired() {}	// For thinkers that can remove their decal. For impact decal bookkeeping.
	FTextureID StickToWall(side_t *wall, double x, double y, F3DFloor * ffloor);
	double GetRealZ (const side_t *wall) const;
	void SetShade (uint32_t rgb);
	void SetShade (int r, int g, int b);
	void SetTranslation(FTranslationID trans)
	{
		Translation = trans;
	}

	void Spread (const FDecalTemplate *tpl, side_t *wall, double x, double y, double z, F3DFloor * ffloor);
	void GetXY (side_t *side, double &x, double &y) const;

	DBaseDecal *WallNext = nullptr, *WallPrev = nullptr;

	double LeftDistance = 0;
	double Z;
	double ScaleX = 1, ScaleY = 1;
	double Alpha = 1;
	uint32_t AlphaColor = 0;
	FTranslationID Translation = NO_TRANSLATION;
	FTextureID PicNum;
	uint32_t RenderFlags = 0;
	FRenderStyle RenderStyle;
	side_t *Side = nullptr;
	sector_t *Sector = nullptr;

protected:
	virtual DBaseDecal *CloneSelf(const FDecalTemplate *tpl, double x, double y, double z, side_t *wall, F3DFloor * ffloor) const;
	void CalcFracPos(side_t *wall, double x, double y);
	void Remove ();

	static void SpreadLeft (double r, vertex_t *v1, side_t *feelwall, F3DFloor *ffloor, SpreadInfo *spread);
	static void SpreadRight (double r, side_t *feelwall, double wallsize, F3DFloor *ffloor, SpreadInfo *spread);
};

class DImpactDecal : public DBaseDecal
{
	DECLARE_CLASS (DImpactDecal, DBaseDecal)
public:
	static const int DEFAULT_STAT = STAT_AUTODECAL;
	void Construct(double z = 0)
	{
		Super::Construct(z);
	}
	void Construct(side_t *wall, const FDecalTemplate *templ);

	static DBaseDecal *StaticCreate(FLevelLocals *Level, const char *name, const DVector3 &pos, side_t *wall, F3DFloor * ffloor, PalEntry color = 0, FTranslationID translation = NO_TRANSLATION);
	static DBaseDecal *StaticCreate(FLevelLocals *Level, const FDecalTemplate *tpl, const DVector3 &pos, side_t *wall, F3DFloor * ffloor, PalEntry color = 0, FTranslationID translation = NO_TRANSLATION, bool permanent = false);

	void BeginPlay ();
	void Expired() override;

protected:
	DBaseDecal *CloneSelf(const FDecalTemplate *tpl, double x, double y, double z, side_t *wall, F3DFloor * ffloor) const;
	void CheckMax ();
};

class DFlashFader : public DThinker
{
	DECLARE_CLASS (DFlashFader, DThinker)
	HAS_OBJECT_POINTERS
public:
	void Construct(float r1, float g1, float b1, float a1,
				 float r2, float g2, float b2, float a2,
				 float time, AActor *who, bool terminate = false);
	void OnDestroy() override;
	void Serialize(FSerializer &arc);
	void Tick ();
	AActor *WhoFor() { return ForWho; }
	void Cancel ();
	

protected:
	float Blends[2][4];
	int TotalTics;
	int RemainingTics;
	TObjPtr<AActor*> ForWho;
	bool Terminate;
	void SetBlend (float time);
};

enum
{
	QF_RELATIVE =		1,
	QF_SCALEDOWN =		1 << 1,
	QF_SCALEUP =		1 << 2,
	QF_MAX =			1 << 3,
	QF_FULLINTENSITY =	1 << 4,
	QF_WAVE =			1 << 5,
	QF_3D =				1 << 6,
	QF_GROUNDONLY =		1 << 7,
	QF_AFFECTACTORS =	1 << 8,
	QF_SHAKEONLY =		1 << 9,
	QF_DAMAGEFALLOFF =	1 << 10,
};

struct FQuakeJiggers
{
	DVector3 Intensity = {};
	DVector3 RelIntensity = {};
	DVector3 Offset = {};
	DVector3 RelOffset = {};
	double RollIntensity = 0.0, RollWave = 0.0;
};

class DEarthquake : public DThinker
{
	DECLARE_CLASS (DEarthquake, DThinker)
	HAS_OBJECT_POINTERS
public:
	static const int DEFAULT_STAT = STAT_EARTHQUAKE;
	void Construct(AActor *center, double intensityX, double intensityY, double intensityZ, int duration,
		double damrad, double tremrad, FSoundID quakesfx, int flags, 
		double waveSpeedX, double waveSpeedY, double waveSpeedZ, double falloff, int highpoint, double rollIntensity, double rollWave, double damageMultiplier, double thrustMultiplier, int damage);

	void Serialize(FSerializer &arc);
	void Tick ();
	TObjPtr<AActor*> m_Spot;
	double m_TremorRadius, m_DamageRadius;
	int m_Countdown;
	int m_CountdownStart;
	FSoundID m_QuakeSFX;
	int m_Flags;
	DVector3 m_Intensity;
	DVector3 m_WaveSpeed;
	double m_Falloff;
	int m_Highpoint, m_MiniCount;
	double m_RollIntensity, m_RollWave;
	double m_DamageMultiplier, m_ThrustMultiplier;
	int m_Damage;

	double GetModIntensity(double intensity, bool fake = false) const;
	double GetModWave(double ticFrac, double waveMultiplier) const;
	double GetFalloff(double dist, double radius) const;
	void DoQuakeDamage(DEarthquake *quake, AActor *victim, bool falloff) const;

	static int StaticGetQuakeIntensities(double ticFrac, AActor *viewer, FQuakeJiggers &jiggers);
};

#endif //__A_SHAREDGLOBAL_H__
