
#include "core/canvas.h"
#include "core/rect.h"
#include "core/colorf.h"
#include "core/utf8reader.h"
#include "core/resourcedata.h"
#include "core/image.h"
#include "core/truetypefont.h"
#include "core/pathfill.h"
#include "core/font_impl.h"
#include "window/window.h"
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstring>
#include <iostream>

#if defined(__SSE2__) || defined(_M_X64)
#include <immintrin.h>
#define USE_SSE2
#endif

////////////////////////////////////////////////////////////////////////////

class CanvasGlyph
{
public:
	struct
	{
		double leftSideBearing = 0.0;
		double yOffset = 0.0;
		double advanceWidth = 0.0;
	} metrics;

	double u = 0.0;
	double v = 0.0;
	double uvwidth = 0.0f;
	double uvheight = 0.0f;
	std::shared_ptr<CanvasTexture> texture;
};

class CanvasFont
{
public:
	CanvasFont(const std::string& fontname, double height, std::vector<uint8_t> data);
	~CanvasFont();

	CanvasGlyph* getGlyph(Canvas* canvas, uint32_t utfchar);

private:
	std::unique_ptr<TrueTypeFont> ttf;

	std::string fontname;
	double height = 0.0;

	TrueTypeTextMetrics textmetrics;
	std::unordered_map<uint32_t, std::unique_ptr<CanvasGlyph>> glyphs;

	friend class CanvasFontGroup;
};

class CanvasFontGroup
{
public:
	struct SingleFont
	{
		std::unique_ptr<CanvasFont> font;
		std::string language;
	};

	CanvasFontGroup(const std::string& fontname, double height);

	CanvasGlyph* getGlyph(Canvas* canvas, uint32_t utfchar, const char* lang = nullptr);
	TrueTypeTextMetrics& GetTextMetrics();

	double height;
	std::vector<SingleFont> fonts;
};

////////////////////////////////////////////////////////////////////////////

CanvasFont::CanvasFont(const std::string& fontname, double height, std::vector<uint8_t> data) : fontname(fontname), height(height)
{
	ttf = std::make_unique<TrueTypeFont>(TTFDataBuffer::create(std::move(data)));
	textmetrics = ttf->GetTextMetrics(height);
}

CanvasFont::~CanvasFont()
{
}

CanvasGlyph* CanvasFont::getGlyph(Canvas* canvas, uint32_t utfchar)
{
	uint32_t glyphIndex = ttf->GetGlyphIndex(utfchar);
	if (glyphIndex == 0) return nullptr;

	auto& glyph = glyphs[glyphIndex];
	if (glyph)
		return glyph.get();

	glyph = std::make_unique<CanvasGlyph>();

	TrueTypeGlyph ttfglyph = ttf->LoadGlyph(glyphIndex, height);

	// Create final subpixel version
	int w = ttfglyph.width;
	int h = ttfglyph.height;
	int destwidth = (w + 2) / 3;
	auto texture = std::make_shared<CanvasTexture>();
	std::vector<uint32_t> data(destwidth * h);

	uint8_t* grayscale = ttfglyph.grayscale.get();
	uint32_t* dest = data.data();
	for (int y = 0; y < h; y++)
	{
		uint8_t* sline = grayscale + y * w;
		uint32_t* dline = dest + y * destwidth;
		for (int x = 0; x < w; x += 3)
		{
			uint32_t values[5] =
			{
				x > 0 ? sline[x - 1] : 0U,
				sline[x],
				x + 1 < w ? sline[x + 1] : 0U,
				x + 2 < w ? sline[x + 2] : 0U,
				x + 3 < w ? sline[x + 3] : 0U
			};

			uint32_t red = (values[0] + values[1] + values[1] + values[2] + 2) >> 2;
			uint32_t green = (values[1] + values[2] + values[2] + values[3] + 2) >> 2;
			uint32_t blue = (values[2] + values[3] + values[3] + values[4] + 2) >> 2;
			uint32_t alpha = (red | green | blue) ? 255 : 0;

			*(dline++) = (alpha << 24) | (red << 16) | (green << 8) | blue;
		}
	}

	glyph->u = 0.0;
	glyph->v = 0.0;
	glyph->uvwidth = destwidth;
	glyph->uvheight = h;
	glyph->texture = canvas->createTexture(destwidth, h, data.data(), ImageFormat::B8G8R8A8);

	glyph->metrics.advanceWidth = (ttfglyph.advanceWidth + 2) / 3;
	glyph->metrics.leftSideBearing = (ttfglyph.leftSideBearing + 2) / 3;
	glyph->metrics.yOffset = ttfglyph.yOffset;

	return glyph.get();
}

////////////////////////////////////////////////////////////////////////////

CanvasFontGroup::CanvasFontGroup(const std::string& fontname, double height) : height(height)
{
	auto fontdata = ResourceData::LoadFont(fontname);
	fonts.resize(fontdata.size());
	for (size_t i = 0; i < fonts.size(); i++)
	{
		fonts[i].font = std::make_unique<CanvasFont>(fontname, height, fontdata[i].fontdata);
		fonts[i].language = fontdata[i].language;
	}
}

CanvasGlyph* CanvasFontGroup::getGlyph(Canvas* canvas, uint32_t utfchar, const char* lang)
{
	for (int i = 0; i < 2; i++)
	{
		for (auto& fd : fonts)
		{
			if (i == 1 || lang == nullptr || *lang == 0 || fd.language.empty() || fd.language == lang)
			{
				auto g = fd.font->getGlyph(canvas, utfchar);
				if (g) return g;
			}
		}
	}

	return nullptr;
}

TrueTypeTextMetrics& CanvasFontGroup::GetTextMetrics()
{
	return fonts[0].font->textmetrics;
}

////////////////////////////////////////////////////////////////////////////

Canvas::Canvas()
{
}

Canvas::~Canvas()
{
}

void Canvas::attach(DisplayWindow* newWindow)
{
	window = newWindow;
	uiscale = window ? window->GetDpiScale() : 1.0f;
	uint32_t white = 0xffffffff;
	whiteTexture = createTexture(1, 1, &white);
}

void Canvas::detach()
{
}

void Canvas::begin(const Colorf& color)
{
	if (window)
	{
		uiscale = window->GetDpiScale();
		width = window->GetPixelWidth();
		height = window->GetPixelHeight();
	}
	else
	{
		uiscale = 1.0f;
		width = 32;
		height = 32;
	}
}

