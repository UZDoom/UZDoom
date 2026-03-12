#pragma once

#include <memory>
#include <string>
#include <vector>
#include "rect.h"

enum class ImageFormat
{
	R8G8B8A8,
	B8G8R8A8
};

class Image
{
public:
	virtual ~Image() = default;

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual ImageFormat GetFormat() const = 0;
	virtual void* GetData() const = 0;

	static std::shared_ptr<Image> Create(int width, int height, ImageFormat format, const void* data);
	static std::shared_ptr<Image> LoadResource(const std::string& resourcename, double dpiscale = 1.0);
};

class CustomCursorFrame
{
public:
	CustomCursorFrame() = default;
	CustomCursorFrame(std::shared_ptr<Image> image, double frameDuration = 1.0) : FrameImage(std::move(image)), FrameDuration(frameDuration) { }

	std::shared_ptr<Image> FrameImage;
	double FrameDuration = 1.0; // the duration this frame is displayed, in seconds
};

class CustomCursor
{
public:
	virtual ~CustomCursor() = default;

	virtual const std::vector<CustomCursorFrame>& GetFrames() const = 0;
	virtual Point GetHotspot() const = 0;

	static std::shared_ptr<CustomCursor> Create(std::vector<CustomCursorFrame> frames, const Point& hotspot);
};
