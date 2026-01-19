
// #define DUMP_GLYPH

#include "truetypefont.h"
#include "core/pathfill.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <utility>

#ifdef DUMP_GLYPH
#include <fstream>
#endif

TrueTypeFont::TrueTypeFont(std::shared_ptr<TTFDataBuffer> initdata, int ttcFontIndex) : data(std::move(initdata))
{
	if (data->size() > 0x7fffffff)
		throw std::runtime_error("TTF file is larger than 2 gigabytes!");

	TrueTypeFileReader reader(data->data(), data->size());
	ttf_Tag versionTag = reader.ReadTag();
	reader.Seek(0);

	if (memcmp(versionTag.data(), "ttcf", 4) == 0) // TTC header
	{
		ttcHeader.Load(reader);
		if (ttcFontIndex >= (int)ttcHeader.numFonts)
			throw std::runtime_error("TTC font index out of bounds");
		reader.Seek(ttcHeader.tableDirectoryOffsets[ttcFontIndex]);
	}

	directory.Load(reader);

	// Load required tables:

	reader = directory.GetReader(data->data(), data->size(), "head");
	head.Load(reader);

	reader = directory.GetReader(data->data(), data->size(), "hhea");
	hhea.Load(reader);

	reader = directory.GetReader(data->data(), data->size(), "maxp");
	maxp.Load(reader);

	reader = directory.GetReader(data->data(), data->size(), "hmtx");
	hmtx.Load(hhea, maxp, reader);

	reader = directory.GetReader(data->data(), data->size(), "name");
	name.Load(reader);

	reader = directory.GetReader(data->data(), data->size(), "OS/2");
	os2.Load(reader);

	reader = directory.GetReader(data->data(), data->size(), "cmap");
	cmap.Load(reader);

	LoadCharacterMapEncoding(reader);

	if (directory.ContainsTTFOutlines())
	{
		// Load TTF Outlines:
		reader = directory.GetReader(data->data(), data->size(), "loca");
		loca.Load(head, maxp, reader);

		glyf = directory.GetRecord("glyf");
	}
	else if (directory.ContainsCFFData())
	{
		cff.Record = directory.GetRecord("CFF ");
		reader = cff.Record.GetReader(data->data(), data->size());
		cff.Load(reader);
	}
	else
	{
		throw std::runtime_error("Unsupported font outlines");
	}

#ifdef DUMP_GLYPH
	LoadGlyph(GetGlyphIndex('g'), 13.0);
#endif
}

TTCFontName TrueTypeFont::GetFontName() const
{
	return name.GetFontName();
}

TrueTypeTextMetrics TrueTypeFont::GetTextMetrics(double height) const
{
	double scale = height / head.unitsPerEm;
	double internalLeading = height - os2.sTypoAscender * scale + os2.sTypoDescender * scale;

	TrueTypeTextMetrics metrics;
	if (os2.usWinAscent != 0 || os2.usWinDescent != 0)
	{
		metrics.ascent = os2.usWinAscent * scale;
		metrics.descent = os2.usWinDescent * scale;
	}
	else
	{
		// Note: os2.sTypoDescender is negative, os2.usWinDescent is not. We use positive in our metrics
		double internalLeading = height - os2.sTypoAscender * scale + os2.sTypoDescender * scale;
		metrics.ascent = os2.sTypoAscender * scale + internalLeading;
		metrics.descent = -(os2.sTypoDescender * scale);
	}
	metrics.lineGap = os2.sTypoLineGap * scale;
	metrics.capHeight = os2.sCapHeight * scale;
	metrics.stemV = 0.0; // How to get this?
	metrics.italicAngle = 0.0; // How to get this?
	metrics.defaultWidth = 0.0; // How to get this?
	metrics.bbox.minX = head.xMin * scale;
	metrics.bbox.maxX = head.xMax * scale;
	metrics.bbox.minY = head.yMin * scale;
	metrics.bbox.maxY = head.yMax * scale;
	return metrics;
}

double TrueTypeFont::GetAdvanceWidth(uint32_t glyphIndex, double height) const
{
	if (glyphIndex >= hhea.numberOfHMetrics)
	{
		return hmtx.hMetrics[hhea.numberOfHMetrics - 1].advanceWidth * height / head.unitsPerEm;
	}
	else
	{
		return hmtx.hMetrics[glyphIndex].advanceWidth * height / head.unitsPerEm;
	}
}

TrueTypeGlyph TrueTypeFont::LoadGlyph(uint32_t glyphIndex, double height) const
{
	if (directory.ContainsTTFOutlines())
	{
		return LoadTTFGlyph(glyphIndex, height);
	}
	else
	{
		return LoadCFFGlyph(glyphIndex, height);
	}
}

TrueTypeGlyph TrueTypeFont::LoadTTFGlyph(uint32_t glyphIndex, double height) const
{
	double scale = height / head.unitsPerEm;
	double scaleX = 3.0f;
	double scaleY = -1.0f;

	ttf_uint16 advanceWidth = 0;
	ttf_int16 lsb = 0;
	if (glyphIndex >= hhea.numberOfHMetrics)
	{
		advanceWidth = hmtx.hMetrics[hhea.numberOfHMetrics - 1].advanceWidth;
		lsb = hmtx.leftSideBearings[glyphIndex - hhea.numberOfHMetrics];
	}
	else
	{
		advanceWidth = hmtx.hMetrics[glyphIndex].advanceWidth;
		lsb = hmtx.hMetrics[glyphIndex].lsb;
	}

	// Glyph is missing if the offset is the same as the next glyph (0 bytes glyph length)
	bool missing = glyphIndex + 1 < loca.offsets.size() ? loca.offsets[glyphIndex] == loca.offsets[glyphIndex + 1] : false;
	if (missing)
	{
		TrueTypeGlyph glyph;

		// TBD: gridfit or not?
		glyph.advanceWidth = (int)std::round(advanceWidth * scale * scaleX);
		glyph.leftSideBearing = (int)std::round(lsb * scale * scaleX);
		glyph.yOffset = 0;

		return glyph;
	}

	TTF_SimpleGlyph g;
	LoadGlyph(g, glyphIndex);

	// Create glyph path:
	PathFillDesc path;
	path.fill_mode = PathFillMode::winding;

	int startPoint = 0;
	int numberOfContours = (int)g.endPtsOfContours.size();
	for (int i = 0; i < numberOfContours; i++)
	{
		int endPoint = g.endPtsOfContours[i];
		if (endPoint < startPoint)
			throw std::runtime_error("Invalid glyph");

		bool prevIsControlPoint;
		bool isControlPoint = !(g.flags[endPoint] & TTF_ON_CURVE_POINT);
		bool nextIsControlPoint = !(g.flags[startPoint] & TTF_ON_CURVE_POINT);
		if (isControlPoint)
		{
			PathPoint nextpoint(g.points[startPoint].x, g.points[startPoint].y);
			if (nextIsControlPoint)
			{
				PathPoint curpoint(g.points[endPoint].x, g.points[endPoint].y);
				PathPoint midpoint = (curpoint + nextpoint) / 2;
				path.MoveTo(midpoint * scale);
				prevIsControlPoint = isControlPoint;
			}
			else
			{
				path.MoveTo(PathPoint(g.points[startPoint].x, g.points[startPoint].y) * scale);
				prevIsControlPoint = false;
			}
		}
		else
		{
			path.MoveTo(PathPoint(g.points[endPoint].x, g.points[endPoint].y) * scale);
			prevIsControlPoint = isControlPoint;
		}

		int pos = startPoint;
		while (pos <= endPoint)
		{
			int nextpos = pos + 1 <= endPoint ? pos + 1 : startPoint;
			bool isControlPoint = !(g.flags[pos] & TTF_ON_CURVE_POINT);
			bool nextIsControlPoint = !(g.flags[nextpos] & TTF_ON_CURVE_POINT);
			PathPoint curpoint(g.points[pos].x, g.points[pos].y);
			if (isControlPoint)
			{
				PathPoint nextpoint(g.points[nextpos].x, g.points[nextpos].y);
				if (nextIsControlPoint)
				{
					PathPoint midpoint = (curpoint + nextpoint) / 2;
					path.BezierTo(curpoint * scale, midpoint * scale);
					prevIsControlPoint = isControlPoint;
					pos++;
				}
				else
				{
					path.BezierTo(curpoint * scale, nextpoint * scale);
					prevIsControlPoint = false;
					pos += 2;
				}
			}
			else
			{
				if (!prevIsControlPoint)
					path.LineTo(curpoint * scale);
				prevIsControlPoint = isControlPoint;
				pos++;
			}
		}
		path.Close();

		startPoint = endPoint + 1;
	}

	// Transform and find the final bounding box
	PathPoint bboxMin, bboxMax;
	if (!path.subpaths.front().points.empty())
	{
		bboxMin = path.subpaths.front().points.front();
		bboxMax = path.subpaths.front().points.front();
		bboxMin.x *= scaleX;
		bboxMin.y *= scaleY;
		bboxMax.x *= scaleX;
		bboxMax.y *= scaleY;
		for (auto& subpath : path.subpaths)
		{
			for (auto& point : subpath.points)
			{
				point.x *= scaleX;
				point.y *= scaleY;
				bboxMin.x = std::min(bboxMin.x, point.x);
				bboxMin.y = std::min(bboxMin.y, point.y);
				bboxMax.x = std::max(bboxMax.x, point.x);
				bboxMax.y = std::max(bboxMax.y, point.y);
			}
		}
	}

	bboxMin.x = std::floor(bboxMin.x);
	bboxMin.y = std::floor(bboxMin.y);

	// Reposition glyph to bitmap so it begins at (0,0) for our bitmap
	for (auto& subpath : path.subpaths)
	{
		for (auto& point : subpath.points)
		{
			point.x -= bboxMin.x;
			point.y -= bboxMin.y;
		}
	}

#ifdef DUMP_GLYPH
	std::string svgxmlstart = R"(<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg width="1000px" height="1000px" viewBox="0 0 25 25" xmlns="http://www.w3.org/2000/svg" version="1.1">
<path fill-rule="evenodd" d=")";
	std::string svgxmlend = R"(" fill="red" />
</svg>)";

	std::ofstream out("c:\\development\\glyph.svg");
	out << svgxmlstart;

	for (auto& subpath : path.subpaths)
	{
		size_t pos = 0;
		out << "M" << subpath.points[pos].x << " " << subpath.points[pos].y << " ";
		pos++;
		for (PathFillCommand cmd : subpath.commands)
		{
			int count = 0;
			if (cmd == PathFillCommand::line)
			{
				out << "L";
				count = 1;
			}
			else if (cmd == PathFillCommand::quadradic)
			{
				out << "Q";
				count = 2;
			}
			else if (cmd == PathFillCommand::cubic)
			{
				out << "C";
				count = 3;
			}

			for (int i = 0; i < count; i++)
			{
				out << subpath.points[pos].x << " " << subpath.points[pos].y << " ";
				pos++;
			}
		}
		if (subpath.closed)
			out << "Z";
	}

	out << svgxmlend;
	out.close();
#endif

	TrueTypeGlyph glyph;

	// Rasterize the glyph
	glyph.width = (int)std::floor(bboxMax.x - bboxMin.x) + 1;
	glyph.height = (int)std::floor(bboxMax.y - bboxMin.y) + 1;
	glyph.grayscale.reset(new uint8_t[glyph.width * glyph.height]);
	uint8_t* grayscale = glyph.grayscale.get();
	path.Rasterize(grayscale, glyph.width, glyph.height);

	// TBD: gridfit or not?
	glyph.advanceWidth = (int)std::round(advanceWidth * scale * scaleX);
	glyph.leftSideBearing = (int)std::round(bboxMin.x);
	glyph.yOffset = (int)std::round(bboxMin.y);

	return glyph;
}