Point Canvas::getOrigin()
{
	return origin;
}

void Canvas::setOrigin(const Point& newOrigin)
{
	origin = newOrigin;
}

void Canvas::pushClip(const Rect& box)
{
	if (!clipStack.empty())
	{
		const Rect& clip = clipStack.back();

		double x0 = box.x + origin.x;
		double y0 = box.y + origin.y;
		double x1 = x0 + box.width;
		double y1 = y0 + box.height;

		x0 = std::max(x0, clip.x);
		y0 = std::max(y0, clip.y);
		x1 = std::min(x1, clip.x + clip.width);
		y1 = std::min(y1, clip.y + clip.height);

		if (x0 < x1 && y0 < y1)
			clipStack.push_back(Rect::ltrb(x0, y0, x1, y1));
		else
			clipStack.push_back(Rect::xywh(0.0, 0.0, 0.0, 0.0));
	}
	else
	{
		clipStack.push_back(box);
	}
}

void Canvas::popClip()
{
	clipStack.pop_back();
}

void Canvas::fillRect(const Rect& box, const Colorf& color)
{
	fillTile(gridFit(origin.x + box.x), gridFit(origin.y + box.y), gridFit(box.width), gridFit(box.height), color);
}

void Canvas::drawImage(const std::shared_ptr<Image>& image, const Point& pos)
{
	auto& texture = imageTextures[image];
	if (!texture)
		texture = createTexture(image->GetWidth(), image->GetHeight(), image->GetData(), image->GetFormat());

	Colorf color(1.0f, 1.0f, 1.0f, 1.0f);
	drawTile(texture.get(), gridFit(origin.x + pos.x), gridFit(origin.y + pos.y), gridFit(texture->Width), gridFit(texture->Height), 0.0, 0.0, (float)texture->Width, (float)texture->Height, color);
}

void Canvas::drawImage(const std::shared_ptr<Image>& image, const Rect& box)
{
	auto& texture = imageTextures[image];
	if (!texture)
		texture = createTexture(image->GetWidth(), image->GetHeight(), image->GetData(), image->GetFormat());

	Colorf color(1.0f, 1.0f, 1.0f);
	drawTile(texture.get(), gridFit(origin.x + box.x), gridFit(origin.y + box.y), gridFit(box.width), gridFit(box.height), 0.0, 0.0, (float)texture->Width, (float)texture->Height, color);
}

void Canvas::drawImage(const std::shared_ptr<Image>& image, const Rect& src, const Rect& dest)
{
	auto& texture = imageTextures[image];
	if (!texture)
		texture = createTexture(image->GetWidth(), image->GetHeight(), image->GetData(), image->GetFormat());

	Colorf color(1.0f, 1.0f, 1.0f);
	drawTile(texture.get(), gridFit(origin.x + dest.x), gridFit(origin.y + dest.y), gridFit(dest.width), gridFit(dest.height), (float)src.x, (float)src.y, (float)src.width, (float)src.height, color);
}

void Canvas::line(const Point& p0, const Point& p1, const Colorf& color)
{
	double x0 = origin.x + p0.x;
	double y0 = origin.y + p0.y;
	double x1 = origin.x + p1.x;
	double y1 = origin.y + p1.y;

	if (clipStack.empty())// || (clipStack.back().contains({ x0, y0 }) && clipStack.back().contains({ x1, y1 })))
	{
		drawLineUnclipped({ x0, y0 }, { x1, y1 }, color);
	}
	else
	{
		const Rect& clip = clipStack.back();

		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		if (x1 < clip.x || x0 >= clip.x + clip.width)
			return;

		// Clip left edge
		if (x0 < clip.x)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dx) < 0.0001)
				return;
			y0 = y0 + (clip.x - x0) * dy / dx;
			x0 = clip.x;
		}

		// Clip right edge
		if (x1 > clip.x + clip.width)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dx) < 0.0001)
				return;
			y1 = y1 + (clip.x + clip.width - x1) * dy / dx;
			x1 = clip.x + clip.width;
		}

		if (y0 > y1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		if (y1 < clip.y || y0 >= clip.y + clip.height)
			return;

		// Clip top edge
		if (y0 < clip.y)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dy) < 0.0001)
				return;
			x0 = x0 + (clip.y - y0) * dx / dy;
			y0 = clip.y;
		}

		// Clip bottom edge
		if (y1 > clip.y + clip.height)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dy) < 0.0001)
				return;
			x1 = x1 + (clip.y + clip.height - y1) * dx / dy;
			y1 = clip.y + clip.height;
		}

		x0 = clamp(x0, clip.x, clip.x + clip.width);
		x1 = clamp(x1, clip.x, clip.x + clip.width);
		y0 = clamp(y0, clip.y, clip.y + clip.height);
		y1 = clamp(y1, clip.y, clip.y + clip.height);

		if (x0 != x1 || y0 != y1)
			drawLineUnclipped({ x0, y0 }, { x1, y1 }, color);
	}
}

void Canvas::drawText(const std::shared_ptr<Font>& font, const Point& pos, const std::string& text, const Colorf& color)
{
	CanvasFontGroup* canvasFont = GetFontGroup(font);

	double x = gridFit(origin.x + pos.x);
	double y = gridFit(origin.y + pos.y);

	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		CanvasGlyph* glyph = canvasFont->getGlyph(this, reader.character(), language.c_str());
		if (!glyph || !glyph->texture)
		{
			glyph = canvasFont->getGlyph(this, 32);
		}

		if (glyph->texture)
		{
			double gx = std::round(x + glyph->metrics.leftSideBearing);
			double gy = std::round(y + glyph->metrics.yOffset);
			drawGlyph(glyph->texture.get(), (float)gx, (float)gy, (float)glyph->uvwidth, (float)glyph->uvheight, (float)glyph->u, (float)glyph->v, (float)glyph->uvwidth, (float)glyph->uvheight, color);
		}

		x += std::round(glyph->metrics.advanceWidth);
		reader.next();
	}
}

void Canvas::drawTextEllipsis(const std::shared_ptr<Font>& font, const Point& pos, const Rect& clipBox, const std::string& text, const Colorf& color)
{
	drawText(font, pos, text, color);
}

