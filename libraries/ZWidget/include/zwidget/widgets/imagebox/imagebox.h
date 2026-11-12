
#pragma once

#include "../../core/widget.h"
#include "../../core/image.h"

enum ImageBoxScale
{
	None     = 0,
	Contain  = 0b0'000'000'01,
	Cover    = 0b0'000'000'10,

	StretchX = 0b0'000'100'00,
	GrowX    = 0b0'000'010'00,
	ShrinkX  = 0b0'000'001'00,

	StretchY = 0b0'100'000'00,
	GrowY    = 0b0'010'000'00,
	ShrinkY  = 0b0'001'000'00,

	Stretch = StretchX|StretchY,
	Grow    = GrowX|GrowY,
	Shrink  = ShrinkX|ShrinkY,
};

enum ImageBoxAnchor
{
	Center = 0,
	North  = 0b00010,
	South  = 0b00011,
	East   = 0b01000,
	West   = 0b01100,
	NorthEast = North|East,
	NorthWest = North|West,
	SouthEast = South|East,
	SouthWest = South|West,
};

class ImageBox : public Widget
{
public:
	ImageBox(Widget* parent);

	void SetImage(std::shared_ptr<Image> newImage);
	void SetImageScale(ImageBoxScale scale);
	void SetImageAnchor(ImageBoxAnchor anchor);

	double GetPreferredWidth() override;
	double GetPreferredHeight() override;

protected:
	void OnPaint(Canvas* canvas) override;

private:
	std::shared_ptr<Image> image;
	ImageBoxScale scale = ImageBoxScale::None;
	ImageBoxAnchor anchor = ImageBoxAnchor::Center;
};