void TrueTypeFont::LoadGlyph(TTF_SimpleGlyph& g, uint32_t glyphIndex, int compositeDepth) const
{
	if (glyphIndex >= loca.offsets.size())
		throw std::runtime_error("Glyph index out of bounds");

	TrueTypeFileReader reader = glyf.GetReader(data->data(), data->size());
	reader.Seek(loca.offsets[glyphIndex]);

	ttf_int16 numberOfContours = reader.ReadInt16();
	/*ttf_int16 xMin = */reader.ReadInt16();
	/*ttf_int16 yMin = */reader.ReadInt16();
	/*ttf_int16 xMax = */reader.ReadInt16();
	/*ttf_int16 yMax = */reader.ReadInt16();

	if (numberOfContours > 0) // Simple glyph
	{
		int pointsOffset = (int)g.points.size();
		for (ttf_uint16 i = 0; i < numberOfContours; i++)
			g.endPtsOfContours.push_back(pointsOffset + reader.ReadUInt16());

		ttf_uint16 instructionLength = reader.ReadUInt16();
		std::vector<ttf_uint8> instructions;
		instructions.resize(instructionLength);
		reader.Read(instructions.data(), instructions.size());

		int numPoints = (int)g.endPtsOfContours.back() - pointsOffset + 1;
		g.points.resize(pointsOffset + numPoints);

		while (g.flags.size() < g.points.size())
		{
			ttf_uint8 flag = reader.ReadUInt8();
			if (flag & TTF_REPEAT_FLAG)
			{
				ttf_uint8 repeatcount = reader.ReadUInt8();
				for (ttf_uint8 i = 0; i < repeatcount; i++)
					g.flags.push_back(flag);
			}
			g.flags.push_back(flag);
		}

		for (int i = pointsOffset; i < pointsOffset + numPoints; i++)
		{
			if (g.flags[i] & TTF_X_SHORT_VECTOR)
			{
				ttf_int16 x = reader.ReadUInt8();
				g.points[i].x = (float)((g.flags[i] & TTF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR) ? x : -x);
			}
			else if (g.flags[i] & TTF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
			{
				g.points[i].x = 0.0f;
			}
			else
			{
				g.points[i].x = (float)reader.ReadInt16();
			}
		}

		for (int i = pointsOffset; i < pointsOffset + numPoints; i++)
		{
			if (g.flags[i] & TTF_Y_SHORT_VECTOR)
			{
				ttf_int16 y = reader.ReadUInt8();
				g.points[i].y = (float)((g.flags[i] & TTF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR) ? y : -y);
			}
			else if (g.flags[i] & TTF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)
			{
				g.points[i].y = 0.0f;
			}
			else
			{
				g.points[i].y = (float)reader.ReadInt16();
			}
		}

		// Convert from relative coordinates to absolute
		for (int i = pointsOffset + 1; i < pointsOffset + numPoints; i++)
		{
			g.points[i].x += g.points[i - 1].x;
			g.points[i].y += g.points[i - 1].y;
		}
	}
	else if (numberOfContours < 0) // Composite glyph
	{
		if (compositeDepth == 8)
			throw std::runtime_error("Composite glyph recursion exceeded");

		int parentPointsOffset = (int)g.points.size();

		bool weHaveInstructions = false;
		while (true)
		{
			ttf_uint16 flags = reader.ReadUInt16();
			ttf_uint16 childGlyphIndex = reader.ReadUInt16();

			int argument1, argument2;
			if (flags & TTF_ARG_1_AND_2_ARE_WORDS)
			{
				if (flags & TTF_ARGS_ARE_XY_VALUES)
				{
					argument1 = reader.ReadInt16();
					argument2 = reader.ReadInt16();
				}
				else
				{
					argument1 = reader.ReadUInt16();
					argument2 = reader.ReadUInt16();
				}
			}
			else
			{
				if (flags & TTF_ARGS_ARE_XY_VALUES)
				{
					argument1 = reader.ReadInt8();
					argument2 = reader.ReadInt8();
				}
				else
				{
					argument1 = reader.ReadUInt8();
					argument2 = reader.ReadUInt8();
				}
			}

			float mat2x2[4];
			bool transform = true;
			if (flags & TTF_WE_HAVE_A_SCALE)
			{
				float scale = F2DOT14_ToFloat(reader.ReadF2DOT14());
				mat2x2[0] = scale;
				mat2x2[1] = 0;
				mat2x2[2] = 0;
				mat2x2[3] = scale;
			}
			else if (flags & TTF_WE_HAVE_AN_X_AND_Y_SCALE)
			{
				mat2x2[0] = F2DOT14_ToFloat(reader.ReadF2DOT14());
				mat2x2[1] = 0;
				mat2x2[2] = 0;
				mat2x2[3] = F2DOT14_ToFloat(reader.ReadF2DOT14());
			}
			else if (flags & TTF_WE_HAVE_A_TWO_BY_TWO)
			{
				mat2x2[0] = F2DOT14_ToFloat(reader.ReadF2DOT14());
				mat2x2[1] = F2DOT14_ToFloat(reader.ReadF2DOT14());
				mat2x2[2] = F2DOT14_ToFloat(reader.ReadF2DOT14());
				mat2x2[3] = F2DOT14_ToFloat(reader.ReadF2DOT14());
			}
			else
			{
				transform = false;
			}

			int childPointsOffset = (int)g.points.size();
			LoadGlyph(g, childGlyphIndex, compositeDepth + 1);

			if (transform)
			{
				for (int i = childPointsOffset; (size_t)i < g.points.size(); i++)
				{
					float x = g.points[i].x * mat2x2[0] + g.points[i].y * mat2x2[1];
					float y = g.points[i].x * mat2x2[2] + g.points[i].y * mat2x2[3];
					g.points[i].x = x;
					g.points[i].y = y;
				}
			}

			float dx, dy;

			if (flags & TTF_ARGS_ARE_XY_VALUES)
			{
				dx = (float)argument1;
				dy = (float)argument2;

				// Spec states we must fall back to TTF_UNSCALED_COMPONENT_OFFSET if both flags are set
				if ((flags & (TTF_SCALED_COMPONENT_OFFSET | TTF_UNSCALED_COMPONENT_OFFSET)) == TTF_SCALED_COMPONENT_OFFSET)
				{
					float x = dx * mat2x2[0] + dy * mat2x2[1];
					float y = dx * mat2x2[2] + dy * mat2x2[3];
					dx = x;
					dy = y;
				}

				if (flags & TTF_ROUND_XY_TO_GRID)
				{
					// To do: round the offset to the pixel grid
				}
			}
			else
			{
				int parentPointIndex = parentPointsOffset + argument1;
				int childPointIndex = childPointsOffset + argument2;

				if ((size_t)parentPointIndex >= g.points.size() || (size_t)childPointIndex >= g.points.size())
					throw std::runtime_error("Invalid glyph offset");

				dx = g.points[parentPointIndex].x - g.points[childPointIndex].x;
				dy = g.points[parentPointIndex].y - g.points[childPointIndex].y;
			}

			for (int i = childPointsOffset; (size_t)i < g.points.size(); i++)
			{
				g.points[i].x += dx;
				g.points[i].y += dy;
			}

			if (flags & TTF_USE_MY_METRICS)
			{
				// To do: this affects lsb + rsb calculations somehow
			}

			if (flags & TTF_WE_HAVE_INSTRUCTIONS)
			{
				weHaveInstructions = true;
			}

			if (!(flags & TTF_MORE_COMPONENTS))
				break;
		}

		if (weHaveInstructions)
		{
			ttf_uint16 instructionLength = reader.ReadUInt16();
			std::vector<ttf_uint8> instructions;
			instructions.resize(instructionLength);
			reader.Read(instructions.data(), instructions.size());
		}
	}
}

float TrueTypeFont::F2DOT14_ToFloat(ttf_F2DOT14 v)
{
	int a = ((ttf_int16)v) >> 14;
	int b = (v & 0x3fff);
	return a + b / (float)0x4000;
}

uint32_t TrueTypeFont::GetGlyphIndex(uint32_t c) const
{
	auto it = std::lower_bound(Ranges.begin(), Ranges.end(), c, [](const TTF_GlyphRange& range, uint32_t c) { return range.endCharCode < c; });
	if (it != Ranges.end() && c >= it->startCharCode && c <= it->endCharCode)
	{
		return it->startGlyphID + (c - it->startCharCode);
	}

	it = std::lower_bound(ManyToOneRanges.begin(), ManyToOneRanges.end(), c, [](const TTF_GlyphRange& range, uint32_t c) { return range.endCharCode < c; });
	if (it != ManyToOneRanges.end() && c >= it->startCharCode && c <= it->endCharCode)
	{
		return it->startGlyphID;
	}
	return 0;
}

void TrueTypeFont::LoadCharacterMapEncoding(TrueTypeFileReader& reader)
{
	// Look for the best encoding available that we support

	TTF_EncodingRecord record;
	if (!record.subtableOffset) record = cmap.GetEncoding(3, 12);
	if (!record.subtableOffset) record = cmap.GetEncoding(0, 4);
	if (!record.subtableOffset) record = cmap.GetEncoding(3, 1);
	if (!record.subtableOffset) record = cmap.GetEncoding(0, 3);
	if (!record.subtableOffset)
		throw std::runtime_error("No supported cmap encoding found in truetype file");

	reader.Seek(record.subtableOffset);

	ttf_uint16 format = reader.ReadUInt16();
	if (format == 4)
	{
		TTF_CMapSubtable4 subformat;
		subformat.Load(reader);

		for (uint16_t i = 0; i < subformat.segCount; i++)
		{
			ttf_uint16 startCode = subformat.startCode[i];
			ttf_uint16 endCode = subformat.endCode[i];
			ttf_uint16 idDelta = subformat.idDelta[i];
			ttf_uint16 idRangeOffset = subformat.idRangeOffsets[i];
			if (idRangeOffset == 0)
			{
				ttf_uint16 glyphId = startCode + idDelta; // Note: relies on modulo 65536

				TTF_GlyphRange range;
				range.startCharCode = startCode;
				range.endCharCode = endCode;
				range.startGlyphID = glyphId;
				Ranges.push_back(range);
			}
			else if (startCode <= endCode)
			{
				TTF_GlyphRange range;
				range.startCharCode = startCode;
				bool firstGlyph = true;
				for (ttf_uint16 c = startCode; c <= endCode; c++)
				{
					int offset = idRangeOffset / 2 + (c - startCode) - ((int)subformat.segCount - i);
					if (offset >= 0 && (size_t)offset < subformat.glyphIdArray.size())
					{
						ttf_uint32 glyphId = subformat.glyphIdArray[offset];
						if (firstGlyph)
						{
							range.startGlyphID = glyphId;
							firstGlyph = false;
						}
						else if (range.startGlyphID + (c - range.startCharCode) != glyphId)
						{
							range.endCharCode = c - 1;
							Ranges.push_back(range);
							range.startCharCode = c;
							range.startGlyphID = glyphId;
						}
					}
				}
				if (!firstGlyph)
				{
					range.endCharCode = endCode;
					Ranges.push_back(range);
				}
			}
		}
	}
	else if (format == 12 || format == 3)
	{
		TTF_CMapSubtable12 subformat;
		subformat.Load(reader);
		Ranges = std::move(subformat.groups);
	}
	else if (format == 13 || format == 3)
	{
		TTF_CMapSubtable13 subformat;
		subformat.Load(reader);
		ManyToOneRanges = std::move(subformat.groups);
	}
}

std::vector<TTCFontName> TrueTypeFont::GetFontNames(const std::shared_ptr<TTFDataBuffer>& data)
{
	if (data->size() > 0x7fffffff)
		throw std::runtime_error("TTF file is larger than 2 gigabytes!");

	TrueTypeFileReader reader(data->data(), data->size());
	ttf_Tag versionTag = reader.ReadTag();
	reader.Seek(0);

	std::vector<TTCFontName> names;
	if (memcmp(versionTag.data(), "ttcf", 4) == 0) // TTC header
	{
		TTC_Header ttcHeader;
		ttcHeader.Load(reader);

		for (size_t i = 0; i < ttcHeader.tableDirectoryOffsets.size(); i++)
		{
			reader.Seek(ttcHeader.tableDirectoryOffsets[i]);

			TTF_TableDirectory directory;
			directory.Load(reader);

			TTF_NamingTable name;
			auto name_reader = directory.GetReader(data->data(), data->size(), "name");
			name.Load(name_reader);

			names.push_back(name.GetFontName());
		}
	}
	else
	{
		TTF_TableDirectory directory;
		directory.Load(reader);

		TTF_NamingTable name;
		auto name_reader = directory.GetReader(data->data(), data->size(), "name");
		name.Load(name_reader);

		names.push_back(name.GetFontName());
	}
	return names;
}

struct TTFSubsetTable
{
	TTFSubsetTable() = default;
	TTFSubsetTable(std::shared_ptr<TTFDataBuffer> data) : table(std::move(data))
	{
		size = table->size();

		// Tables (and checksum) must always be 32bit aligned with padded zeros
		size_t padding = size % 4;
		if (padding > 0)
		{
			padding = 4 - padding;
			table->setSize(size + padding);
			memset(table->data() + size, 0, padding);
		}

		size_t count = table->size();
		const uint8_t* src = table->data<uint8_t>();
		ttf_uint32 sum = 0;
		for (size_t i = 0; i < count; i += 4)
		{
			ttf_uint32 value =
				(static_cast<ttf_uint32>(src[i]) << 24) |
				(static_cast<ttf_uint32>(src[i + 1]) << 16) |
				(static_cast<ttf_uint32>(src[i + 2]) << 8) |
				static_cast<ttf_uint32>(src[i + 3]);
			sum += value;
		}
		checksum = sum;
	}

	ttf_uint32 checksum = 0;
	size_t size = 0;
	std::shared_ptr<TTFDataBuffer> table;
};

std::shared_ptr<TTFDataBuffer> TrueTypeFont::CreatePdfSubsetFont(const std::vector<uint16_t>& glyphs)
{
	// PDF subset tables: head, hhea, loca, maxp, cvt, prep, glyf, hmtx, fpgm

	std::map<std::string, TTFSubsetTable> tables;

	// Font Header Table
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "head");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		dest.Seek(8);
		dest.WriteUInt32(0); // set 'checksumAdjustment' field back to zero
		tables["head"] = TTFSubsetTable(dest.Data());
	}

	// Horizontal Header Table
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "hhea");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		dest.Seek(34);
		dest.WriteUInt16((ttf_uint16)glyphs.size()); // update 'numberOfHMetrics'
		tables["hhea"] = TTFSubsetTable(dest.Data());
	}

	// Maximum Profile
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "maxp");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		dest.Seek(4);
		dest.WriteUInt16((ttf_uint16)glyphs.size()); // update 'numGlyphs'
		tables["maxp"] = TTFSubsetTable(dest.Data());
	}

	// Horizontal Metrics Table
	{
		TrueTypeFileWriter dest;
		for (uint16_t glyphIndex : glyphs)
		{
			TTF_HorizontalMetrics::longHorMetric hm;
			if (glyphIndex >= hhea.numberOfHMetrics)
			{
				hm = hmtx.hMetrics[hhea.numberOfHMetrics - 1];
				hm.lsb = hmtx.leftSideBearings[glyphIndex - hhea.numberOfHMetrics];
			}
			else
			{
				hm = hmtx.hMetrics[glyphIndex];
			}

			dest.WriteUInt16(hm.advanceWidth);
			dest.WriteInt16(hm.lsb);
		}
		tables["hmtx"] = TTFSubsetTable(dest.Data());
	}

	// Glyphs and offsets
	{
		TrueTypeFileReader srcGlyphs = directory.GetReader(data->data(), data->size(), "glyf");
		TrueTypeFileWriter destOffsets, destGlyphs;

		for (uint16_t glyphIndex : glyphs)
		{
			size_t offset = destGlyphs.Position();
			if (head.indexToLocFormat == 0)
				destOffsets.WriteOffset16((ttf_Offset16)(offset / 2));
			else
				destOffsets.WriteOffset32((ttf_Offset32)offset);

			ttf_Offset32 srcOffset = loca.offsets[glyphIndex];
			ttf_Offset32 srcSize = loca.offsets[glyphIndex + 1] - srcOffset;
			srcGlyphs.Seek(srcOffset);
			destGlyphs.Write(srcGlyphs, srcSize);
		}

		size_t offset = destGlyphs.Position();
		if (head.indexToLocFormat == 0)
			destOffsets.WriteOffset16((ttf_Offset16)(offset / 2));
		else
			destOffsets.WriteOffset32((ttf_Offset32)offset);

		tables["loca"] = TTFSubsetTable(destOffsets.Data());
		tables["glyf"] = TTFSubsetTable(destGlyphs.Data());
	}

	// Font program
	if (directory.TableExists("fpgm"))
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "fpgm");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		tables["fpgm"] = TTFSubsetTable(dest.Data());
	}

	// Control Value Program
	if (directory.TableExists("prep"))
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "prep");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		tables["prep"] = TTFSubsetTable(dest.Data());
	}

	// Control Value Table
	if (directory.TableExists("cvt "))
	{
		TrueTypeFileReader src = directory.GetReader(data->data(), data->size(), "cvt ");
		TrueTypeFileWriter dest;
		dest.Write(src, src.Size());
		tables["cvt "] = TTFSubsetTable(dest.Data());
	}

	// Write truetype file:

	ttf_uint32 sfntVersion = 0x00010000;
	ttf_uint16 numTables = static_cast<ttf_uint16>(tables.size());
	ttf_uint16 searchRange = static_cast<ttf_uint16>(static_cast<int>(std::round(std::pow(2.0, std::floor(std::log2(numTables))) * 16.0))); // ((2**floor(log2(numTables))) * 16
	ttf_uint16 entrySelector = static_cast<ttf_uint16>(static_cast<int>(std::floor(std::log2(numTables)))); // floor(log2(numTables))
	ttf_uint16 rangeShift = (numTables * 16) - searchRange;

	TrueTypeFileWriter dest;
	dest.WriteInt32(sfntVersion);
	dest.WriteUInt16(numTables);
	dest.WriteUInt16(searchRange);
	dest.WriteUInt16(entrySelector);
	dest.WriteUInt16(rangeShift);

	ttf_Offset32 checksumAdjustmentOffset = 0;
	ttf_Offset32 offset = 12 + numTables * 16;
	for (auto& it : tables)
	{
		if (it.first == "head")
		{
			checksumAdjustmentOffset = offset + 8;
		}

		dest.Write(it.first.data(), 4);
		dest.WriteUInt32(it.second.checksum);
		dest.WriteOffset32(offset);
		dest.WriteUInt32(static_cast<ttf_uint32>(it.second.size));
		offset += static_cast<ttf_Offset32>(it.second.table->size());
	}

	for (auto& it : tables)
	{
		dest.Write(it.second.table->data(), it.second.table->size());
	}

	// Update 'checksumAdjustment' in the 'head' table

	size_t count = dest.Data()->size();
	const uint8_t* src = dest.Data()->data<uint8_t>();
	ttf_uint32 sum = 0;
	for (size_t i = 0; i < count; i += 4)
	{
		ttf_uint32 value =
			(static_cast<ttf_uint32>(src[i]) << 24) |
			(static_cast<ttf_uint32>(src[i + 1]) << 16) |
			(static_cast<ttf_uint32>(src[i + 2]) << 8) |
			static_cast<ttf_uint32>(src[i + 3]);
		sum += value;
	}
	ttf_uint32 checksumAdjustment = 0xb1b0afba - sum;

	dest.Seek(checksumAdjustmentOffset);
	dest.WriteUInt32(checksumAdjustment);

	return dest.Data();
}

