#include "vidtexmanager.h"
#include "printf.h"
#include "i_time.h"
#include "animtexture.h"
#include "s_music.h"
#include "vm.h"

FVideoManager VidMan;

void FVideoManager::DeleteAll()
{
	for (int i = 0; i < videos.size(); i++)
	{
		auto& curvid = videos[i];
		if (curvid.videostate != VIDSTATE_Stopped)
		{
			curvid.player->Stop();
			static_cast<AnimTexture*>(TexMan.GetGameTexture(curvid.target_tex)->GetTexture())->ClearFrame();
			TexMan.GetGameTexture(curvid.target_tex)->CleanHardwareData();
		}
	}
	videos.Clear();
}

void FVideoManager::StopVideo(int vid)
{
	for (int i = 0; i < videos.size(); i++)
	{
		auto& curvid = videos[i];
		if (curvid.id == vid)
		{
			curvid.player->Stop();
			videos.Delete(i);
			return;
		}
	}
}

void FVideoManager::TickAll()
{
	auto curMs = I_msTime();
	for (int i = 0; i < videos.size(); i++)
	{
		auto& curvid = videos[i];
		if (curvid.videostate == VIDSTATE_Starting)
		{
			curvid.startTicksMs = curMs;
			curvid.player->Start();
			curvid.player->Frame(0);
			curvid.videostate = VIDSTATE_Running;
		}
		else if (curvid.videostate == VIDSTATE_Running)
		{
			if (!curvid.player->Frame((curMs - curvid.startTicksMs) * 1000000))
			{
				curvid.player->Stop();
				videos.Delete(i);
			}
		}
	}
}

int FVideoManager::AddVideo(const FString& filename, FTextureID target, TArray<int>& ans, int firstframetime, int lastframetime, int frametime, int flags)
{
	FString error;
	if (!target.Exists())
	{
		return -1;
	}
	videos.Resize(videos.Size() + 1);
	auto& lastVid = videos[videos.Size() - 1];

	lastVid.videostate = VIDSTATE_Stopped;
	if (firstframetime == -1) firstframetime = frametime;
	if (lastframetime == -1) lastframetime = frametime;
	int frametimes[] = { firstframetime, frametime, lastframetime };
	lastVid.player.reset(OpenMovie(filename.GetChars(), ans, frametime == -1 ? nullptr : frametimes, flags, error));
	if (!lastVid.player)
	{
		Printf(TEXTCOLOR_YELLOW "%s", error.GetChars());
		videos.Pop();
		return -1;
	}
	lastVid.player->SetTarget(target);
	lastVid.startTicksMs = I_msTime();
	lastVid.videostate = VIDSTATE_Starting;
	internalid++;
	lastVid.id = internalid;
	if (!(flags & MoviePlayer::NOMUSICCUTOFF))
	{
		S_StopMusic(true);
	}
	return internalid;
}

DEFINE_ACTION_FUNCTION(_VidMan, AddVideo)
{
	PARAM_PROLOGUE;
	PARAM_STRING(filename);
	PARAM_INT(target);
	PARAM_POINTER(sndinf, TArray<int>);
	PARAM_INT(frametime);
	PARAM_INT(firstframetime);
	PARAM_INT(lastframetime);
	PARAM_INT(flags);
	ACTION_RETURN_INT(VidMan.AddVideo(filename, FSetTextureID(target), *sndinf, firstframetime, lastframetime, frametime, flags));
}

DEFINE_ACTION_FUNCTION(_VidMan, GetVideoTime)
{
	PARAM_PROLOGUE;
	PARAM_INT(vid);
	ACTION_RETURN_INT(VidMan.GetVideoTime(vid));
}

DEFINE_ACTION_FUNCTION(_VidMan, GetVideoStatus)
{
	PARAM_PROLOGUE;
	PARAM_INT(vid);
	ACTION_RETURN_INT(VidMan.GetVideoStatus(vid));
}

DEFINE_ACTION_FUNCTION(_VidMan, StopVideo)
{
	PARAM_PROLOGUE;
	PARAM_INT(vid);

	VidMan.StopVideo(vid);
	return 0;
}
