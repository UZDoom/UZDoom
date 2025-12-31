/*
** joystickmenu.cpp
**
** The joystick configuration menus
**
**---------------------------------------------------------------------------
**
** Copyright 2009-2016 Marisa Heit
** Copyright 2010-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
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

#include "m_joy.h"
#include "menu.h"
#include "vm.h"

static TArray<IJoystickConfig *> Joysticks;

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetSensitivity)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_FLOAT(self->GetSensitivity());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetSensitivity)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_FLOAT(sens);
	self->SetSensitivity((float)sens);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, HasHaptics)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_BOOL(self->HasHaptics());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetHapticsStrength)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_FLOAT(self->GetHapticsStrength());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetHapticsStrength)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_FLOAT(strength);
	self->SetHapticsStrength((float)strength);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisScale)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	ACTION_RETURN_FLOAT(self->GetAxisScale(axis));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetAxisScale)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_FLOAT(sens);
	self->SetAxisScale(axis, (float)sens);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisDeadZone)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	ACTION_RETURN_FLOAT(self->GetAxisDeadZone(axis));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetAxisDeadZone)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_FLOAT(dz);
	self->SetAxisDeadZone(axis, (float)dz);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisDigitalThreshold)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	ACTION_RETURN_FLOAT(self->GetAxisDigitalThreshold(axis));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetAxisDigitalThreshold)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_FLOAT(dt);
	self->SetAxisDigitalThreshold(axis, (float)dt);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisResponseCurve)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	ACTION_RETURN_INT(self->GetAxisResponseCurve(axis));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetAxisResponseCurve)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_INT(curve);
	self->SetAxisResponseCurve(axis, (EJoyCurve)curve);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisResponseCurvePoint)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_INT(point);
	ACTION_RETURN_FLOAT(self->GetAxisResponseCurvePoint(axis, point));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetAxisResponseCurvePoint)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	PARAM_INT(point);
	PARAM_FLOAT(value);
	self->SetAxisResponseCurvePoint(axis, point, value);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetName)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_STRING(self->GetName());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetAxisName)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_INT(axis);
	ACTION_RETURN_STRING(self->GetAxisName(axis));
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetNumAxes)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_INT(self->GetNumAxes());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetEnabled)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_BOOL(self->GetEnabled());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetEnabled)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_BOOL(enabled);
	self->SetEnabled(enabled);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, AllowsEnabledInBackground)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_BOOL(self->AllowsEnabledInBackground());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, GetEnabledInBackground)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	ACTION_RETURN_BOOL(self->GetEnabledInBackground());
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, SetEnabledInBackground)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	PARAM_BOOL(enabled);
	self->SetEnabledInBackground(enabled);
	return 0;
}

DEFINE_ACTION_FUNCTION(IJoystickConfig, Reset)
{
	PARAM_SELF_STRUCT_PROLOGUE(IJoystickConfig);
	self->Reset();
	return 0;
}

void UpdateJoystickMenu(IJoystickConfig *selected)
{
	DMenuDescriptor **desc = MenuDescriptors.CheckKey(NAME_JoystickOptions);
	DMenuDescriptor **ddesc = MenuDescriptors.CheckKey("JoystickOptionsDefaults");
	if (ddesc == nullptr) return;	// without any data the menu cannot be set up and must remain empty.
	if (desc != NULL && (*desc)->IsKindOf(RUNTIME_CLASS(DOptionMenuDescriptor)))
	{
		DOptionMenuDescriptor *opt = (DOptionMenuDescriptor *)*desc;
		DOptionMenuDescriptor *dopt = (DOptionMenuDescriptor *)*ddesc;
		if (dopt == nullptr) return;
		DMenuItemBase *it;

		int i;
		int itemnum = -1;

		I_GetJoysticks(Joysticks);
		if ((unsigned)itemnum >= Joysticks.Size())
		{
			itemnum = Joysticks.Size() - 1;
		}
		if (selected != NULL)
		{
			for (i = 0; (unsigned)i < Joysticks.Size(); ++i)
			{
				if (Joysticks[i] == selected)
				{
					itemnum = i;
					break;
				}
			}
		}
		opt->mItems = dopt->mItems;

		it = opt->GetItem("ConfigureMessage");
		if (it != nullptr) it->SetValue(0, !!Joysticks.Size());
		it = opt->GetItem("ConnectMessage1");
		if (it != nullptr) it->SetValue(0, !use_joystick);
		it = opt->GetItem("ConnectMessage2");
		if (it != nullptr) it->SetValue(0, !use_joystick);

		for (int ii = 0; ii < (int)Joysticks.Size(); ++ii)
		{
			it = CreateOptionMenuItemJoyConfigMenu(Joysticks[ii]->GetName().GetChars(), Joysticks[ii]);
			GC::WriteBarrier(opt, it);
			opt->mItems.Push(it);
			if (ii == itemnum) opt->mSelectedItem = opt->mItems.Size();
		}
		if (opt->mSelectedItem >= (int)opt->mItems.Size())
		{
			opt->mSelectedItem = opt->mItems.Size() - 1;
		}
		//opt->CalcIndent();

		// If the joystick config menu is open, close it if the device it's open for is gone.
		if (CurrentMenu != nullptr && (CurrentMenu->IsKindOf("JoystickConfigMenu")))
		{
			auto p = CurrentMenu->PointerVar<IJoystickConfig>("mJoy");
			if (p != nullptr)
			{
				unsigned ii;
				for (ii = 0; ii < Joysticks.Size(); ++ii)
				{
					if (Joysticks[ii] == p)
					{
						break;
					}
				}
				if (ii == Joysticks.Size())
				{
					CurrentMenu->Close();
				}
			}
		}
	}
}
