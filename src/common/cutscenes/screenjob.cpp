/*
** screenjob.cpp
**
** Generic cutscene display
**
**---------------------------------------------------------------------------
**
** Copyright 2020 Christoph Oelckers
** Copyright 2020-2025 GZDoom Maintainers and Contributors
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

#include "types.h"
#include "screenjob.h"
#include "i_time.h"
#include "v_2ddrawer.h"
#include "animlib.h"
#include "v_draw.h"
#include "s_soundinternal.h"
#include "animtexture.h"
#include "gamestate.h"
#include "vm.h"
#include "c_bind.h"
#include "c_console.h"
#include "gamestate.h"
#include "printf.h"
#include "c_dispatch.h"
#include "s_music.h"
#include "m_argv.h"
#include "i_interface.h"

CVAR(Bool, inter_subtitles, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG);

CutsceneState cutscene;
static int ticks;

//=============================================================================
//
//
//
//=============================================================================

void Job_Init()
{
	static bool done = false;
	if (!done)
	{
		done = true;
		GC::AddMarkerFunc([] { GC::Mark(cutscene.runner); });
	}
	cutscene.runnerclass = PClass::FindClass("ScreenJobRunner");
	if (!cutscene.runnerclass) I_FatalError("ScreenJobRunner not defined");
	cutscene.runnerclasstype = NewPointer(cutscene.runnerclass);
}

//=============================================================================
//
//
//
//=============================================================================

VMFunction* LookupFunction(const char* qname, bool validate)
{
	size_t p = strcspn(qname, ".");
	if (p == 0) 
		I_Error("Call to undefined function %s", qname);
	FName clsname(qname, p, true);
	FName funcname(qname + p + 1, true);

	auto func = PClass::FindFunction(clsname, funcname);
	if (func == nullptr) 
		I_Error("Call to undefined function %s", qname);
	if (validate)
	{
		// these conditions must be met by all functions for this interface.
		if (func->Proto->ReturnTypes.Size() != 0) I_Error("Bad cutscene function %s. Return value not allowed", qname);
		if (func->ImplicitArgs != 0) I_Error("Bad cutscene function %s. Must be static", qname);
	}
	return func;
}

//=============================================================================
//
//
//
//=============================================================================

void CallCreateFunction(const char* qname, DObject* runner)
{
	auto func = LookupFunction(qname);
	if (func->Proto->ArgumentTypes.Size() != 1) I_Error("Bad cutscene function %s. Must receive precisely one argument.", qname);
	if (func->Proto->ArgumentTypes[0] != cutscene.runnerclasstype) I_Error("Bad cutscene function %s. Must receive ScreenJobRunner reference.", qname);
	VMValue val = runner;
	VMCall(func, &val, 1, nullptr, 0);
}

//=============================================================================
//
//
//
//=============================================================================

DObject* CreateRunner(bool clearbefore, int skipType)
{
	auto obj = cutscene.runnerclass->CreateNew();
	auto func = LookupFunction("ScreenJobRunner.Init", false);
	VMValue val[4] = { obj, clearbefore, false, skipType };
	VMCall(func, val, 4, nullptr, 0);
	return obj;
}

//=============================================================================
//
//
//
//=============================================================================

void AddGenericVideo(DObject* runner, const FString& fn, int soundid, int fps)
{
	auto func = LookupFunction("ScreenJobRunner.AddGenericVideo", false);
	VMValue val[] = { runner, &fn, soundid, fps };
	VMCall(func, val, 4, nullptr, 0);
}

//=============================================================================
//
//
//
//=============================================================================

int CutsceneDef::GetSound()
{
	FSoundID id = INVALID_SOUND;
	if (soundName.IsNotEmpty()) id = soundEngine->FindSound(soundName.GetChars());
	if (id == INVALID_SOUND) id = soundEngine->FindSoundByResID(soundID);
	return id.index();
}

void CutsceneDef::Create(DObject* runner)
{
	if (function.IsNotEmpty())
	{
		CallCreateFunction(function.GetChars(), runner);
	}
	else if (video.IsNotEmpty())
	{
		AddGenericVideo(runner, video, GetSound(), framespersec);
	}
}

//=============================================================================
//
//
//
//=============================================================================

void DeleteScreenJob()
{
	if (cutscene.runner) cutscene.runner->Destroy();
	cutscene.runner = nullptr;
}

void EndScreenJob()
{
	DeleteScreenJob();
	if (cutscene.completion) cutscene.completion(false);
	cutscene.completion = nullptr;
}


//=============================================================================
//
//
//
//=============================================================================

bool ScreenJobResponder(event_t* ev)
{
	if (ev->type == EV_KeyDown)
	{
		// We never reach the key binding checks in G_Responder, so for the console we have to check for ourselves here.
		auto binding = Bindings.GetBinding(ev->data1);
		if (binding.CompareNoCase("toggleconsole") == 0)
		{
			C_ToggleConsole();
			return true;
		}
		if (binding.CompareNoCase("screenshot") == 0)
		{
			C_DoCommand("screenshot");
			return true;
		}
	}
	FInputEvent evt = ev;
	if (cutscene.runner)
	{
		ScaleOverrider ovr(twod);
		IFVIRTUALPTRNAME(cutscene.runner, NAME_ScreenJobRunner, OnEvent)
		{
			int result = 0;
			VMValue parm[] = { cutscene.runner, &evt };
			VMReturn ret(&result);
			VMCall(func, parm, 2, &ret, 1);
			return result;
		}
	}
	return false;
}

//=============================================================================
//
//
//
//=============================================================================

bool ScreenJobTick()
{
	ticks++;
	if (cutscene.runner)
	{
		ScaleOverrider ovr(twod);
		IFVIRTUALPTRNAME(cutscene.runner, NAME_ScreenJobRunner, OnTick)
		{
			int result = 0;
			VMValue parm[] = { cutscene.runner };
			VMReturn ret(&result);
			VMCall(func, parm, 1, &ret, 1);
			return result;
		}
	}
	return false;
}

//=============================================================================
//
//
//
//=============================================================================

void ScreenJobDraw()
{
	double smoothratio = I_GetTimeFrac();

	if (cutscene.runner)
	{
		twod->ClearScreen();
		ScaleOverrider ovr(twod);
		IFVIRTUALPTRNAME(cutscene.runner, NAME_ScreenJobRunner, RunFrame)
		{
			int ret = 0;
			VMValue parm[] = { cutscene.runner, smoothratio };
			VMReturn rets[] = { &ret };
			VMCall(func, parm, 2, rets, 1);
		}
	}
}

//=============================================================================
//
//
//
//=============================================================================

bool ScreenJobValidate()
{
	if (cutscene.runner)
	{
		ScaleOverrider ovr(twod);
		IFVIRTUALPTRNAME(cutscene.runner, NAME_ScreenJobRunner, Validate)
		{
			int res;
			VMValue parm[] = { cutscene.runner };
			VMReturn ret(&res);
			VMCall(func, parm, 1, &ret, 1);
			I_ResetFrameTime();
			return res;
		}
	}
	return false;
}

//=============================================================================
//
//
//
//=============================================================================

bool StartCutscene(CutsceneDef& cs, int flags, const CompletionFunc& completion_)
{
	if ((cs.function.IsNotEmpty() || cs.video.IsNotEmpty()) && cs.function.CompareNoCase("none") != 0)
	{
		cutscene.completion = completion_;
		cutscene.runner = CreateRunner();
		GC::WriteBarrier(cutscene.runner);
		try
		{
			cs.Create(cutscene.runner);
			if (!ScreenJobValidate())
			{
				DeleteScreenJob();
				return false;
			}
			if (sysCallbacks.StartCutscene) sysCallbacks.StartCutscene(flags & SJ_BLOCKUI);
		}
		catch (...)
		{
			DeleteScreenJob();
			throw;
		}
		return true;
	}
	return false;
}

bool StartCutscene(const char* s, int flags, const CompletionFunc& completion)
{
	CutsceneDef def;
	def.function = s;
	return StartCutscene(def, flags, completion);
}

//=============================================================================
//
// initiates a screen wipe. Needs to call the game code for it.
//
//=============================================================================

DEFINE_ACTION_FUNCTION(DScreenJobRunner, setTransition)
{
	PARAM_PROLOGUE;
	PARAM_INT(type);
	
	if (type && sysCallbacks.SetTransition) sysCallbacks.SetTransition(type);
	return 0;
}

//=============================================================================
//
// to block wipes on cutscenes that cannot handle it
//
//=============================================================================

bool CanWipe()
{
	if (cutscene.runner == nullptr) return true;
	IFVM(ScreenJobRunner, CanWipe)
	{
		int can;
		VMReturn ret(&can);
		VMValue param = cutscene.runner;
		VMCall(func, &param, 1, &ret, 1);
		return can;
	}
	return true;
}

//=============================================================================
//
//
//
//=============================================================================

CCMD(testcutscene)
{
	if (argv.argc() < 2)
	{
		Printf("Usage: testcutscene <buildfunction>\n");
		return;
	}
	try
	{
		if (StartCutscene(argv[1], 0, [](bool) {}))
		{
			C_HideConsole();
		}
	}
	catch (const CRecoverableError& err)
	{
		Printf(TEXTCOLOR_RED "Unable to play cutscene: %s\n", err.what());
	}
}



