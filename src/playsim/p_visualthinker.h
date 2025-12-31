/*
** p_visualthinker.h
**
** VisualThinkers
**
**---------------------------------------------------------------------------
**
** Copyright 2023-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** by Major Cooke
** Credit to phantombeta, Jay & RaveYard for aid
*/

#pragma once

#include "palettecontainer.h"
#include "hwrenderer/scene/hw_drawstructs.h"

//===========================================================================
//
//
// 
//===========================================================================

enum EVisualThinkerFlags
{
	VTF_FlipOffsetX		= 1 << 0,
	VTF_FlipOffsetY		= 1 << 1,
	VTF_FlipX			= 1 << 2,
	VTF_FlipY			= 1 << 3, // flip the sprite on the x/y axis.
	VTF_DontInterpolate	= 1 << 4, // disable all interpolation
	VTF_AddLightLevel	= 1 << 5, // adds sector light level to 'LightLevel'

	VTF_ParticleDefault = 0x40, 
	VTF_ParticleSquare = 0x80, 
	VTF_ParticleRound = 0xC0, 
	VTF_ParticleSmooth = 0x100,
	VTF_IsParticle = 0x1C0,			// Renders as a particle instead
};

class DVisualThinker : public DThinker
{
	DECLARE_CLASS(DVisualThinker, DThinker);
	void UpdateSector(subsector_t * newSubsector);

	DVisualThinker* _next, * _prev;
public:
	static const int DEFAULT_STAT = STAT_VISUALTHINKER;

	DVector3		Prev;
	DVector2		Scale,
					Offset;
	float			PrevRoll;
	int16_t			LightLevel;
	FTranslationID	Translation;
	FTextureID		AnimatedTexture;
	sector_t		*cursector;

	int flags;

	// internal only variables
	particle_t		PT;

	void Construct();
	void OnDestroy() override;
	DVisualThinker* GetNext() const;

	static DVisualThinker* NewVisualThinker(FLevelLocals* Level, PClass* type, bool clientSide);
	void SetTranslation(FName trname);
	int GetRenderStyle() const;
	bool isFrozen();
	bool ValidTexture();
	int GetParticleType() const;
	int GetLightLevel(sector_t *rendersector) const;
	FVector3 InterpolatedPosition(double ticFrac) const;
	float InterpolatedRoll(double ticFrac) const;

	void Tick() override;
	void UpdateSpriteInfo();
	void UpdateSector();
	void Serialize(FSerializer& arc) override;

	float GetOffset(bool y) const;
};