Rect Canvas::measureText(const std::shared_ptr<Font>& font, const std::string& text)
{
	CanvasFontGroup* canvasFont = GetFontGroup(font);
	const TrueTypeTextMetrics& tm = canvasFont->GetTextMetrics();
	double lineHeight = tm.ascent + tm.descent + tm.lineGap;
	double x = 0.0;

	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		CanvasGlyph* glyph = canvasFont->getGlyph(this, reader.character(), language.c_str());
		if (!glyph || !glyph->texture)
		{
			glyph = canvasFont->getGlyph(this, 32);
		}

		x += std::round(glyph->metrics.advanceWidth);
		reader.next();
	}

	return Rect::xywh(0.0, 0.0, x / uiscale, lineHeight / uiscale);
}

FontMetrics Canvas::getFontMetrics(const std::shared_ptr<Font>& font)
{
	const TrueTypeTextMetrics& tm = GetFontGroup(font)->GetTextMetrics();
	FontMetrics metrics;
	metrics.external_leading = tm.lineGap / uiscale;
	metrics.ascent = tm.ascent / uiscale;
	metrics.descent = tm.descent / uiscale;
	metrics.height = (tm.ascent + tm.descent) / uiscale;
	return metrics;
}

int Canvas::getCharacterIndex(const std::shared_ptr<Font>& font, const std::string& text, const Point& hitPoint)
{
	CanvasFontGroup* canvasFont = GetFontGroup(font);

	double x = 0.0;
	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		CanvasGlyph* glyph = canvasFont->getGlyph(this, reader.character(), language.c_str());
		if (!glyph || !glyph->texture)
		{
			glyph = canvasFont->getGlyph(this, 32);
		}

		if (hitPoint.x <= (x + glyph->metrics.advanceWidth * 0.5) / uiscale)
			return (int)reader.position();

		x += std::round(glyph->metrics.advanceWidth);
		reader.next();
	}
	return (int)text.size();
}

VerticalTextPosition Canvas::verticalTextAlign(const std::shared_ptr<Font>& font)
{
	const TrueTypeTextMetrics& tm = GetFontGroup(font)->GetTextMetrics();
	VerticalTextPosition align;
	align.top = 0.0f;
	align.baseline = (tm.ascent + tm.lineGap * 0.5) / uiscale;
	align.bottom = (tm.ascent + tm.descent + tm.lineGap) / uiscale;
	return align;
}

CanvasFontGroup* Canvas::GetFontGroup(const std::shared_ptr<Font>& font)
{
	FontImpl* fontImpl = static_cast<FontImpl*>(const_cast<Font*>(font.get()));

	std::shared_ptr<CanvasFontGroup>& group = fontCache[{fontImpl->Name, fontImpl->Height}];
	if (!group)
		group = std::make_unique<CanvasFontGroup>(fontImpl->Name, std::round(fontImpl->Height * uiscale));
	return group.get();
}

void Canvas::drawLineUnclipped(const Point& p0, const Point& p1, const Colorf& color)
{
	if (p0.x == p1.x)
	{
		fillTile(gridFit(p0.x - 0.5), gridFit(p0.y), (float)uiscale, gridFit(p1.y - p0.y), color);
	}
	else if (p0.y == p1.y)
	{
		fillTile(gridFit(p0.x), gridFit(p0.y - 0.5), gridFit(p1.x - p0.x), (float)uiscale, color);
	}
	else
	{
		drawLineAntialiased(gridFit(p0.x), gridFit(p0.y), gridFit(p1.x), gridFit(p1.y), color);
	}
}

int Canvas::getClipMinX() const
{
	return clipStack.empty() ? 0 : (int)std::round(std::max(clipStack.back().x * uiscale, 0.0));
}

int Canvas::getClipMinY() const
{
	return clipStack.empty() ? 0 : (int)std::round(std::max(clipStack.back().y * uiscale, 0.0));
}

int Canvas::getClipMaxX() const
{
	return clipStack.empty() ? width : (int)std::round(std::min((clipStack.back().x + clipStack.back().width) * uiscale, (double)width));
}

int Canvas::getClipMaxY() const
{
	return clipStack.empty() ? height : (int)std::round(std::min((clipStack.back().y + clipStack.back().height) * uiscale, (double)height));
}

/////////////////////////////////////////////////////////////////////////////

class BitmapTexture : public CanvasTexture
{
public:
	std::vector<uint32_t> Data;
};

class BitmapCanvas : public Canvas
{
public:
	void begin(const Colorf& color) override;
	void end() override;

	void fillTile(float x, float y, float width, float height, Colorf color) override;
	void drawTile(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color) override;
	void drawGlyph(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color) override;
	void drawLineAntialiased(float x0, float y0, float x1, float y1, Colorf color) override;
	void plot(float x, float y, float alpha, const Colorf& color);

	std::unique_ptr<CanvasTexture> createTexture(int width, int height, const void* pixels, ImageFormat format = ImageFormat::B8G8R8A8) override;

	std::vector<uint32_t> pixels;
};

