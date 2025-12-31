/*
** m_joy.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2005-2016 Marisa Heit
** Copyright 2007-2016 Christoph Oelckers
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

#ifndef M_JOY_H
#define M_JOY_H

#include "c_cvars.h"
#include "keydef.h"
#include "m_argv.h"
#include "tarray.h"

union CubicBezier {
	struct {
		float x1;
		float y1;
		float x2;
		float y2;
	};
	float pts[4];
};

enum EJoyCurve {
	JOYCURVE_CUSTOM = -1,
	JOYCURVE_DEFAULT,
	JOYCURVE_LINEAR,
	JOYCURVE_QUADRATIC,
	JOYCURVE_CUBIC,

	NUM_JOYCURVE
};

extern const float JOYDEADZONE_DEFAULT;

extern const float JOYSENSITIVITY_DEFAULT;

extern const float JOYHAPSTRENGTH_DEFAULT;

extern const float JOYTHRESH_DEFAULT;

extern const float JOYTHRESH_TRIGGER;
extern const float JOYTHRESH_STICK_X;
extern const float JOYTHRESH_STICK_Y;

extern const CubicBezier JOYCURVE[NUM_JOYCURVE];

// Generic configuration interface for a controller.
struct IJoystickConfig
{
	virtual ~IJoystickConfig() = 0;

	virtual FString GetName() = 0;
	virtual float GetSensitivity() = 0;
	virtual void SetSensitivity(float scale) = 0;

	virtual bool HasHaptics() = 0;
	virtual float GetHapticsStrength() = 0;
	virtual void SetHapticsStrength(float strength) = 0;

	virtual int GetNumAxes() = 0;
	virtual float GetAxisDeadZone(int axis) = 0;
	virtual const char *GetAxisName(int axis) = 0;
	virtual float GetAxisScale(int axis) = 0;
	virtual float GetAxisDigitalThreshold(int axis) = 0;
	virtual EJoyCurve GetAxisResponseCurve(int axis) = 0;
	virtual float GetAxisResponseCurvePoint(int axis, int point) = 0;

	virtual void SetAxisDeadZone(int axis, float zone) = 0;
	virtual void SetAxisScale(int axis, float scale) = 0;
	virtual void SetAxisDigitalThreshold(int axis, float threshold) = 0;
	virtual void SetAxisResponseCurve(int axis, EJoyCurve preset) = 0;
	virtual void SetAxisResponseCurvePoint(int axis, int point, float value) = 0;

	virtual bool GetEnabled() = 0;
	virtual void SetEnabled(bool enabled) = 0;

	virtual bool AllowsEnabledInBackground() = 0;
	virtual bool GetEnabledInBackground() = 0;
	virtual void SetEnabledInBackground(bool enabled) = 0;

	// Used by the saver to not save properties that are at their defaults.
	virtual bool IsSensitivityDefault() = 0;
	virtual bool IsHapticsStrengthDefault() = 0;
	virtual bool IsAxisDeadZoneDefault(int axis) = 0;
	virtual bool IsAxisScaleDefault(int axis) = 0;
	virtual bool IsAxisDigitalThresholdDefault(int axis) = 0;
	virtual bool IsAxisResponseCurveDefault(int axis) = 0;

	virtual void SetDefaultConfig() = 0;
	virtual FString GetIdentifier() = 0;

	void Reset()
	{
		SetDefaultConfig();
		SetEnabled(true);
		SetEnabledInBackground(AllowsEnabledInBackground());
		SetSensitivity(1);
	}
};

EXTERN_CVAR(Bool, use_joystick);
EXTERN_FARG(nojoy);

bool M_LoadJoystickConfig(IJoystickConfig *joy);
void M_SaveJoystickConfig(IJoystickConfig *joy);

void Joy_GenerateButtonEvent(bool down, EKeyCodes which);
void Joy_GenerateButtonEvents(int oldbuttons, int newbuttons, int numbuttons, int base);
void Joy_GenerateButtonEvents(int oldbuttons, int newbuttons, int numbuttons, const int *keys);

double Joy_ApplyResponseCurveBezier(const CubicBezier &curve, double input);
double Joy_ManageSingleAxis(double axisval, double deadzone, double threshold, const CubicBezier &curve, uint8_t *buttons);
int Joy_XYAxesToButtons(double x, double y);
double Joy_ManageThumbstick(double *axis_x, double *axis_y, double deadzone_x, double deadzone_y,
	double threshold_x, double threshold_y, const CubicBezier &curve_x, const CubicBezier &curve_y, uint8_t *buttons);


// These ought to be provided by a system-specific i_input.cpp.
void I_GetAxes(float axes[NUM_AXIS_CODES]);
void I_GetJoysticks(TArray<IJoystickConfig *> &sticks);
IJoystickConfig *I_UpdateDeviceList();
extern void UpdateJoystickMenu(IJoystickConfig *);

#endif
