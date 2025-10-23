#include <stdint.h>

#include "animtexture.h"
#include "texturemanager.h"

class MoviePlayer
{
protected:
	int flags;
public:
	// public because it's needed for the video manager.
	enum EMovieFlags
	{
		NOSOUNDCUTOFF = 1,
		FIXEDVIEWPORT = 2,	// Forces fixed 640x480 screen size like for Blood's intros.
		NOMUSICCUTOFF = 4,
	};

	virtual void Start() {}
	virtual bool Frame(uint64_t clock) = 0;
	virtual void Stop() {}
	virtual ~MoviePlayer() { Stop(); };
	virtual FTextureID GetTexture() = 0;
	virtual void SetTarget(FTextureID first) = 0;
};

MoviePlayer* OpenMovie(const char* filename, TArray<int>& ans, const int* frameticks, int flags, FString& error);