std::unique_ptr<CanvasTexture> BitmapCanvas::createTexture(int width, int height, const void* pixels, ImageFormat format)
{
	auto texture = std::make_unique<BitmapTexture>();
	texture->Width = width;
	texture->Height = height;
	texture->Data.resize(width * height);
	if (format == ImageFormat::B8G8R8A8)
	{
		memcpy(texture->Data.data(), pixels, width * height * sizeof(uint32_t));
	}
	else
	{
		const uint32_t* src = (const uint32_t*)pixels;
		uint32_t* dest = texture->Data.data();
		int count = width * height;
		for (int i = 0; i < count; i++)
		{
			uint32_t a = (src[i] >> 24) & 0xff;
			uint32_t b = (src[i] >> 16) & 0xff;
			uint32_t g = (src[i] >> 8) & 0xff;
			uint32_t r = src[i] & 0xff;
			dest[i] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}
	return texture;
}

void BitmapCanvas::plot(float x, float y, float alpha, const Colorf& color)
{
	int xx = (int)x;
	int yy = (int)y;

	uint32_t* dest = pixels.data() + xx + yy * width;

	uint32_t cred = (int32_t)clamp(color.r * 256.0f, 0.0f, 256.0f);
	uint32_t cgreen = (int32_t)clamp(color.g * 256.0f, 0.0f, 256.0f);
	uint32_t cblue = (int32_t)clamp(color.b * 256.0f, 0.0f, 256.0f);
	uint32_t calpha = (int32_t)clamp(color.a * alpha * 256.0f, 0.0f, 256.0f);
	uint32_t invalpha = 256 - calpha;

	uint32_t dpixel = *dest;
	uint32_t dalpha = dpixel >> 24;
	uint32_t dred = (dpixel >> 16) & 0xff;
	uint32_t dgreen = (dpixel >> 8) & 0xff;
	uint32_t dblue = dpixel & 0xff;

	// dest.rgba = color.rgba + dest.rgba * (1-color.a)
	uint32_t a = (calpha * calpha + dalpha * invalpha + 127) >> 8;
	uint32_t r = (cred * calpha + dred * invalpha + 127) >> 8;
	uint32_t g = (cgreen * calpha + dgreen * invalpha + 127) >> 8;
	uint32_t b = (cblue * calpha + dblue * invalpha + 127) >> 8;
	*dest = (a << 24) | (r << 16) | (g << 8) | b;
}

static float fpart(float x)
{
	return x - std::floor(x);
}

static float rfpart(float x)
{
	return 1 - fpart(x);
}

void BitmapCanvas::drawLineAntialiased(float x0, float y0, float x1, float y1, Colorf color)
{
	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
	
	if (steep)
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	float dx = x1 - x0;
	float dy = y1 - y0;
	float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

	// handle first endpoint
	float xend = std::round(x0);
	float yend = y0 + gradient * (xend - x0);
	float xgap = rfpart(x0 + 0.5f);
	float xpxl1 = xend; // this will be used in the main loop
	float ypxl1 = std::floor(yend);
	if (steep)
	{
		plot(ypxl1, xpxl1, rfpart(yend) * xgap, color);
		plot(ypxl1 + 1, xpxl1, fpart(yend) * xgap, color);
	}
	else
	{
		plot(xpxl1, ypxl1, rfpart(yend) * xgap, color);
		plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap, color);
	}
	float intery = yend + gradient; // first y-intersection for the main loop

	// handle second endpoint
	xend = std::floor(x1 + 0.5f);
	yend = y1 + gradient * (xend - x1);
	xgap = fpart(x1 + 0.5f);
	float xpxl2 = xend; // this will be used in the main loop
	float ypxl2 = std::floor(yend);
	if (steep)
	{
		plot(ypxl2, xpxl2, rfpart(yend) * xgap, color);
		plot(ypxl2 + 1.0f, xpxl2, fpart(yend) * xgap, color);
	}
	else
	{
		plot(xpxl2, ypxl2, rfpart(yend) * xgap, color);
		plot(xpxl2, ypxl2 + 1.0f, fpart(yend) * xgap, color);
	}

	// main loop
	if (steep)
	{
		for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x++)
		{
			plot(std::floor(intery), x, rfpart(intery), color);
			plot(std::floor(intery) + 1.0f, x, fpart(intery), color);
			intery = intery + gradient;
		}
	}
	else
	{
		for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x++)
		{
			plot(x, std::floor(intery), rfpart(intery), color);
			plot(x, std::floor(intery) + 1, fpart(intery), color);
			intery = intery + gradient;
		}
	}
}

void BitmapCanvas::fillTile(float left, float top, float width, float height, Colorf color)
{
	if (width <= 0.0f || height <= 0.0f || color.a <= 0.0f)
		return;

	int dwidth = this->width;
	uint32_t* dest = this->pixels.data();

	int x0 = (int)left;
	int x1 = (int)(left + width);
	int y0 = (int)top;
	int y1 = (int)(top + height);

	x0 = std::max(x0, getClipMinX());
	y0 = std::max(y0, getClipMinY());
	x1 = std::min(x1, getClipMaxX());
	y1 = std::min(y1, getClipMaxY());
	if (x1 <= x0 || y1 <= y0)
		return;

	uint32_t cred = (int32_t)clamp(color.r * 255.0f, 0.0f, 255.0f);
	uint32_t cgreen = (int32_t)clamp(color.g * 255.0f, 0.0f, 255.0f);
	uint32_t cblue = (int32_t)clamp(color.b * 255.0f, 0.0f, 255.0f);
	uint32_t calpha = (int32_t)clamp(color.a * 255.0f, 0.0f, 255.0f);
	uint32_t invalpha = 256 - (calpha + (calpha >> 7));

	if (invalpha == 0) // Solid fill
	{
		uint32_t c = (calpha << 24) | (cred << 16) | (cgreen << 8) | cblue;
#ifdef USE_SSE2
		__m128i cargb = _mm_set1_epi32(c);
#endif

		for (int y = y0; y < y1; y++)
		{
			uint32_t* dline = dest + y * dwidth;

			int x = x0;
#ifdef USE_SSE2
			int ssex1 = x0 + (((x1 - x0) >> 2) << 2);
			while (x < ssex1)
			{
				_mm_storeu_si128((__m128i*)(dline + x), cargb);
				x += 4;
			}
#endif

			while (x < x1)
			{
				dline[x] = c;
				x++;
			}
		}
	}
	else // Alpha blended fill
	{
		cred <<= 8;
		cgreen <<= 8;
		cblue <<= 8;
		calpha <<= 8;
#ifdef USE_SSE2
		__m128i cargb = _mm_set_epi16(calpha, cred, cgreen, cblue, calpha, cred, cgreen, cblue);
		__m128i cinvalpha = _mm_set1_epi16(invalpha);
#endif

		for (int y = y0; y < y1; y++)
		{
			uint32_t* dline = dest + y * dwidth;

			int x = x0;
#ifdef USE_SSE2
			int ssex1 = x0 + (((x1 - x0) >> 1) << 1);
			while (x < ssex1)
			{
				__m128i dpixel = _mm_loadl_epi64((const __m128i*)(dline + x));
				dpixel = _mm_unpacklo_epi8(dpixel, _mm_setzero_si128());

				// dest.rgba = color.rgba + dest.rgba * (1-color.a)
				__m128i result = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(cargb, _mm_mullo_epi16(dpixel, cinvalpha)), _mm_set1_epi16(127)), 8);
				_mm_storel_epi64((__m128i*)(dline + x), _mm_packus_epi16(result, _mm_setzero_si128()));
				x += 2;
			}
#endif

			while (x < x1)
			{
				uint32_t dpixel = dline[x];
				uint32_t dalpha = dpixel >> 24;
				uint32_t dred = (dpixel >> 16) & 0xff;
				uint32_t dgreen = (dpixel >> 8) & 0xff;
				uint32_t dblue = dpixel & 0xff;

				// dest.rgba = color.rgba + dest.rgba * (1-color.a)
				uint32_t a = (calpha + dalpha * invalpha + 127) >> 8;
				uint32_t r = (cred + dred * invalpha + 127) >> 8;
				uint32_t g = (cgreen + dgreen * invalpha + 127) >> 8;
				uint32_t b = (cblue + dblue * invalpha + 127) >> 8;
				dline[x] = (a << 24) | (r << 16) | (g << 8) | b;
				x++;
			}
		}
	}
}

