#include <bit>

#include "widgets/imagebox/imagebox.h"

ImageBox::ImageBox(Widget* parent) : Widget(parent)
{
}

double ImageBox::GetPreferredWidth()
{
	if (image)
		return (double)image->GetWidth();
	else
		return 0.0;
}

double ImageBox::GetPreferredHeight()
{
	if (image)
		return (double)image->GetHeight();
	else
		return 0.0;
}

void ImageBox::SetImage(std::shared_ptr<Image> newImage)
{
	if (image != newImage)
	{
		image = newImage;
		Update();
	}
}

void ImageBox::SetImageScale(ImageBoxScale newScale)
{
	if (scale != newScale)
	{
		scale = newScale;
		Update();
	}
}

void ImageBox::SetImageAnchor(ImageBoxAnchor newAnchor)
{
	if (anchor != newAnchor)
	{
		anchor = newAnchor;
		Update();
	}
}

template <typename T>
concept Bits = std::unsigned_integral<T> || std::is_enum_v<T>;

template <Bits T, typename U = T>
[[nodiscard]] inline constexpr U onebit(T n)
{
	return static_cast<U>(std::has_single_bit(n) ? n: static_cast<T>(0));
}

template <Bits T, typename U = T, typename V = T>
[[nodiscard]] inline constexpr U only(T a, V b)
{
	return static_cast<U>(!(a&~b)? a: static_cast<T>(0));
}

template <Bits T, typename U = T, typename V = T>
[[nodiscard]] inline constexpr U all(T a, V b)
{
	return static_cast<U>((a&b)==b? a: static_cast<T>(0));
}

void ImageBox::OnPaint(Canvas* canvas)
{
	if (!image) return;

	double bw = GetWidth(), bh = GetHeight();
	double iw = image->GetWidth(), ih = image->GetHeight();
	double rw = bw / iw, rh = bh / ih;
	double xscale = 1, yscale = 1;

	if (only(scale, ImageBoxScale::Contain|ImageBoxScale::Cover|ImageBoxScale::Grow|ImageBoxScale::Shrink)
		&& (only(scale, ImageBoxScale::Contain|ImageBoxScale::Cover)
			|| all(scale, ImageBoxScale::Grow)
			|| all(scale, ImageBoxScale::Shrink)
		))
	{
		auto rc = onebit<unsigned, ImageBoxScale>(scale & (ImageBoxScale::Cover | ImageBoxScale::Contain));
		double rm = rc == ImageBoxScale::Cover? std::max(rw, rh): std::min(rw, rh);
		xscale = yscale = (scale & ImageBoxScale::Grow)
			? std::max(1., rm)
			: (scale & ImageBoxScale::Shrink)
				? std::min(1., rm)
				: rm;
	}
	else
	{
		switch (onebit<unsigned, ImageBoxScale>(scale & (ImageBoxScale::StretchX | ImageBoxScale::GrowX | ImageBoxScale::ShrinkX)))
		{
			default: break;
			case ImageBoxScale::StretchX: xscale = rw;            break;
			case ImageBoxScale::GrowX:    xscale = rw > 1? rw: 1; break;
			case ImageBoxScale::ShrinkX:  xscale = rw < 1? rw: 1; break;
		}
		switch (onebit<unsigned, ImageBoxScale>(scale & (ImageBoxScale::StretchY | ImageBoxScale::GrowY | ImageBoxScale::ShrinkY)))
		{
			default: break;
			case ImageBoxScale::StretchY: yscale = rh;            break;
			case ImageBoxScale::GrowY:    yscale = rh > 1? rh: 1; break;
			case ImageBoxScale::ShrinkY:  yscale = rh < 1? rh: 1; break;
		}
	}
	iw *= xscale;
	ih *= yscale;

	constexpr auto Horizontal = ImageBoxAnchor::East & ImageBoxAnchor::West;
	constexpr auto Vertical = ImageBoxAnchor::North & ImageBoxAnchor::South;
	double x = (anchor & Horizontal)
		? (anchor & ~Horizontal & ImageBoxAnchor::West)
			? 0
			: bw-iw
		: (bw-iw)*0.5;
	double y = (anchor & Vertical)
		? (anchor & ~Vertical & ImageBoxAnchor::South)
			? bh-ih
			: 0
		: (bh-ih)*0.5;

	canvas->drawImage(image, Rect::xywh(x, y, iw, ih));
}