/////////////////////////////////////////////////////////////////////////////

void TTF_CMapSubtable0::Load(TrueTypeFileReader& reader)
{
	length = reader.ReadUInt16();
	language = reader.ReadUInt16();
	glyphIdArray.resize(256);
	reader.Read(glyphIdArray.data(), glyphIdArray.size());
}

/////////////////////////////////////////////////////////////////////////////

void TTF_CMapSubtable4::Load(TrueTypeFileReader& reader)
{
	length = reader.ReadUInt16();
	language = reader.ReadUInt16();

	segCount = reader.ReadUInt16() / 2;
	/*ttf_uint16 searchRange = */reader.ReadUInt16();
	/*ttf_uint16 entrySelector = */reader.ReadUInt16();
	/*ttf_uint16 rangeShift = */reader.ReadUInt16();

	endCode.reserve(segCount);
	startCode.reserve(segCount);
	idDelta.reserve(segCount);
	idRangeOffsets.reserve(segCount);
	for (ttf_uint16 i = 0; i < segCount; i++) endCode.push_back(reader.ReadUInt16());
	reservedPad = reader.ReadUInt16();
	for (ttf_uint16 i = 0; i < segCount; i++) startCode.push_back(reader.ReadUInt16());
	for (ttf_uint16 i = 0; i < segCount; i++) idDelta.push_back(reader.ReadInt16());
	for (ttf_uint16 i = 0; i < segCount; i++) idRangeOffsets.push_back(reader.ReadUInt16());

	int glyphIdArraySize = ((int)length - (8 + (int)segCount * 4) * sizeof(ttf_uint16)) / 2;
	if (glyphIdArraySize < 0)
		throw std::runtime_error("Invalid TTF cmap subtable 4 length");
	glyphIdArray.reserve(glyphIdArraySize);
	for (int i = 0; i < glyphIdArraySize; i++) glyphIdArray.push_back(reader.ReadUInt16());
}

/////////////////////////////////////////////////////////////////////////////