#if 1 // drawTile linear filtered

void BitmapCanvas::drawTile(CanvasTexture* tex, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	if (width <= 0.0f || height <= 0.0f || color.a <= 0.0f)
		return;

	auto texture = static_cast<BitmapTexture*>(tex);
	int swidth = texture->Width;
	int sheight = texture->Height;
	const uint32_t* src = texture->Data.data();

	int dwidth = this->width;
	uint32_t* dest = this->pixels.data();

	int x0 = (int)left;
	int x1 = (int)(left + width);
	int y0 = (int)top;
	int y1 = (int)(top + height);

	x0 = std::max(x0, getClipMinX());
	y0 = std::max(y0, getClipMinY());
	x1 = std::min(x1, getClipMaxX());
	y1 = std::min(y1, getClipMaxY());
	if (x1 <= x0 || y1 <= y0)
		return;

	uint32_t cred = (int32_t)clamp(color.r * 256.0f, 0.0f, 256.0f);
	uint32_t cgreen = (int32_t)clamp(color.g * 256.0f, 0.0f, 256.0f);
	uint32_t cblue = (int32_t)clamp(color.b * 256.0f, 0.0f, 256.0f);
	uint32_t calpha = (int32_t)clamp(color.a * 256.0f, 0.0f, 256.0f);
#ifdef USE_SSE2
	__m128i cargb = _mm_set_epi16(calpha, cred, cgreen, cblue, calpha, cred, cgreen, cblue);
#endif

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y - top);
		float vfrac = vpix - (int)vpix;
		int sy0 = (int)vpix;
		int sy1 = sy0 + 1;
		sy0 = sy0 < sheight ? sy0 : sheight - 1;
		sy1 = sy1 < sheight ? sy1 : sheight - 1;

		const uint32_t* sline0 = src + sy0 * swidth;
		const uint32_t* sline1 = src + sy1 * swidth;
		uint32_t* dline = dest + y * dwidth;

		uint32_t ty = (int)(vfrac * 128.0f);
		uint32_t invty = 128 - ty;
#ifdef USE_SSE2
		__m128i tyy = _mm_set_epi16(invty, invty, ty, ty, invty, invty, ty, ty);
#endif

		int x = x0;

#ifdef USE_SSE2
		int ssex1 = x0 + (((x1 - x0) >> 1) << 1);
		while (x < ssex1)
		{
			float upix0 = u + uscale * (x - left);
			float upix1 = u + uscale * (x + 1.0f - left);
			float ufrac0 = upix0 - (int)upix0;
			float ufrac1 = upix1 - (int)upix1;
			int sx0[2] = { (int)upix0, (int)upix1 };
			int sx1[2] = { sx0[0] + 1, sx0[1] + 1 };
			sx0[0] = sx0[0] < swidth ? sx0[0] : swidth - 1;
			sx0[1] = sx0[1] < swidth ? sx0[1] : swidth - 1;
			sx1[0] = sx1[0] < swidth ? sx1[0] : swidth - 1;
			sx1[1] = sx1[1] < swidth ? sx1[1] : swidth - 1;

			// Linear filter sample:
			uint32_t tx0 = (int)(ufrac0 * 128.0f);
			uint32_t tx1 = (int)(ufrac1 * 128.0f);
			uint32_t invtx0 = 128 - tx0;
			uint32_t invtx1 = 128 - tx1;
			__m128i txx = _mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(_mm_set_epi16(invtx1, tx1, invtx1, tx1, invtx0, tx0, invtx0, tx0), tyy), _mm_set1_epi16(63)), 7);
			__m128i t00 = _mm_shufflehi_epi16(_mm_shufflelo_epi16(txx, _MM_SHUFFLE(3, 3, 3, 3)), _MM_SHUFFLE(3, 3, 3, 3));
			__m128i spixel00 = _mm_mullo_epi16(t00, _mm_unpacklo_epi8(_mm_set_epi32(0, 0, sline0[sx0[1]], sline0[sx0[0]]), _mm_setzero_si128()));
			__m128i t10 = _mm_shufflehi_epi16(_mm_shufflelo_epi16(txx, _MM_SHUFFLE(2, 2, 2, 2)), _MM_SHUFFLE(2, 2, 2, 2));
			__m128i spixel10 = _mm_mullo_epi16(t10, _mm_unpacklo_epi8(_mm_set_epi32(0, 0, sline0[sx1[1]], sline0[sx1[0]]), _mm_setzero_si128()));
			__m128i t01 = _mm_shufflehi_epi16(_mm_shufflelo_epi16(txx, _MM_SHUFFLE(1, 1, 1, 1)), _MM_SHUFFLE(1, 1, 1, 1));
			__m128i spixel01 = _mm_mullo_epi16(t01, _mm_unpacklo_epi8(_mm_set_epi32(0, 0, sline1[sx0[1]], sline1[sx0[0]]), _mm_setzero_si128()));
			__m128i t11 = _mm_shufflehi_epi16(_mm_shufflelo_epi16(txx, _MM_SHUFFLE(0, 0, 0, 0)), _MM_SHUFFLE(0, 0, 0, 0));
			__m128i spixel11 = _mm_mullo_epi16(t11, _mm_unpacklo_epi8(_mm_set_epi32(0, 0, sline1[sx1[1]], sline1[sx1[0]]), _mm_setzero_si128()));
			__m128i spixel = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(spixel00, spixel10), spixel01), spixel11), _mm_set1_epi16(63)), 7);

			// Pixel shade
			spixel = _mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(spixel, cargb), _mm_set1_epi16(127)), 8);

			// Rescale from [0,255] to [0,256]
			__m128i sa = _mm_shufflehi_epi16(_mm_shufflelo_epi16(spixel, _MM_SHUFFLE(3, 3, 3, 3)), _MM_SHUFFLE(3, 3, 3, 3));
			sa = _mm_add_epi16(sa, _mm_srli_epi16(sa, 7));
			__m128i sinva = _mm_sub_epi16(_mm_set1_epi16(256), sa);

			__m128i dpixel = _mm_loadl_epi64((const __m128i*)(dline + x));
			dpixel = _mm_unpacklo_epi8(dpixel, _mm_setzero_si128());

			// dest.rgba = color.rgba * src.rgba * src.a + dest.rgba * (1-src.a)
			__m128i result = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(spixel, sa), _mm_mullo_epi16(dpixel, sinva)), _mm_set1_epi16(127)), 8);
			_mm_storel_epi64((__m128i*)(dline + x), _mm_packus_epi16(result, _mm_setzero_si128()));
			x += 2;
		}
