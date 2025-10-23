#pragma once

#include "textures.h"


class AnimTexture : public FTexture
{
	TArray<uint8_t> Image;
	int pixelformat;
	int active = 0;
public:
	enum
	{
		Paletted = 0,
		RGB = 1,
		YUV = 2,
		VPX = 3
	};
	void SetFrameSize(int format, int width, int height);
	// Changed to at least initialize the texture size to sane values.
	AnimTexture() { SetFrameSize(RGB, 128, 128); };
	void SetFrame(const uint8_t* palette, const void* data);
	virtual FBitmap GetBgraBitmap(const PalEntry* remap, int* trans) override;
	void ClearFrame();

	bool IsAnimTex() override { return true; }
};

class AnimTextures
{
	int active;
	FGameTexture* tex = nullptr;

public:
	AnimTextures();
	~AnimTextures();
	void Clean(bool clearframe = false);
	void ClearFrame();
	void SetSize(int format, int width, int height);
	void SetFrame(const uint8_t* palette, const void* data);
	void SetTarget(FTextureID first);
	FGameTexture* GetFrame()
	{
		return tex;
	}

	FTextureID GetFrameID()
	{
		return tex->GetID();
	}
};
