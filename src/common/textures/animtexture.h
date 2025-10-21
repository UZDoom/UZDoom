#pragma once

#include "textures.h"


class AnimTexture : public FTexture
{
	TArray<uint8_t> Image;
	int pixelformat;
public:
	enum
	{
		Paletted = 0,
		RGB = 1,
		YUV = 2,
		VPX = 3
	};
	AnimTexture() = default;
	void SetFrameSize(int format, int width, int height);
	void SetFrame(const uint8_t* palette, const void* data);
	virtual FBitmap GetBgraBitmap(const PalEntry* remap, int* trans) override;

	bool IsAnimTex() override { return true; }
};

class AnimTextures
{
	int active;
	FGameTexture* tex[2] = { nullptr, nullptr };

public:
	AnimTextures();
	~AnimTextures();
	void Clean();
	void SetSize(int format, int width, int height);
	void SetFrame(const uint8_t* palette, const void* data);
	void SetTargets(FTextureID first, FTextureID second);
	FGameTexture* GetFrame()
	{
		return tex[active];
	}

	FTextureID GetFrameID()
	{
		return tex[active]->GetID();
	}
};