#endif

		while (x < x1)
		{
			float upix = u + uscale * (x - left);
			float ufrac = upix - (int)upix;
			int sx0 = (int)upix;
			int sx1 = sx0 + 1;
			sx0 = sx0 < swidth ? sx0 : swidth - 1;
			sx1 = sx1 < swidth ? sx1 : swidth - 1;

			// Linear filter sample:
			uint32_t spixel00 = sline0[sx0];
			uint32_t spixel10 = sline0[sx1];
			uint32_t spixel01 = sline1[sx0];
			uint32_t spixel11 = sline1[sx1];

			uint32_t salpha00 = spixel00 >> 24;
			uint32_t sred00 = (spixel00 >> 16) & 0xff;
			uint32_t sgreen00 = (spixel00 >> 8) & 0xff;
			uint32_t sblue00 = spixel00 & 0xff;

			uint32_t salpha10 = spixel10 >> 24;
			uint32_t sred10 = (spixel10 >> 16) & 0xff;
			uint32_t sgreen10 = (spixel10 >> 8) & 0xff;
			uint32_t sblue10 = spixel10 & 0xff;

			uint32_t salpha01 = spixel01 >> 24;
			uint32_t sred01 = (spixel01 >> 16) & 0xff;
			uint32_t sgreen01 = (spixel01 >> 8) & 0xff;
			uint32_t sblue01 = spixel01 & 0xff;

			uint32_t salpha11 = spixel11 >> 24;
			uint32_t sred11 = (spixel11 >> 16) & 0xff;
			uint32_t sgreen11 = (spixel11 >> 8) & 0xff;
			uint32_t sblue11 = spixel11 & 0xff;

			uint32_t tx = (int)(ufrac * 128.0f);
			uint32_t invtx = 128 - tx;

			uint32_t t00 = (invtx * invty + 63) >> 7;
			uint32_t t10 = (tx * invty + 63) >> 7;
			uint32_t t01 = (invtx * ty + 63) >> 7;
			uint32_t t11 = (tx * ty + 63) >> 7;

			uint32_t salpha = (t00 * salpha00 + t10 * salpha10 + t01 * salpha01 + t11 * salpha11 + 63) >> 7;
			uint32_t sred = (t00 * sred00 + t10 * sred10 + t01 * sred01 + t11 * sred11 + 63) >> 7;
			uint32_t sgreen = (t00 * sgreen00 + t10 * sgreen10 + t01 * sgreen01 + t11 * sgreen11 + 63) >> 7;
			uint32_t sblue = (t00 * sblue00 + t10 * sblue10 + t01 * sblue01 + t11 * sblue11 + 63) >> 7;

			// Pixel shade
			sred = (cred * sred + 127) >> 8;
			sgreen = (cgreen * sgreen + 127) >> 8;
			sblue = (cblue * sblue + 127) >> 8;
			salpha = (calpha * salpha + 127) >> 8;

			// Rescale from [0,255] to [0,256]
			uint32_t sa = salpha + (salpha >> 7);
			uint32_t sinva = 256 - sa;

			// Load dest pixel
			uint32_t dpixel = dline[x];
			uint32_t dalpha = dpixel >> 24;
			uint32_t dred = (dpixel >> 16) & 0xff;
			uint32_t dgreen = (dpixel >> 8) & 0xff;
			uint32_t dblue = dpixel & 0xff;

			// dest.rgba = color.rgba * src.rgba * src.a + dest.rgba * (1-src.a)
			uint32_t a = (salpha * sa + dalpha * sinva + 127) >> 8;
			uint32_t r = (sred * sa + dred * sinva + 127) >> 8;
			uint32_t g = (sgreen * sa + dgreen * sinva + 127) >> 8;
			uint32_t b = (sblue * sa + dblue * sinva + 127) >> 8;
			dline[x] = (a << 24) | (r << 16) | (g << 8) | b;
			x++;
		}
	}
}

