/*
** TRS.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2022 Andrew Clarke
** Copyright 2022-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#pragma once
#include "vectors.h"
#include "quaternion.h"

class TRS
{
public:
	FVector3 translation = FVector3(0,0,0);
	FQuaternion rotation = FQuaternion::Identity();
	FVector3 scaling = FVector3(1,1,1);

	bool operator==(const TRS& other) const
	{
		return other.translation == translation && other.rotation == rotation && other.scaling == scaling;
	}

	template<typename T>
	TRS& operator=(const T& other)
	{ // templated because IQMJoint is defined in model_iqm.h
		translation = other.Translate;
		rotation = other.Quaternion;
		scaling = other.Scale;
		return *this;
	}
};

