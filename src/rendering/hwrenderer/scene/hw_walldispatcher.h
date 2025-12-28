/*
** hw_walldispatcher.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2023 Christoph Oelckers
** Copyright 2023-2025 GZDoom Maintainers and Contributors
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

#pragma once

struct HWMissing
{
	side_t* side;
	subsector_t* sub;
	double plane;
};

struct HWMeshHelper
{
	TArray<HWWall> list;
	TArray<HWWall> translucent;
	TArray<HWWall> portals;
	TArray<HWMissing> lower;
	TArray<HWMissing> upper;
};


struct HWWallDispatcher
{
	FLevelLocals* Level;
	HWDrawInfo* di;
	HWMeshHelper* mh;
	ELightMode lightmode;

	HWWallDispatcher(HWDrawInfo* info)
	{
		Level = info->Level;
		di = info;
		mh = nullptr;
		lightmode = info->lightmode;
	}

	HWWallDispatcher(FLevelLocals* lev, HWMeshHelper* help, ELightMode lm)
	{
		Level = lev;
		di = nullptr;
		mh = help;
		lightmode = lm;
	}

	void AddUpperMissingTexture(side_t* side, subsector_t* sub, float height)
	{
		if (di) di->AddUpperMissingTexture(side, sub, height);
		else
		{
			mh->upper.Reserve(1);
			mh->upper.Last() = { side, sub, height };
		}
	}
	void AddLowerMissingTexture(side_t* side, subsector_t* sub, float height)
	{
		if (di) di->AddLowerMissingTexture(side, sub, height);
		else
		{
			mh->lower.Reserve(1);
			mh->lower.Last() = { side, sub, height };
		}
	}

	bool isFullbrightScene()
	{
		// The mesh builder cannot know this and must treat everything as not fullbright.
		return di && di->isFullbrightScene();
	}

	void AddWall(HWWall* wal)
	{
		if (di) di->AddWall(wal);
		else if (!(wal->flags & HWWall::HWF_TRANSLUCENT)) mh->list.Push(*wal);
		else mh->translucent.Push(*wal);
	}

	void AddPortal(HWWall* wal)
	{
		mh->portals.Push(*wal);
	}

};