#else // drawTile nearest version:
void BitmapCanvas::drawTile(CanvasTexture* tex, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	if (width <= 0.0f || height <= 0.0f || color.a <= 0.0f)
		return;

	auto texture = static_cast<BitmapTexture*>(tex);
	int swidth = texture->Width;
	const uint32_t* src = texture->Data.data();

	int dwidth = this->width;
	uint32_t* dest = this->pixels.data();

	int x0 = (int)left;
	int x1 = (int)(left + width);
	int y0 = (int)top;
	int y1 = (int)(top + height);

	x0 = std::max(x0, getClipMinX());
	y0 = std::max(y0, getClipMinY());
	x1 = std::min(x1, getClipMaxX());
	y1 = std::min(y1, getClipMaxY());
	if (x1 <= x0 || y1 <= y0)
		return;

	uint32_t cred = (int32_t)clamp(color.r * 256.0f, 0.0f, 256.0f);
	uint32_t cgreen = (int32_t)clamp(color.g * 256.0f, 0.0f, 256.0f);
	uint32_t cblue = (int32_t)clamp(color.b * 256.0f, 0.0f, 256.0f);
	uint32_t calpha = (int32_t)clamp(color.a * 256.0f, 0.0f, 256.0f);
#ifdef USE_SSE2
	__m128i cargb = _mm_set_epi16(calpha, cred, cgreen, cblue, calpha, cred, cgreen, cblue);
#endif

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint32_t* sline = src + ((int)vpix) * swidth;
		uint32_t* dline = dest + y * dwidth;

		int x = x0;
#ifdef USE_SSE2
		int ssex1 = x0 + (((x1 - x0) >> 1) << 1);
		while (x < ssex1)
		{
			float upix0 = u + uscale * (x + 0.5f - left);
			float upix1 = u + uscale * (x + 1 + 0.5f - left);
			uint32_t spixel0 = sline[(int)upix0];
			uint32_t spixel1 = sline[(int)upix1];
			__m128i spixel = _mm_set_epi32(0, 0, spixel1, spixel0);
			spixel = _mm_unpacklo_epi8(spixel, _mm_setzero_si128());

			__m128i dpixel = _mm_loadl_epi64((const __m128i*)(dline + x));
			dpixel = _mm_unpacklo_epi8(dpixel, _mm_setzero_si128());

			// Pixel shade
			spixel = _mm_srli_epi16(_mm_add_epi16(_mm_mullo_epi16(spixel, cargb), _mm_set1_epi16(127)), 8);

			// Rescale from [0,255] to [0,256]
			__m128i sa = _mm_shufflehi_epi16(_mm_shufflelo_epi16(spixel, _MM_SHUFFLE(3, 3, 3, 3)), _MM_SHUFFLE(3, 3, 3, 3));
			sa = _mm_add_epi16(sa, _mm_srli_epi16(sa, 7));
			__m128i sinva = _mm_sub_epi16(_mm_set1_epi16(256), sa);

			// dest.rgba = color.rgba * src.rgba * src.a + dest.rgba * (1-src.a)
			__m128i result = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(spixel, sa), _mm_mullo_epi16(dpixel, sinva)), _mm_set1_epi16(127)), 8);
			_mm_storel_epi64((__m128i*)(dline + x), _mm_packus_epi16(result, _mm_setzero_si128()));
			x += 2;
		}
#endif

		while (x < x1)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint32_t spixel = sline[(int)upix];
			uint32_t salpha = spixel >> 24;
			uint32_t sred = (spixel >> 16) & 0xff;
			uint32_t sgreen = (spixel >> 8) & 0xff;
			uint32_t sblue = spixel & 0xff;

			uint32_t dpixel = dline[x];
			uint32_t dalpha = dpixel >> 24;
			uint32_t dred = (dpixel >> 16) & 0xff;
			uint32_t dgreen = (dpixel >> 8) & 0xff;
			uint32_t dblue = dpixel & 0xff;

			// Pixel shade
			sred = (cred * sred + 127) >> 8;
			sgreen = (cgreen * sgreen + 127) >> 8;
			sblue = (cblue * sblue + 127) >> 8;
			salpha = (calpha * salpha + 127) >> 8;

			// Rescale from [0,255] to [0,256]
			uint32_t sa = salpha + (salpha >> 7);
			uint32_t sinva = 256 - sa;

			// dest.rgba = color.rgba * src.rgba * src.a + dest.rgba * (1-src.a)
			uint32_t a = (salpha * sa + dalpha * sinva + 127) >> 8;
			uint32_t r = (sred * sa + dred * sinva + 127) >> 8;
			uint32_t g = (sgreen * sa + dgreen * sinva + 127) >> 8;
			uint32_t b = (sblue * sa + dblue * sinva + 127) >> 8;
			dline[x] = (a << 24) | (r << 16) | (g << 8) | b;
			x++;
		}
	}
}
#endif

void BitmapCanvas::drawGlyph(CanvasTexture* tex, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	if (width <= 0.0f || height <= 0.0f)
		return;

	auto texture = static_cast<BitmapTexture*>(tex);
	int swidth = texture->Width;
	const uint32_t* src = texture->Data.data();

	int dwidth = this->width;
	uint32_t* dest = this->pixels.data();

	int x0 = (int)left;
	int x1 = (int)(left + width);
	int y0 = (int)top;
	int y1 = (int)(top + height);

	x0 = std::max(x0, getClipMinX());
	y0 = std::max(y0, getClipMinY());
	x1 = std::min(x1, getClipMaxX());
	y1 = std::min(y1, getClipMaxY());
	if (x1 <= x0 || y1 <= y0)
		return;

#if 1 // Use gamma correction

	// To linear
	float cred = color.r * color.r; // std::pow(color.r, 2.2f);
	float cgreen = color.g * color.g; // std::pow(color.g, 2.2f);
	float cblue = color.b * color.b; // std::pow(color.b, 2.2f);
#ifdef USE_SSE2
	__m128 crgba = _mm_set_ps(0.0f, cred, cgreen, cblue);
#endif

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint32_t* sline = src + ((int)vpix) * swidth;
		uint32_t* dline = dest + y * dwidth;

		int x = x0;
#ifdef USE_SSE2
		while (x < x1)
		{
			float upix = u + uscale * (x + 0.5f - left);
			__m128i spixel = _mm_cvtsi32_si128(sline[(int)upix]);
			spixel = _mm_unpacklo_epi8(spixel, _mm_setzero_si128());
			spixel = _mm_unpacklo_epi16(spixel, _mm_setzero_si128());
			__m128 srgba = _mm_mul_ps(_mm_cvtepi32_ps(spixel), _mm_set_ps1(1.0f / 255.0f));

			__m128i dpixel = _mm_cvtsi32_si128(dline[x]);
			dpixel = _mm_unpacklo_epi8(dpixel, _mm_setzero_si128());
			dpixel = _mm_unpacklo_epi16(dpixel, _mm_setzero_si128());
			__m128 drgba = _mm_mul_ps(_mm_cvtepi32_ps(dpixel), _mm_set_ps1(1.0f / 255.0f));

			// To linear
			drgba = _mm_mul_ps(drgba, drgba);

			// dest.rgb = color.rgb * src.rgb + dest.rgb * (1-src.rgb)
			__m128 frgba = _mm_add_ps(_mm_mul_ps(crgba, srgba), _mm_mul_ps(drgba, _mm_sub_ps(_mm_set_ps1(1.0f), srgba)));

			// To srgb
			frgba = _mm_sqrt_ps(frgba);

			__m128i rgba = _mm_cvtps_epi32(_mm_add_ps(_mm_mul_ps(frgba, _mm_set_ps1(255.0f)), _mm_set_ps1(0.5f)));
			rgba = _mm_packs_epi32(rgba, _mm_setzero_si128());
			rgba = _mm_packus_epi16(rgba, _mm_setzero_si128());
			dline[x] = ((uint32_t)_mm_cvtsi128_si32(rgba)) | 0xff000000;
			x++;
		}
#else
		while (x < x1)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint32_t spixel = sline[(int)upix];
			float sred = ((spixel >> 16) & 0xff) * (1.0f / 255.0f);
			float sgreen = ((spixel >> 8) & 0xff) * (1.0f / 255.0f);
			float sblue = (spixel & 0xff) * (1.0f / 255.0f);

			uint32_t dpixel = dline[x];
			float dred = ((dpixel >> 16) & 0xff) * (1.0f / 255.0f);
			float dgreen = ((dpixel >> 8) & 0xff) * (1.0f / 255.0f);
			float dblue = (dpixel & 0xff) * (1.0f / 255.0f);

			// To linear
			dred = dred * dred; // std::pow(dred, 2.2f);
			dgreen = dgreen * dgreen; // std::pow(dgreen, 2.2f);
			dblue = dblue * dblue; // std::pow(dblue, 2.2f);

			// dest.rgb = color.rgb * src.rgb + dest.rgb * (1-src.rgb)
			double fr = cred * sred + dred * (1.0f - sred);
			double fg = cgreen * sgreen + dgreen * (1.0f - sgreen);
			double fb = cblue * sblue + dblue * (1.0f - sblue);

			// To srgb
			fr = std::sqrt(fr); // std::pow(fr, 1.0f / 2.2f);
			fg = std::sqrt(fg); // std::pow(fg, 1.0f / 2.2f);
			fb = std::sqrt(fb); // std::pow(fb, 1.0f / 2.2f);

			uint32_t r = (int)(fr * 255.0f + 0.5f);
			uint32_t g = (int)(fg * 255.0f + 0.5f);
			uint32_t b = (int)(fb * 255.0f + 0.5f);
			dline[x] = 0xff000000 | (r << 16) | (g << 8) | b;
			x++;
		}
