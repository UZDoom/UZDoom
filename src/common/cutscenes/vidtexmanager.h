#pragma once

#include "movieplayer.h"
#include "tarray.h"
#include "i_time.h"

enum EVideoState
{
	VIDSTATE_Starting,
	VIDSTATE_Running,
	VIDSTATE_Stopped
};

class FVideoTexture
{
public:
	std::unique_ptr<MoviePlayer> player;
	FTextureID target_tex;

	int videostate;
	uint64_t startTicksMs;

	int id;
};

class FVideoManager
{
	TArray<FVideoTexture> videos;
	int internalid;

public:
	int AddVideo(const FString& filename, FTextureID target, TArray<int>& ans, int firstframetime, int lastframetime, int frametime, int flags);
	void StopVideo(int vid);
	void DeleteAll();
	
	void TickAll();

	int GetVideoStatus(int vid)
	{
		if (vid == -1)
			return VIDSTATE_Stopped;

		auto it = std::find_if(videos.begin(), videos.end(), [vid](const FVideoTexture& target) { return target.id == vid; });

		if (it == videos.end())
			return VIDSTATE_Stopped;

		return (*it).videostate;
	}

	int GetVideoTime(int vid)
	{
		if (vid == -1)
			return 0;

		auto it = std::find_if(videos.begin(), videos.end(), [vid](const FVideoTexture& target) { return target.id == vid; });

		if (it == videos.end())
			return 0;

		return I_msTime() - (*it).startTicksMs;
	}
};

extern FVideoManager VidMan;