void TTF_CMapSubtable12::Load(TrueTypeFileReader& reader)
{
	reserved = reader.ReadUInt16();
	length = reader.ReadUInt32();
	language = reader.ReadUInt32();
	numGroups = reader.ReadUInt32();
	for (ttf_uint32 i = 0; i < numGroups; i++)
	{
		TTF_GlyphRange range;
		range.startCharCode = reader.ReadUInt32();
		range.endCharCode = reader.ReadUInt32();
		range.startGlyphID = reader.ReadUInt32();
		groups.push_back(range);
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTF_TableRecord::Load(TrueTypeFileReader& reader)
{
	tableTag = reader.ReadTag();
	checksum = reader.ReadUInt32();
	offset = reader.ReadOffset32();
	length = reader.ReadUInt32();
}

/////////////////////////////////////////////////////////////////////////////

void TTF_TableDirectory::Load(TrueTypeFileReader& reader)
{
	sfntVersion = reader.ReadUInt32();
	numTables = reader.ReadUInt16();

	// opentype spec says we can't use these for security reasons, so we pretend they never was part of the header
	ttf_uint16 searchRange = reader.ReadUInt16();
	ttf_uint16 entrySelector = reader.ReadUInt16();
	ttf_uint16 rangeShift = reader.ReadUInt16();

	for (ttf_uint16 i = 0; i < numTables; i++)
	{
		TTF_TableRecord record;
		record.Load(reader);
		tableRecords.push_back(record);
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTC_Header::Load(TrueTypeFileReader& reader)
{
	ttcTag = reader.ReadTag();
	majorVersion = reader.ReadUInt16();
	minorVersion = reader.ReadUInt16();

	if (!(majorVersion == 1 && minorVersion == 0) && !(majorVersion == 2 && minorVersion == 0))
		throw std::runtime_error("Unsupported TTC header version");

	numFonts = reader.ReadUInt32();
	for (ttf_uint16 i = 0; i < numFonts; i++)
	{
		tableDirectoryOffsets.push_back(reader.ReadOffset32());
	}

	if (majorVersion == 2 && minorVersion == 0)
	{
		dsigTag = reader.ReadUInt32();
		dsigLength = reader.ReadUInt32();
		dsigOffset = reader.ReadUInt32();
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTF_CMap::Load(TrueTypeFileReader& reader)
{
	version = reader.ReadUInt16();
	numTables = reader.ReadUInt16();

	for (ttf_uint16 i = 0; i < numTables; i++)
	{
		TTF_EncodingRecord record;
		record.platformID = reader.ReadUInt16();
		record.encodingID = reader.ReadUInt16();
		record.subtableOffset = reader.ReadOffset32();
		encodingRecords.push_back(record);
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTF_FontHeader::Load(TrueTypeFileReader& reader)
{
	majorVersion = reader.ReadUInt16();
	minorVersion = reader.ReadUInt16();
	fontRevision = reader.ReadFixed();
	checksumAdjustment = reader.ReadUInt32();
	magicNumber = reader.ReadUInt32();
	flags = reader.ReadUInt16();
	unitsPerEm = reader.ReadUInt16();
	created = reader.ReadLONGDATETIME();
	modified = reader.ReadLONGDATETIME();
	xMin = reader.ReadInt16();
	yMin = reader.ReadInt16();
	xMax = reader.ReadInt16();
	yMax = reader.ReadInt16();
	macStyle = reader.ReadUInt16();
	lowestRecPPEM = reader.ReadUInt16();
	fontDirectionHint = reader.ReadInt16();
	indexToLocFormat = reader.ReadInt16();
	glyphDataFormat = reader.ReadInt16();
}

/////////////////////////////////////////////////////////////////////////////

void TTF_HorizontalHeader::Load(TrueTypeFileReader& reader)
{
	majorVersion = reader.ReadUInt16();
	minorVersion = reader.ReadUInt16();
	ascender = reader.ReadFWORD();
	descender = reader.ReadFWORD();
	lineGap = reader.ReadFWORD();
	advanceWidthMax = reader.ReadUFWORD();
	minLeftSideBearing = reader.ReadFWORD();
	minRightSideBearing = reader.ReadFWORD();
	xMaxExtent = reader.ReadFWORD();
	caretSlopeRise = reader.ReadInt16();
	caretSlopeRun = reader.ReadInt16();
	caretOffset = reader.ReadInt16();
	reserved0 = reader.ReadInt16();
	reserved1 = reader.ReadInt16();
	reserved2 = reader.ReadInt16();
	reserved3 = reader.ReadInt16();
	metricDataFormat = reader.ReadInt16();
	numberOfHMetrics = reader.ReadUInt16();
}

/////////////////////////////////////////////////////////////////////////////

void TTF_HorizontalMetrics::Load(const TTF_HorizontalHeader& hhea, const TTF_MaximumProfile& maxp, TrueTypeFileReader& reader)
{
	for (ttf_uint16 i = 0; i < hhea.numberOfHMetrics; i++)
	{
		longHorMetric metric;
		metric.advanceWidth = reader.ReadUInt16();
		metric.lsb = reader.ReadInt16();
		hMetrics.push_back(metric);
	}

	int count = (int)maxp.numGlyphs - (int)hhea.numberOfHMetrics;
	if (count < 0)
		throw std::runtime_error("Invalid TTF file");

	for (int i = 0; i < count; i++)
	{
		leftSideBearings.push_back(reader.ReadInt16());
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTF_MaximumProfile::Load(TrueTypeFileReader& reader)
{
	version = reader.ReadVersion16Dot16();
	numGlyphs = reader.ReadUInt16();

	if (version == (1 << 16)) // v1 only
	{
		maxPoints = reader.ReadUInt16();
		maxContours = reader.ReadUInt16();
		maxCompositePoints = reader.ReadUInt16();
		maxCompositeContours = reader.ReadUInt16();
		maxZones = reader.ReadUInt16();
		maxTwilightPoints = reader.ReadUInt16();
		maxStorage = reader.ReadUInt16();
		maxFunctionDefs = reader.ReadUInt16();
		maxInstructionDefs = reader.ReadUInt16();
		maxStackElements = reader.ReadUInt16();
		maxSizeOfInstructions = reader.ReadUInt16();
		maxComponentElements = reader.ReadUInt16();
		maxComponentDepth = reader.ReadUInt16();
	}
}

/////////////////////////////////////////////////////////////////////////////

static std::string unicode_to_utf8(unsigned int value)
{
	char text[8];

	if ((value < 0x80) && (value > 0))
	{
		text[0] = (char)value;
		text[1] = 0;
	}
	else if (value < 0x800)
	{
		text[0] = (char)(0xc0 | (value >> 6));
		text[1] = (char)(0x80 | (value & 0x3f));
		text[2] = 0;
	}
	else if (value < 0x10000)
	{
		text[0] = (char)(0xe0 | (value >> 12));
		text[1] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[2] = (char)(0x80 | (value & 0x3f));
		text[3] = 0;
	}
	else if (value < 0x200000)
	{
		text[0] = (char)(0xf0 | (value >> 18));
		text[1] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[3] = (char)(0x80 | (value & 0x3f));
		text[4] = 0;
	}
	else if (value < 0x4000000)
	{
		text[0] = (char)(0xf8 | (value >> 24));
		text[1] = (char)(0x80 | ((value >> 18) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[3] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[4] = (char)(0x80 | (value & 0x3f));
		text[5] = 0;
	}
	else if (value < 0x80000000)
	{
		text[0] = (char)(0xfc | (value >> 30));
		text[1] = (char)(0x80 | ((value >> 24) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 18) & 0x3f));
		text[3] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[4] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[5] = (char)(0x80 | (value & 0x3f));
		text[6] = 0;
	}
	else
	{
		text[0] = 0;	// Invalid wchar value
	}
	return text;
}

void TTF_NamingTable::Load(TrueTypeFileReader& reader)
{
	version = reader.ReadUInt16();
	count = reader.ReadUInt16();
	storageOffset = reader.ReadOffset16();
	for (ttf_uint16 i = 0; i < count; i++)
	{
		NameRecord record;
		record.platformID = reader.ReadUInt16();
		record.encodingID = reader.ReadUInt16();
		record.languageID = reader.ReadUInt16();
		record.nameID = reader.ReadUInt16();
		record.length = reader.ReadUInt16();
		record.stringOffset = reader.ReadOffset16();
		nameRecord.push_back(record);
	}

	if (version == 1)
	{
		langTagCount = reader.ReadUInt16();
		for (ttf_uint16 i = 0; i < langTagCount; i++)
		{
			LangTagRecord record;
			record.length = reader.ReadUInt16();
			record.langTagOffset = reader.ReadOffset16();
			langTagRecord.push_back(record);
		}
	}

	for (NameRecord& record : nameRecord)
	{
		if (record.length > 0 && record.platformID == 3 && record.encodingID == 1)
		{
			reader.Seek(storageOffset + record.stringOffset);
			ttf_uint16 ucs2len = record.length / 2;
			for (ttf_uint16 i = 0; i < ucs2len; i++)
			{
				record.text += unicode_to_utf8(reader.ReadUInt16());
			}
		}
	}
}

TTCFontName TTF_NamingTable::GetFontName() const
{
	TTCFontName fname;
	for (const auto& record : nameRecord)
	{
		if (record.nameID == 1) fname.FamilyName = record.text;
		if (record.nameID == 2) fname.SubfamilyName = record.text;
		if (record.nameID == 3) fname.UniqueID = record.text;
		if (record.nameID == 4) fname.FullName = record.text;
		if (record.nameID == 5) fname.VersionString = record.text;
		if (record.nameID == 6) fname.PostscriptName = record.text;
	}
	return fname;
}

/////////////////////////////////////////////////////////////////////////////

void TTF_OS2Windows::Load(TrueTypeFileReader& reader)
{
	version = reader.ReadUInt16();
	xAvgCharWidth = reader.ReadInt16();
	usWeightClass = reader.ReadUInt16();
	usWidthClass = reader.ReadUInt16();
	fsType = reader.ReadUInt16();
	ySubscriptXSize = reader.ReadInt16();
	ySubscriptYSize = reader.ReadInt16();
	ySubscriptXOffset = reader.ReadInt16();
	ySubscriptYOffset = reader.ReadInt16();
	ySuperscriptXSize = reader.ReadInt16();
	ySuperscriptYSize = reader.ReadInt16();
	ySuperscriptXOffset = reader.ReadInt16();
	ySuperscriptYOffset = reader.ReadInt16();
	yStrikeoutSize = reader.ReadInt16();
	yStrikeoutPosition = reader.ReadInt16();
	sFamilyClass = reader.ReadInt16();
	for (int i = 0; i < 10; i++)
	{
		panose[i] = reader.ReadUInt8();
	}
	ulUnicodeRange1 = reader.ReadUInt32();
	ulUnicodeRange2 = reader.ReadUInt32();
	ulUnicodeRange3 = reader.ReadUInt32();
	ulUnicodeRange4 = reader.ReadUInt32();
	achVendID = reader.ReadTag();
	fsSelection = reader.ReadUInt16();
	usFirstCharIndex = reader.ReadUInt16();
	usLastCharIndex = reader.ReadUInt16();
	sTypoAscender = reader.ReadInt16();
	sTypoDescender = reader.ReadInt16();
	sTypoLineGap = reader.ReadInt16();
	if (!reader.IsEndOfData()) // may be missing in v0 due to a bug in Apple's TTF documentation
	{
		usWinAscent = reader.ReadUInt16();
		usWinDescent = reader.ReadUInt16();
	}
	if (version >= 1)
	{
		ulCodePageRange1 = reader.ReadUInt32();
		ulCodePageRange2 = reader.ReadUInt32();
	}
	if (version >= 2)
	{
		sxHeight = reader.ReadInt16();
		sCapHeight = reader.ReadInt16();
		usDefaultChar = reader.ReadUInt16();
		usBreakChar = reader.ReadUInt16();
		usMaxContext = reader.ReadUInt16();
	}
	if (version >= 5)
	{
		usLowerOpticalPointSize = reader.ReadUInt16();
		usUpperOpticalPointSize = reader.ReadUInt16();
	}
}

/////////////////////////////////////////////////////////////////////////////

void TTF_IndexToLocation::Load(const TTF_FontHeader& head, const TTF_MaximumProfile& maxp, TrueTypeFileReader& reader)
{
	int count = (int)maxp.numGlyphs + 1;
	if (head.indexToLocFormat == 0)
	{
		offsets.reserve(count);
		for (int i = 0; i < count; i++)
		{
			offsets.push_back((ttf_Offset32)reader.ReadOffset16() * 2);
		}
	}
	else
	{
		offsets.reserve(count);
		for (int i = 0; i < count; i++)
		{
			offsets.push_back(reader.ReadOffset32());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

ttf_uint8 TrueTypeFileReader::ReadUInt8()
{
	ttf_uint8 v; Read(&v, 1); return v;
}

ttf_uint16 TrueTypeFileReader::ReadUInt16()
{
	ttf_uint8 v[2]; Read(v, 2); return (((ttf_uint16)v[0]) << 8) | (ttf_uint16)v[1];
}

ttf_uint24 TrueTypeFileReader::ReadUInt24()
{
	ttf_uint8 v[3]; Read(v, 3); return (((ttf_uint32)v[0]) << 16) | (((ttf_uint32)v[1]) << 8) | (ttf_uint32)v[2];
}

ttf_uint32 TrueTypeFileReader::ReadUInt32()
{
	ttf_uint8 v[4]; Read(v, 4); return (((ttf_uint32)v[0]) << 24) | (((ttf_uint32)v[1]) << 16) | (((ttf_uint32)v[2]) << 8) | (ttf_uint32)v[3];
}

ttf_int8 TrueTypeFileReader::ReadInt8()
{
	return ReadUInt8();
}

ttf_int16 TrueTypeFileReader::ReadInt16()
{
	return ReadUInt16();
}

ttf_int32 TrueTypeFileReader::ReadInt32()
{
	return ReadUInt32();
}

ttf_Fixed TrueTypeFileReader::ReadFixed()
{
	return ReadUInt32();
}

ttf_UFWORD TrueTypeFileReader::ReadUFWORD()
{
	return ReadUInt16();
}

ttf_FWORD TrueTypeFileReader::ReadFWORD()
{
	return ReadUInt16();
}

ttf_F2DOT14 TrueTypeFileReader::ReadF2DOT14()
{
	return ReadUInt16();
}

ttf_LONGDATETIME TrueTypeFileReader::ReadLONGDATETIME()
{
	ttf_uint8 v[8]; Read(v, 8);
	return
		(((ttf_LONGDATETIME)v[0]) << 56) | (((ttf_LONGDATETIME)v[1]) << 48) | (((ttf_LONGDATETIME)v[2]) << 40) | (((ttf_LONGDATETIME)v[3]) << 32) |
		(((ttf_LONGDATETIME)v[4]) << 24) | (((ttf_LONGDATETIME)v[5]) << 16) | (((ttf_LONGDATETIME)v[6]) << 8) | (ttf_LONGDATETIME)v[7];
}

ttf_Tag TrueTypeFileReader::ReadTag()
{
	ttf_Tag v; Read(v.data(), v.size()); return v;
}

ttf_Offset16 TrueTypeFileReader::ReadOffset16()
{
	return ReadUInt16();
}

ttf_Offset24 TrueTypeFileReader::ReadOffset24()
{
	return ReadUInt24();
}

ttf_Offset32 TrueTypeFileReader::ReadOffset32()
{
	return ReadUInt32();
}

ttf_Version16Dot16 TrueTypeFileReader::ReadVersion16Dot16()
{
	return ReadUInt32();
}

void TrueTypeFileReader::Seek(size_t newpos)
{
	if (newpos > size)
		throw std::runtime_error("Invalid TTF file");

	pos = newpos;
}

void TrueTypeFileReader::Read(void* output, size_t count)
{
	if (pos + count > size)
		throw std::runtime_error("Unexpected end of TTF file");
	memcpy(output, data + pos, count);
	pos += count;
}

std::vector<CFFObjectData> TrueTypeFileReader::ReadIndex()
{
	int count = ReadCard16();
	if (count == 0)
		return {};

	uint8_t offsetSize = ReadOffsetSize();
	size_t firstOffset = ReadOffset(offsetSize);
	size_t baseOffset = Position() + offsetSize * count - 1;

	std::vector<CFFObjectData> objects(count);
	size_t offset = firstOffset;
	for (CFFObjectData& obj : objects)
	{
		size_t nextOffset = ReadOffset(offsetSize);
		obj.Offset = baseOffset + offset;
		obj.Size = nextOffset - offset;
		offset = nextOffset;
	}
	Seek(baseOffset + offset);
	return objects;
}

uint8_t TrueTypeFileReader::ReadOffsetSize()
{
	uint8_t offsetSize = ReadCard8();
	if (offsetSize >= 1 && offsetSize <= 4)
		return offsetSize;
	throw std::runtime_error("Invalid CFF offset size");
}

uint32_t TrueTypeFileReader::ReadOffset(uint8_t offsetSize)
{
	switch (offsetSize)
	{
	case 1: return ReadCard8();
	case 2: return ReadCard16();
	case 3: return ReadCard24();
	case 4: return ReadCard32();
	default: throw std::runtime_error("Invalid CFF offset size");
	}
}

int TrueTypeFileReader::ReadOperator(uint8_t b0)
{
	if (b0 <= 21 && b0 != 12)
		return b0;
	else if (b0 == 12) // Two-byte operator
		return 1200 + (int)ReadCard8();
	else
		throw std::runtime_error("Invalid operator");
}

double TrueTypeFileReader::ReadOperand(uint8_t b0)
{
	if (b0 >= 32 && b0 <= 246)
	{
		return (double)((int)b0 - 139);
	}
	else if (b0 >= 247 && b0 <= 250)
	{
		uint8_t b1 = ReadCard8();
		return (double)(((int)b0 - 247) * 256 + (int)b1 + 108);
	}
	else if (b0 >= 251 && b0 <= 254)
	{
		uint8_t b1 = ReadCard8();
		return (double)(-((int)b0 - 251) * 256 - (int)b1 - 108);
	}
	else if (b0 == 28)
	{
		return (double)((int16_t)ReadCard16());
	}
	else if (b0 == 29)
	{
		return (double)((int32_t)ReadCard32());
	}
	else if (b0 == 30)
	{
		static const char* values[15] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "E", "E-", "", "-" };
		std::string number;
		while (true)
		{
			int v = ReadCard8();
			int nibbles[2] = { (v >> 4), (v & 15) };
			for (int i = 0; i < 2; i++)
			{
				if (nibbles[i] != 15)
					number += values[nibbles[i]];
				else
					return std::atof(number.c_str());
			}
		}
	}
	else
	{
		throw std::runtime_error("Invalid operand");
	}
}

/////////////////////////////////////////////////////////////////////////////

TrueTypeFileWriter::TrueTypeFileWriter()
{
	data = TTFDataBuffer::create(0);
}

void TrueTypeFileWriter::Write(TrueTypeFileReader& src, size_t count)
{
	if (src.pos + count > src.size)
		throw std::runtime_error("Unexpected end of TTF file");
	Write(src.data + src.pos, count);
	src.pos += count;
}

void TrueTypeFileWriter::Write(const void* src, size_t count)
{
	if (pos + count > data->capacity())
		data->setCapacity((pos + count) * 2);

	if (pos + count > data->size())
		data->setSize(pos + count);

	memcpy(data->data() + pos, src, count);
	pos += count;
}

void TrueTypeFileWriter::WriteUInt8(ttf_uint8 value)
{
	Write(&value, 1);
}

void TrueTypeFileWriter::WriteUInt16(ttf_uint16 value)
{
	ttf_uint8 v[2];
	v[0] = static_cast<ttf_uint8>(value >> 8);
	v[1] = static_cast<ttf_uint8>(value);
	Write(v, 2);
}

void TrueTypeFileWriter::WriteUInt24(ttf_uint24 value)
{
	ttf_uint8 v[3];
	v[0] = static_cast<ttf_uint8>(value >> 16);
	v[1] = static_cast<ttf_uint8>(value >> 8);
	v[2] = static_cast<ttf_uint8>(value);
	Write(v, 3);
}

void TrueTypeFileWriter::WriteUInt32(ttf_uint32 value)
{
	ttf_uint8 v[4];
	v[0] = static_cast<ttf_uint8>(value >> 24);
	v[1] = static_cast<ttf_uint8>(value >> 16);
	v[2] = static_cast<ttf_uint8>(value >> 8);
	v[3] = static_cast<ttf_uint8>(value);
	Write(v, 4);
}

void TrueTypeFileWriter::WriteInt8(ttf_int8 value)
{
	WriteUInt8(value);
}

void TrueTypeFileWriter::WriteInt16(ttf_int16 value)
{
	WriteUInt16(value);
}

void TrueTypeFileWriter::WriteInt32(ttf_int32 value)
{
	WriteUInt32(value);
}

void TrueTypeFileWriter::WriteFixed(ttf_Fixed value)
{
	WriteUInt32(value);
}

void TrueTypeFileWriter::WriteUFWORD(ttf_UFWORD value)
{
	WriteUInt16(value);
}

void TrueTypeFileWriter::WriteFWORD(ttf_FWORD value)
{
	WriteUInt16(value);
}

void TrueTypeFileWriter::WriteF2DOT14(ttf_F2DOT14 value)
{
	WriteUInt16(value);
}

void TrueTypeFileWriter::WriteLONGDATETIME(ttf_LONGDATETIME value)
{
	ttf_uint8 v[8];
	v[0] = static_cast<ttf_uint8>(value >> 56);
	v[1] = static_cast<ttf_uint8>(value >> 48);
	v[2] = static_cast<ttf_uint8>(value >> 40);
	v[3] = static_cast<ttf_uint8>(value >> 32);
	v[4] = static_cast<ttf_uint8>(value >> 24);
	v[5] = static_cast<ttf_uint8>(value >> 16);
	v[6] = static_cast<ttf_uint8>(value >> 8);
	v[7] = static_cast<ttf_uint8>(value);
	Write(v, 8);
}

void TrueTypeFileWriter::WriteTag(ttf_Tag value)
{
	Write(value.data(), value.size());
}

void TrueTypeFileWriter::WriteOffset16(ttf_Offset16 value)
{
	WriteUInt16(value);
}

void TrueTypeFileWriter::WriteOffset24(ttf_Offset24 value)
{
	WriteUInt24(value);
}

void TrueTypeFileWriter::WriteOffset32(ttf_Offset32 value)
{
	WriteUInt32(value);
}

void TrueTypeFileWriter::WriteVersion16Dot16(ttf_Version16Dot16 value)
{
	WriteUInt32(value);
}

/////////////////////////////////////////////////////////////////////////////

void TTF_CFF::Load(TrueTypeFileReader& reader)
{
	Header.major = reader.ReadCard8();
	Header.minor = reader.ReadCard8();
	if (Header.major != 1)
		throw std::runtime_error("Unsupported CFF version");
	Header.headerSize = reader.ReadCard8();
	if (Header.headerSize < 4)
		throw std::runtime_error("Invalid CFF header size");
	Header.offsetSize = reader.ReadOffsetSize();
	reader.Seek(Header.headerSize);

	NameIndex = reader.ReadIndex();
	TopDictIndex = reader.ReadIndex();
	StringIndex = reader.ReadIndex();
	GlobalSubroutines = reader.ReadIndex();

	if (TopDictIndex.size() != 1)
		throw std::runtime_error("Only one Top dictionary allowed in CFF table");

	TopDict = LoadTopDict(reader, 0);
	PrivateDict = LoadPrivateDict(reader, TopDict.PrivateDict);
	
	if (TopDict.CharStrings != 0)
	{
		reader.Seek(TopDict.CharStrings);
		CharStrings = reader.ReadIndex();
	}

	if (PrivateDict.Subrs != 0)
	{
		reader.Seek(PrivateDict.Subrs);
		LocalSubroutines = reader.ReadIndex();
	}
}

std::string TTF_CFF::GetName(TrueTypeFileReader& reader, int index)
{
	if ((size_t)index >= NameIndex.size())
		throw std::runtime_error("CFF index out of bounds");
	size_t pos = reader.Position();
	reader.Seek(NameIndex[index].Offset);
	std::string name(NameIndex[index].Size, 0);
	reader.Read(name.data(), name.size());
	reader.Seek(pos);
	if (!name.empty() && name.front() == 0)
		return {};
	return name;
}

static const char* CFFStandardStrings[391] =
{
	".notdef", "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright", "parenleft",
	"parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon", "semicolon", "less",
	"equal", "greater", "question", "at", "A", "B", "C", "D", "E", "F",
	"G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "quoteleft", "a", "b", "c", "d",
	"e", "f", "g", "h", "i", "j", "k", "l", "m", "n",
	"o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
	"y", "z", "braceleft", "bar", "braceright", "asciitilde", "exclamdown", "cent", "sterling", "fraction",
	"yen", "florin", "section", "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi",
	"fl", "endash", "dagger", "daggerdbl", "periodcentered", "paragraph", "bullet", "quotesinglbase", "quotedblbase", "quotedblright",
	"guillemotright", "ellipsis", "perthousand", "questiondown", "grave", "acute", "circumflex", "tilde", "macron", "breve",
	"dotaccent", "dieresis", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", "emdash", "AE", "ordfeminine",
	"Lslash", "Oslash", "OE", "ordmasculine", "ae", "dotlessi", "lslash", "oslash", "oe", "germandbls",
	"onesuperior", "logicalnot", "mu", "trademark", "Eth", "onehalf", "plusminus", "Thorn", "onequarter", "divide",
	"brokenbar", "degree", "thorn", "threequarters", "twosuperior", "registered", "minus", "eth", "multiply", "threesuperior",
	"copyright", "Aacute", "Acircumflex", "Adieresis", "Agrave", "Aring", "Atilde", "Ccedilla", "Eacute", "Ecircumflex",
	"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Ntilde", "Oacute", "Ocircumflex", "Odieresis",
	"Ograve", "Otilde", "Scaron", "Uacute", "Ucircumflex", "Udieresis", "Ugrave", "Yacute", "Ydieresis", "Zcaron",
	"aacute", "acircumflex", "adieresis", "agrave", "aring", "atilde", "ccedilla", "eacute", "ecircumflex", "edieresis",
	"egrave", "iacute", "icircumflex", "idieresis", "igrave", "ntilde", "oacute", "ocircumflex", "odieresis", "ograve",
	"otilde", "scaron", "uacute", "ucircumflex", "udieresis", "ugrave", "yacute", "ydieresis", "zcaron", "exclamsmall",
	"Hungarumlautsmall", "dollaroldstyle", "dollarsuperior", "ampersandsmall", "Acutesmall", "parenleftsuperior", "parenrightsuperior", "twodotenleader", "onedotenleader", "zerooldstyle",
	"oneoldstyle", "twooldstyle", "threeoldstyle", "fouroldstyle", "fiveoldstyle", "sixoldstyle", "sevenoldstyle", "eightoldstyle", "nineoldstyle", "commasuperior",
	"threequartersemdash", "periodsuperior", "questionsmall", "asuperior", "bsuperior", "centsuperior", "dsuperior", "esuperior", "isuperior", "lsuperior",
	"msuperior", "nsuperior", "osuperior", "rsuperior", "ssuperior", "tsuperior", "ff", "ffi", "ffl", "parenleftinferior",
	"parenrightinferior", "Circumflexsmall", "hyphensuperior", "Gravesmall", "Asmall", "Bsmall", "Csmall", "Dsmall", "Esmall", "Fsmall",
	"Gsmall", "Hsmall", "Ismall", "Jsmall", "Ksmall", "Lsmall", "Msmall", "Nsmall", "Osmall", "Psmall",
	"Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall", "Vsmall", "Wsmall", "Xsmall", "Ysmall", "Zsmall",
	"colonmonetary", "onefitted", "rupiah", "Tildesmall", "exclamdownsmall", "centoldstyle", "Lslashsmall", "Scaronsmall", "Zcaronsmall", "Dieresissmall",
	"Brevesmall", "Caronsmall", "Dotaccentsmall", "Macronsmall", "figuredash", "hypheninferior", "Ogoneksmall", "Ringsmall", "Cedillasmall", "questiondownsmall",
	"oneeighth", "threeeighths", "fiveeighths", "seveneighths", "onethird", "twothirds", "zerosuperior", "foursuperior", "fivesuperior", "sixsuperior",
	"sevensuperior", "eightsuperior", "ninesuperior", "zeroinferior", "oneinferior", "twoinferior", "threeinferior", "fourinferior", "fiveinferior", "sixinferior",
	"seveninferior", "eightinferior", "nineinferior", "centinferior", "dollarinferior", "periodinferior", "commainferior", "Agravesmall", "Aacutesmall", "Acircumflexsmall",
	"Atildesmall", "Adieresissmall", "Aringsmall", "AEsmall", "Ccedillasmall", "Egravesmall", "Eacutesmall", "Ecircumflexsmall", "Edieresissmall", "Igravesmall",
	"Iacutesmall", "Icircumflexsmall", "Idieresissmall", "Ethsmall", "Ntildesmall", "Ogravesmall", "Oacutesmall", "Ocircumflexsmall", "Otildesmall", "Odieresissmall",
	"OEsmall", "Oslashsmall", "Ugravesmall", "Uacutesmall", "Ucircumflexsmall", "Udieresissmall", "Yacutesmall", "Thornsmall", "Ydieresissmall", "001.000",
	"001.001", "001.002", "001.003", "Black", "Bold", "Book", "Light", "Medium", "Regular", "Roman",
	"Semibold"
};

std::string TTF_CFF::GetString(TrueTypeFileReader& reader, int index)
{
	if (index < 391)
		return CFFStandardStrings[index];
	index -= 391;

	if ((size_t)index >= StringIndex.size())
		throw std::runtime_error("CFF index out of bounds");
	size_t pos = reader.Position();
	reader.Seek(StringIndex[index].Offset);
	std::string str(StringIndex[index].Size, 0);
	reader.Read(str.data(), str.size());
	reader.Seek(pos);
	return str;
}

CFFTopDict TTF_CFF::LoadTopDict(TrueTypeFileReader& reader, int index)
{
	if ((size_t)index >= TopDictIndex.size())
		throw std::runtime_error("CFF index out of bounds");

	reader.Seek(TopDictIndex[index].Offset);
	size_t endpos = TopDictIndex[index].Offset + TopDictIndex[index].Size;

	std::vector<double> operands;
	operands.reserve(16);

	CFFTopDict topdict;
	while (reader.Position() < endpos)
	{
		uint8_t b0 = reader.ReadCard8();
		if (reader.IsOperand(b0))
		{
			operands.push_back(reader.ReadOperand(b0));
		}
		else
		{
			int oper = reader.ReadOperator(b0);

			switch (oper)
			{
			case 0: topdict.version = GetString(reader, (int)operands[0]); break;
			case 1: topdict.Notice = GetString(reader, (int)operands[0]); break;
			case 1200: topdict.Copyright = GetString(reader, (int)operands[0]); break;
			case 2: topdict.FullName = GetString(reader, (int)operands[0]); break;
			case 3: topdict.FamilyName = GetString(reader, (int)operands[0]); break;
			case 4: topdict.Weight = GetString(reader, (int)operands[0]); break;
			case 1201: topdict.isFixedPitch = (operands[0] == 1.0); break;
			case 1202: topdict.ItalicAngle = operands[0]; break;
			case 1203: topdict.UnderlinePosition = operands[0]; break;
			case 1204: topdict.UnderlineThickness = operands[0]; break;
			case 1205: topdict.PaintType = operands[0]; break;
			case 1206: topdict.CharstringType = operands[0]; break;
			case 1207:
				for (int i = 0; i < 6; i++)
					topdict.FontMatrix[i] = operands[i];
				break;
			case 13: topdict.UniqueID = operands[0]; break;
			case 5:
				for (int i = 0; i < 4; i++)
					topdict.FontBBox[i] = operands[i];
				break;
			case 1208: topdict.StrokeWidth = operands[0]; break;
			case 14: topdict.XUID = operands; break;
			case 15: topdict.charset = (size_t)(int)operands[0]; break;
			case 16: topdict.Encoding = (size_t)(int)operands[0]; break;
			case 17: topdict.CharStrings = (size_t)(int)operands[0]; break;
			case 18:
				topdict.PrivateDict.Size = (size_t)(int)operands[0];
				topdict.PrivateDict.Offset = (size_t)(int)operands[1];
				break;
			case 1220: topdict.SyntheticBase = operands[0]; break;
			case 1221: topdict.Postscript = GetString(reader, (int)operands[0]); break;
			case 1222: topdict.BaseFontName = GetString(reader, (int)operands[0]); break;
			case 1223: topdict.BaseFontBlend = operands; break;
			case 1230:
				topdict.Registry = GetString(reader, (int)operands[0]);
				topdict.Ordering = GetString(reader, (int)operands[1]);
				topdict.Supplement = operands[2];
				break;
			case 1231: topdict.CIDFontVersion = operands[0]; break;
			case 1232: topdict.CIDFontRevision = operands[0]; break;
			case 1233: topdict.CIDFontType = operands[0]; break;
			case 1234: topdict.CIDCount = operands[0]; break;
			case 1235: topdict.UIDBase = operands[0]; break;
			case 1236: topdict.FDArray = (size_t)(int)operands[0]; break;
			case 1237: topdict.FDSelect = (size_t)(int)operands[0]; break;
			case 1238: topdict.FontName = GetString(reader, (int)operands[0]); break;
			default:
				throw std::runtime_error("Unknown CFF topdict operand");
			}

			operands.clear();
		}
	}
	return topdict;
}

CFFPrivateDict TTF_CFF::LoadPrivateDict(TrueTypeFileReader& reader, CFFObjectData obj)
{
	reader.Seek(obj.Offset);
	size_t endpos = obj.Offset + obj.Size;

	std::vector<double> operands;
	operands.reserve(16);

	CFFPrivateDict privdict;
	while (reader.Position() < endpos)
	{
		uint8_t b0 = reader.ReadCard8();
		if (reader.IsOperand(b0))
		{
			operands.push_back(reader.ReadOperand(b0));
		}
		else
		{
			int oper = reader.ReadOperator(b0);

			switch (oper)
			{
			case 6: privdict.BlueValues = operands; break;
			case 7: privdict.OtherValues = operands; break;
			case 8: privdict.FamilyBlues = operands; break;
			case 9: privdict.FamilyOtherBlues = operands; break;
			case 1209: privdict.BlueScale = operands[0]; break;
			case 1210: privdict.BlueShift = operands[0]; break;
			case 1211: privdict.BlueFuzz = operands[0]; break;
			case 10: privdict.StdHW = operands[0]; break;
			case 11: privdict.StdVW = operands[0]; break;
			case 1212: privdict.StemSnapH = operands; break;
			case 1213: privdict.StemSnapV = operands; break;
			case 1214: privdict.ForceBold = (operands[0] == 1.0); break;
			case 1217: privdict.LanguageGroup = operands[0]; break;
			case 1218: privdict.ExpansionFactor = operands[0]; break;
			case 1219: privdict.initialRandomSeed = operands[0]; break;
			case 19: privdict.Subrs = obj.Offset + (size_t)(int)operands[0]; break;
			case 20: privdict.defaultWidthX = operands[0]; break;
			case 21: privdict.norminalWidthX = operands[0]; break;
			default:
				throw std::runtime_error("Unknown CFF topdict operand");
			}

			operands.clear();
		}
	}
	return privdict;
}

class CFFGlyphOperands
{
public:
	CFFGlyphOperands()
	{
		operands.reserve(48);
	}

	double& get(int index)
	{
		if (index < 0)
		{
			if ((size_t)(-index) > operands.size())
				throw std::runtime_error("CFF operands out of bounds");
			return operands[(int)operands.size() + index];
		}
		else
		{
			if ((size_t)index >= operands.size())
				throw std::runtime_error("CFF operands out of bounds");
			return operands[index];
		}
	}

	int size() const
	{
		return (int)operands.size();
	}

	void clear()
	{
		operands.clear();
	}

	void push(double value)
	{
		if (operands.size() == 48)
			throw std::runtime_error("Exceeded CFF operands limit");
		operands.push_back(value);
	}

	void pop(int count)
	{
		if (operands.size() < (size_t)count)
			throw std::runtime_error("CFF operands out of bounds");
		operands.resize(operands.size() - count);
	}

	void pop_front()
	{
		if (operands.empty())
			throw std::runtime_error("CFF operands out of bounds");
		operands.erase(operands.begin());
	}

	typedef std::vector<double>::const_iterator const_iterator;
	typedef std::vector<double>::iterator iterator;
	typedef std::vector<double>::const_reverse_iterator const_reverse_iterator;
	typedef std::vector<double>::reverse_iterator reverse_iterator;

	const_iterator begin() const { return operands.begin(); }
	const_iterator end() const { return operands.end(); }
	iterator begin() { return operands.begin(); }
	iterator end() { return operands.end(); }

	const_reverse_iterator rbegin() const { return operands.rbegin(); }
	const_reverse_iterator rend() const { return operands.rend(); }
	reverse_iterator rbegin() { return operands.rbegin(); }
	reverse_iterator rend() { return operands.rend(); }

private:
	std::vector<double> operands;
};

TrueTypeGlyph TrueTypeFont::LoadCFFGlyph(uint32_t glyphIndex, double height) const
{
	double scale = height / head.unitsPerEm;
	double scaleX = 3.0;
	double scaleY = -1.0;

	ttf_uint16 advanceWidth = 0;
	ttf_int16 lsb = 0;
	if (glyphIndex >= hhea.numberOfHMetrics)
	{
		advanceWidth = hmtx.hMetrics[hhea.numberOfHMetrics - 1].advanceWidth;
		lsb = hmtx.leftSideBearings[glyphIndex - hhea.numberOfHMetrics];
	}
	else
	{
		advanceWidth = hmtx.hMetrics[glyphIndex].advanceWidth;
		lsb = hmtx.hMetrics[glyphIndex].lsb;
	}

	if (glyphIndex >= cff.CharStrings.size())
		throw std::runtime_error("Glyph index out of bounds");

	TrueTypeFileReader reader = cff.Record.GetReader(data->data(), data->size());
	reader.Seek(cff.CharStrings[glyphIndex].Offset);
	size_t endpos = cff.CharStrings[glyphIndex].Offset + cff.CharStrings[glyphIndex].Size;

	// Create glyph path:
	PathFillDesc path;
	path.fill_mode = PathFillMode::winding;

	CFFGlyphOperands operands;
	std::array<double, 32> transient;
	std::vector<std::pair<size_t, size_t>> callstack;
	callstack.reserve(10);
	int hintCount = 0;

	bool endchar = false;
	bool widthArg = true;
	double widthValue = cff.PrivateDict.defaultWidthX;
	PathPoint cur, cp1, cp2, flex1start;
	while (!endchar)
	{
		if (reader.Position() >= endpos)
			throw std::runtime_error("Premature end of CFF charstring");

		uint8_t b0 = reader.ReadCard8();
		if (b0 >= 32)
		{
			if (b0 <= 246)
			{
				operands.push((int)b0 - 139);
			}
			else if (b0 <= 250)
			{
				uint8_t b1 = reader.ReadCard8();
				operands.push(((int)b0 - 247) * 256 + b1 + 108);
			}
			else if (b0 <= 254)
			{
				uint8_t b1 = reader.ReadCard8();
				operands.push(-((int)b0 - 251) * 256 - b1 - 108);
			}
			else
			{
				int32_t fixed16 = reader.ReadCard32();
				operands.push(fixed16 / (double)(1 << 16));
			}
		}
		else if (b0 == 28) // ShortInt
		{
			int16_t value = reader.ReadCard16();
			operands.push(value);
		}
		else
		{
			int oper = b0;
			if (oper == 12)
				oper = 1200 + (int)reader.ReadCard8();

			double tmp, fd;
			switch (oper)
			{
			// Path construction:
			case 21: // rmoveto
				if (widthArg)
				{
					if (operands.size() > 2)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				cur.x += operands.get(0);
				cur.y += operands.get(1);
				path.Close();
				path.MoveTo(cur * scale);
				operands.clear();
				break;
			case 22: // hmoveto
				if (widthArg)
				{
					if (operands.size() > 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				cur.x += operands.get(0);
				path.Close();
				path.MoveTo(cur * scale);
				operands.clear();
				break;
			case 4: // vmoveto
				if (widthArg)
				{
					if (operands.size() > 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				cur.y += operands.get(0);
				path.Close();
				path.MoveTo(cur * scale);
				operands.clear();
				break;
			case 5: // rlineto
				for (int i = 1, count = operands.size(); i < count; i += 2)
				{
					cur.x += operands.get(i - 1);
					cur.y += operands.get(i);
					path.LineTo(cur * scale);
				}
				operands.clear();
				break;
			case 6: // hlineto
				if (operands.size() % 2 == 0)
				{
					for (int i = 0, count = operands.size(); i < count; i++)
					{
						if ((i & 1) == 0)
						{
							cur.x += operands.get(i);
							path.LineTo(cur * scale);
						}
						else
						{
							cur.y += operands.get(i);
							path.LineTo(cur * scale);
						}
					}
				}
				else
				{
					cur.x += operands.get(0);
					path.LineTo(cur* scale);
					for (int i = 1, count = operands.size(); i < count; i++)
					{
						if ((i & 1) == 0)
						{
							cur.x += operands.get(i);
							path.LineTo(cur * scale);
						}
						else
						{
							cur.y += operands.get(i);
							path.LineTo(cur * scale);
						}
					}
				}
				operands.clear();
				break;
			case 7: // vlineto
				if (operands.size() % 2 == 0)
				{
					for (int i = 0, count = operands.size(); i < count; i++)
					{
						if ((i & 1) == 0)
						{
							cur.y += operands.get(i);
							path.LineTo(cur * scale);
						}
						else
						{
							cur.x += operands.get(i);
							path.LineTo(cur * scale);
						}
					}
				}
				else
				{
					cur.y += operands.get(0);
					path.LineTo(cur * scale);
					for (int i = 1, count = operands.size(); i < count; i++)
					{
						if ((i & 1) == 0)
						{
							cur.y += operands.get(i);
							path.LineTo(cur * scale);
						}
						else
						{
							cur.x += operands.get(i);
							path.LineTo(cur * scale);
						}
					}
				}
				operands.clear();
				break;
			case 8: // rrcurveto
				for (int i = 5, count = operands.size(); i < count; i += 6)
				{
					cur.x += operands.get(i - 5);
					cur.y += operands.get(i - 4);
					cp1 = cur;
					cur.x += operands.get(i - 3);
					cur.y += operands.get(i - 2);
					cp2 = cur;
					cur.x += operands.get(i - 1);
					cur.y += operands.get(i);
					path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				}
				operands.clear();
				break;
			case 27: // hhcurveto
				if (operands.size() % 2 == 0)
				{
					for (int i = 3, count = operands.size(); i < count; i += 4)
					{
						cur.x += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.x += operands.get(i);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				else
				{
					cur.y += operands.get(0);
					for (int i = 4, count = operands.size(); i < count; i += 4)
					{
						cur.x += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.x += operands.get(i);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				operands.clear();
				break;
			case 31: // hvcurveto
				if (operands.size() % 8 == 4 || operands.size() % 8 == 5)
				{
					cur.x += operands.get(0);
					cp1 = cur;
					cur.x += operands.get(1);
					cur.y += operands.get(2);
					cp2 = cur;
					cur.y += operands.get(3);
					if (operands.size() == 5)
						cur.x += operands.get(4);
					path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);

					for (int i = 11, count = operands.size(); i < count; i += 8)
					{
						cur.y += operands.get(i - 7);
						cp1 = cur;
						cur.x += operands.get(i - 6);
						cur.y += operands.get(i - 5);
						cp2 = cur;
						cur.x += operands.get(i - 4);
						path.BezierTo(cp1 * scale, cp2  * scale, cur * scale);

						cur.x += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.y += operands.get(i);
						if (i + 2 == count)
							cur.x += operands.get(i + 1);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				else
				{
					for (int i = 7, count = operands.size(); i < count; i += 8)
					{
						cur.x += operands.get(i - 7);
						cp1 = cur;
						cur.x += operands.get(i - 6);
						cur.y += operands.get(i - 5);
						cp2 = cur;
						cur.y += operands.get(i - 4);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);

						cur.y += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.x += operands.get(i);
						if (i + 2 == count)
							cur.y += operands.get(i + 1);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				operands.clear();
				break;
			case 24: // rcurveline
				for (int i = 5, count = operands.size() - 2; i < count; i += 6)
				{
					cur.x += operands.get(i - 5);
					cur.y += operands.get(i - 4);
					cp1 = cur;
					cur.x += operands.get(i - 3);
					cur.y += operands.get(i - 2);
					cp2 = cur;
					cur.x += operands.get(i - 1);
					cur.y += operands.get(i);
					path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				}
				cur.x += operands.get(-2);
				cur.y += operands.get(-1);
				path.LineTo(cur * scale);
				operands.clear();
				break;
			case 25: // rlinecurve
				for (int i = 1, count = operands.size() - 6; i < count; i += 2)
				{
					cur.x += operands.get(i - 1);
					cur.y += operands.get(i);
					path.LineTo(cur * scale);
				}
				cur.x += operands.get(-6);
				cur.y += operands.get(-5);
				cp1 = cur;
				cur.x += operands.get(-4);
				cur.y += operands.get(-3);
				cp2 = cur;
				cur.x += operands.get(-2);
				cur.y += operands.get(-1);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				operands.clear();
				break;
			case 30: // vhcurveto
				if (operands.size() % 8 == 4 || operands.size() % 8 == 5)
				{
					cur.y += operands.get(0);
					cp1 = cur;
					cur.x += operands.get(1);
					cur.y += operands.get(2);
					cp2 = cur;
					cur.x += operands.get(3);
					if (operands.size() == 5)
						cur.y += operands.get(4);
					path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);

					for (int i = 11, count = operands.size(); i < count; i += 8)
					{
						cur.x += operands.get(i - 7);
						cp1 = cur;
						cur.x += operands.get(i - 6);
						cur.y += operands.get(i - 5);
						cp2 = cur;
						cur.y += operands.get(i - 4);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);

						cur.y += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.x += operands.get(i);
						if (i + 2 == count)
							cur.y += operands.get(i + 1);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				else
				{
					for (int i = 7, count = operands.size(); i < count; i += 8)
					{
						cur.y += operands.get(i - 7);
						cp1 = cur;
						cur.x += operands.get(i - 6);
						cur.y += operands.get(i - 5);
						cp2 = cur;
						cur.x += operands.get(i - 4);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);

						cur.x += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.y += operands.get(i);
						if (i + 2 == count)
							cur.x += operands.get(i + 1);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				operands.clear();
				break;
			case 26: // vvcurveto
				if (operands.size() % 2 == 0)
				{
					for (int i = 3, count = operands.size(); i < count; i += 4)
					{
						cur.y += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.y += operands.get(i);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				else
				{
					cur.x += operands.get(0);
					for (int i = 4, count = operands.size(); i < count; i += 4)
					{
						cur.y += operands.get(i - 3);
						cp1 = cur;
						cur.x += operands.get(i - 2);
						cur.y += operands.get(i - 1);
						cp2 = cur;
						cur.y += operands.get(i);
						path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
					}
				}
				operands.clear();
				break;
			case 1235: // flex
				// To do: collapse to line when the flex depth is less than fd/100 device pixels
				fd = operands.get(12);
				cur.x += operands.get(0);
				cur.y += operands.get(1);
				cp1 = cur;
				cur.x += operands.get(2);
				cur.y += operands.get(3);
				cp2 = cur;
				cur.x += operands.get(4);
				cur.y += operands.get(5);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				cur.x += operands.get(6);
				cur.y += operands.get(7);
				cp1 = cur;
				cur.x += operands.get(8);
				cur.y += operands.get(9);
				cp2 = cur;
				cur.x += operands.get(10);
				cur.y += operands.get(11);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				operands.clear();
				break;
			case 1234: // hflex
				// To do: collapse to line when flex depth is less than 0.5 device pixels (fd is 50)
				cur.x += operands.get(0);
				cp1 = cur;
				cur.x += operands.get(1);
				cur.y += operands.get(2);
				cp2 = cur;
				cur.x += operands.get(3);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				cur.x += operands.get(4);
				cp1 = cur;
				cur.x += operands.get(5);
				cp2 = cur;
				cur.x += operands.get(6);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				operands.clear();
				break;
			case 1236: // hflex1
				// To do: collapse to line when flex depth is less than 0.5 device pixels (fd is 50)
				cur.x += operands.get(0);
				cur.y += operands.get(1);
				cp1 = cur;
				cur.x += operands.get(2);
				cur.y += operands.get(3);
				cp2 = cur;
				cur.x += operands.get(4);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				cur.x += operands.get(5);
				cp1 = cur;
				cur.x += operands.get(6);
				cur.y += operands.get(7);
				cp2 = cur;
				cur.x += operands.get(8);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				operands.clear();
				break;
			case 1237: // flex1
				// To do: collapse to line when flex depth is less than 0.5 device pixels (fd is 50)
				flex1start = cur;
				cur.x += operands.get(0);
				cur.y += operands.get(1);
				cp1 = cur;
				cur.x += operands.get(2);
				cur.y += operands.get(3);
				cp2 = cur;
				cur.x += operands.get(4);
				cur.y += operands.get(5);
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				cur.x += operands.get(6);
				cur.y += operands.get(7);
				cp1 = cur;
				cur.x += operands.get(8);
				cur.y += operands.get(9);
				cp2 = cur;
				if (std::abs(flex1start.x - cur.x) > std::abs(flex1start.y - cur.y))
				{
					cur.x += operands.get(10);
					cur.y = flex1start.y;
				}
				else
				{
					cur.x = flex1start.x;
					cur.y += operands.get(10);
				}
				path.BezierTo(cp1 * scale, cp2 * scale, cur * scale);
				operands.clear();
				break;

			// Finish path:
			case 14: // endchar
				if (widthArg)
				{
					if (operands.size() % 2 == 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				endchar = true;
				break;

			// Hint operators:
			case 1: // hstem
				if (widthArg)
				{
					if (operands.size() % 2 == 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				// y = operands.get(0);
				// dy = operands.get(1);
				hintCount++;
				for (int i = 3, count = operands.size(); i < count; i += 2)
				{
					// dya = operands.get(i - 1);
					// dyb = operands.get(i);
					hintCount++;
				}
				operands.clear();
				break;
			case 3: // vstem
				if (widthArg)
				{
					if (operands.size() % 2 == 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				// x = operands.get(0);
				// dx = operands.get(1);
				hintCount++;
				for (int i = 3, count = operands.size(); i < count; i += 2)
				{
					// dxa = operands.get(i - 1);
					// dxb = operands.get(i);
					hintCount++;
				}
				operands.clear();
				break;
			case 18: // hstemhm
				if (widthArg)
				{
					if (operands.size() % 2 == 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				// y = operands.get(0);
				// dy = operands.get(1);
				hintCount++;
				for (int i = 3, count = operands.size(); i < count; i += 2)
				{
					// dya = operands.get(i - 1);
					// dyb = operands.get(i);
					hintCount++;
				}
				operands.clear();
				break;
			case 23: // vstemhm
				if (widthArg)
				{
					if (operands.size() % 2 == 1)
					{
						widthValue = cff.PrivateDict.norminalWidthX + operands.get(0);
						operands.pop_front();
					}
					widthArg = false;
				}
				// x = operands.get(0);
				// dx = operands.get(1);
				hintCount++;
				for (int i = 3, count = operands.size(); i < count; i += 2)
				{
					// dxa = operands.get(i - 1);
					// dxb = operands.get(i);
					hintCount++;
				}
				operands.clear();
				break;
			case 19: // hintmask
				if (widthArg)
				{
					widthArg = false;
				}
				for (int i = 0; i < hintCount; i += 8)
				{
					reader.ReadCard8();
				}
				operands.clear();
				break;
			case 20: // cntrmask
				if (widthArg)
				{
					widthArg = false;
				}
				for (int i = 0; i < hintCount; i += 8)
				{
					reader.ReadCard8();
				}
				operands.clear();
				break;

			// Arithmetic operators:
			case 1209: // abs
				tmp = std::abs(operands.get(-1));
				operands.pop(1);
				operands.push(tmp);
				break;
			case 1210: // add
				tmp = operands.get(-2) + operands.get(-1);
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1211: // sub
				tmp = operands.get(-2) - operands.get(-1);
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1212: // div
				tmp = operands.get(-2) / operands.get(-1);
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1214: // neg
				tmp = -operands.get(-1);
				operands.pop(1);
				operands.push(tmp);
				break;
			case 1223: // random
				operands.push((rand() + 1) / (double)RAND_MAX);
				break;
			case 1224: // mul
				tmp = operands.get(-2) * operands.get(-1);
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1226: // sqrt
				tmp = operands.get(-1);
				operands.pop(1);
				operands.push(tmp);
				break;
			case 1218: // drop
				operands.pop(1);
				break;
			case 1228: // exch
				std::exchange(operands.get(-2), operands.get(-1));
				break;
			case 1229: // index
			{
				int i = std::max((int)operands.get(-1), 0);
				tmp = operands.get(-1 - i);
				operands.pop(1);
				operands.push(tmp);
				break;
			}
			case 1230: // roll
			{
				int n = (int)operands.get(-2);
				int j = (int)operands.get(-1);
				if (n > 0)
				{
					if (n + 2 > operands.size())
						throw std::runtime_error("CFF roll count too large");
					if (j > 0)
					{
						j = j % n;
						auto it = operands.begin() + (operands.size() - 2 - n);
						std::rotate(it, it + j, it + n);
					}
					else if (j < 0)
					{
						j = (-j) % n;
						auto it = operands.rbegin() + 2;
						std::rotate(it, it + j, it + n);
					}
				}
				operands.pop(2);
				break;
			}
			case 1227: // dup
				tmp = operands.get(-1);
				operands.push(tmp);
				break;

			// Storage operators:
			case 1220: // put
				if ((int)operands.get(-1) < 0 || (int)operands.get(-1) >= 32)
					throw std::runtime_error("CFF storage index out of bounds");
				transient[(int)operands.get(-1)] = operands.get(-2);
				operands.pop(2);
				break;
			case 1221: // get
				if ((int)operands.get(-1) < 0 || (int)operands.get(-1) >= 32)
					throw std::runtime_error("CFF storage index out of bounds");
				tmp = transient[(int)operands.get(-1)];
				operands.pop(1);
				operands.push(tmp);
				break;

			// Conditional operators:
			case 1203: // and
				tmp = (operands.get(-2) != 0.0) && (operands.get(-1) != 0.0) ? 1.0 : 0.0;
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1204: // or
				tmp = (operands.get(-2) != 0.0) || (operands.get(-1) != 0.0) ? 1.0 : 0.0;
				operands.pop(2);
				operands.push(tmp);
				break;
			case 1205: // not
				tmp = (operands.get(-1) != 0.0) ? 0.0 : 1.0;
				operands.pop(1);
				operands.push(tmp);
				break;
			case 1215: // eq
				tmp = (operands.get(-2) == operands.get(-1)) ? 1.0 : 0.0;
				operands.pop(1);
				operands.push(tmp);
				break;
			case 1222: // ifelse
				tmp = (operands.get(-2) <= operands.get(-1)) ? operands.get(-4) : operands.get(-3);
				operands.pop(4);
				operands.push(tmp);
				break;

			// Subroutine operators:
			case 10: // callsubr
				callstack.push_back({ reader.Position(), endpos });
				tmp = operands.get(-1);
				operands.pop(1);
				if (cff.LocalSubroutines.size() < 1240)
					tmp += 107;
				else if (cff.LocalSubroutines.size() < 33900)
					tmp += 1131;
				else
					tmp += 32768;
				if ((int)tmp >= 0 && (int)cff.LocalSubroutines.size() <= (int)tmp)
					throw std::runtime_error("CFF local subroutine index out of bounds");
				reader.Seek(cff.LocalSubroutines[(int)tmp].Offset);
				endpos = cff.LocalSubroutines[(int)tmp].Offset + cff.LocalSubroutines[(int)tmp].Size;
				break;
			case 29: // callgsubr
				callstack.push_back({ reader.Position(), endpos });
				tmp = operands.get(-1);
				operands.pop(1);
				if (cff.GlobalSubroutines.size() < 1240)
					tmp += 107;
				else if (cff.GlobalSubroutines.size() < 33900)
					tmp += 1131;
				else
					tmp += 32768;
				if ((int)tmp >= 0 && (int)cff.GlobalSubroutines.size() <= (int)tmp)
					throw std::runtime_error("CFF global subroutine index out of bounds");
				reader.Seek(cff.GlobalSubroutines[(int)tmp].Offset);
				endpos = cff.GlobalSubroutines[(int)tmp].Offset + cff.GlobalSubroutines[(int)tmp].Size;
				break;
			case 11: // return
				if (callstack.empty())
					throw std::runtime_error("Invalid CFF return statement");
				reader.Seek(callstack.back().first);
				endpos = callstack.back().second;
				callstack.pop_back();
				break;

			// Obsolete operator:
			case 1200: // dotsection (no-op)
				break;

			default:
				throw std::runtime_error("Unknown CFF charstring operand");
			}
		}
	}
	path.Close();

	// Transform and find the final bounding box
	PathPoint bboxMin, bboxMax;
	if (!path.subpaths.front().points.empty())
	{
		bboxMin = path.subpaths.front().points.front();
		bboxMax = path.subpaths.front().points.front();
		bboxMin.x *= scaleX;
		bboxMin.y *= scaleY;
		bboxMax.x *= scaleX;
		bboxMax.y *= scaleY;
		for (auto& subpath : path.subpaths)
		{
			for (auto& point : subpath.points)
			{
				point.x *= scaleX;
				point.y *= scaleY;
				bboxMin.x = std::min(bboxMin.x, point.x);
				bboxMin.y = std::min(bboxMin.y, point.y);
				bboxMax.x = std::max(bboxMax.x, point.x);
				bboxMax.y = std::max(bboxMax.y, point.y);
			}
		}
	}

	bboxMin.x = std::floor(bboxMin.x);
	bboxMin.y = std::floor(bboxMin.y);

	// Reposition glyph to bitmap so it begins at (0,0) for our bitmap
	for (auto& subpath : path.subpaths)
	{
		for (auto& point : subpath.points)
		{
			point.x -= bboxMin.x;
			point.y -= bboxMin.y;
		}
	}

#ifdef DUMP_GLYPH
	std::string svgxmlstart = R"(<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg width="1000px" height="1000px" viewBox="0 0 25 25" xmlns="http://www.w3.org/2000/svg" version="1.1">
<path fill-rule="evenodd" d=")";
	std::string svgxmlend = R"(" fill="red" />
</svg>)";

	std::ofstream out("c:\\development\\glyph.svg");
	out << svgxmlstart;

	for (auto& subpath : path.subpaths)
	{
		size_t pos = 0;
		out << "M" << subpath.points[pos].x << " " << subpath.points[pos].y << " ";
		pos++;
		for (PathFillCommand cmd : subpath.commands)
		{
			int count = 0;
			if (cmd == PathFillCommand::line)
			{
				out << "L";
				count = 1;
			}
			else if (cmd == PathFillCommand::quadradic)
			{
				out << "Q";
				count = 2;
			}
			else if (cmd == PathFillCommand::cubic)
			{
				out << "C";
				count = 3;
			}

			for (int i = 0; i < count; i++)
			{
				out << subpath.points[pos].x << " " << subpath.points[pos].y << " ";
				pos++;
			}
		}
		if (subpath.closed)
			out << "Z";
	}

	out << svgxmlend;
	out.close();
#endif

	TrueTypeGlyph glyph;

	// Rasterize the glyph
	glyph.width = (int)std::floor(bboxMax.x - bboxMin.x) + 1;
	glyph.height = (int)std::floor(bboxMax.y - bboxMin.y) + 1;
	glyph.grayscale.reset(new uint8_t[glyph.width * glyph.height]);
	uint8_t* grayscale = glyph.grayscale.get();
	path.Rasterize(grayscale, glyph.width, glyph.height);

	// TBD: gridfit or not?
	glyph.advanceWidth = (int)std::round(advanceWidth * scale * scaleX);
	glyph.leftSideBearing = (int)std::round(bboxMin.x);
	glyph.yOffset = (int)std::round(bboxMin.y);

	return glyph;
}