#endif
	}

#else

	uint32_t cred = (int32_t)clamp(color.r * 255.0f, 0.0f, 255.0f);
	uint32_t cgreen = (int32_t)clamp(color.g * 255.0f, 0.0f, 255.0f);
	uint32_t cblue = (int32_t)clamp(color.b * 255.0f, 0.0f, 255.0f);
#ifdef USE_SSE2
	__m128i crgba = _mm_set_epi16(0, cred, cgreen, cblue, 0, cred, cgreen, cblue);
#endif

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint32_t* sline = src + ((int)vpix) * swidth;
		uint32_t* dline = dest + y * dwidth;

		int x = x0;
#ifdef USE_SSE2
		int ssex1 = x0 + (((x1 - x0) >> 1) << 1);
		while (x < ssex1)
		{
			float upix0 = u + uscale * (x + 0.5f - left);
			float upix1 = u + uscale * (x + 1 + 0.5f - left);
			uint32_t spixel0 = sline[(int)upix0];
			uint32_t spixel1 = sline[(int)upix1];
			__m128i spixel = _mm_set_epi32(0, 0, spixel1, spixel0);
			spixel = _mm_unpacklo_epi8(spixel, _mm_setzero_si128());

			__m128i dpixel = _mm_loadl_epi64((const __m128i*)(dline + x));
			dpixel = _mm_unpacklo_epi8(dpixel, _mm_setzero_si128());

			// Rescale from [0,255] to [0,256]
			spixel = _mm_add_epi16(spixel, _mm_srli_epi16(spixel, 7));

			// dest.rgb = color.rgb * src.rgb + dest.rgb * (1-src.rgb)
			__m128i result = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(crgba, spixel), _mm_mullo_epi16(dpixel, _mm_sub_epi16(_mm_set1_epi16(256), spixel))), _mm_set1_epi16(127)), 8);
			_mm_storel_epi64((__m128i*)(dline + x), _mm_or_si128(_mm_packus_epi16(result, _mm_setzero_si128()), _mm_set1_epi32(0xff000000)));
			x += 2;
		}
#endif

		while (x < x1)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint32_t spixel = sline[(int)upix];
			uint32_t sred = (spixel >> 16) & 0xff;
			uint32_t sgreen = (spixel >> 8) & 0xff;
			uint32_t sblue = spixel & 0xff;

			uint32_t dpixel = dline[x];
			uint32_t dred = (dpixel >> 16) & 0xff;
			uint32_t dgreen = (dpixel >> 8) & 0xff;
			uint32_t dblue = dpixel & 0xff;

			// Rescale from [0,255] to [0,256]
			sred += sred >> 7;
			sgreen += sgreen >> 7;
			sblue += sblue >> 7;

			// dest.rgb = color.rgb * src.rgb + dest.rgb * (1-src.rgb)
			uint32_t r = (cred * sred + dred * (256 - sred) + 127) >> 8;
			uint32_t g = (cgreen * sgreen + dgreen * (256 - sgreen) + 127) >> 8;
			uint32_t b = (cblue * sblue + dblue * (256 - sblue) + 127) >> 8;
			dline[x] = 0xff000000 | (r << 16) | (g << 8) | b;
			x++;
		}
	}
#endif
}

void BitmapCanvas::begin(const Colorf& color)
{
	Canvas::begin(color);

	uint32_t r = (int32_t)clamp(color.r * 255.0f, 0.0f, 255.0f);
	uint32_t g = (int32_t)clamp(color.g * 255.0f, 0.0f, 255.0f);
	uint32_t b = (int32_t)clamp(color.b * 255.0f, 0.0f, 255.0f);
	uint32_t a = (int32_t)clamp(color.a * 255.0f, 0.0f, 255.0f);
	uint32_t bgcolor = (a << 24) | (r << 16) | (g << 8) | b;
	pixels.clear();
	pixels.resize(width * height, bgcolor);
}

void BitmapCanvas::end()
{
	if (window)
		window->PresentBitmap(width, height, pixels.data());
}

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Canvas> Canvas::create()
{
	return std::make_unique<BitmapCanvas>();
}
