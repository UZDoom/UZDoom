#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <string>
#include "core/rect.h"

class TTFDataBuffer
{
public:
	static std::shared_ptr<TTFDataBuffer> create(size_t size);
	static std::shared_ptr<TTFDataBuffer> create(std::vector<uint8_t> buffer);

	virtual ~TTFDataBuffer() = default;
	virtual char* data() = 0;
	virtual const char* data() const = 0;
	virtual size_t size() const = 0;
	virtual size_t capacity() const = 0;
	virtual void setSize(size_t size) = 0;
	virtual void setCapacity(size_t capacity) = 0;

	template<typename Type> Type* data() { return reinterpret_cast<Type*>(data()); }
	template<typename Type> const Type* data() const { return reinterpret_cast<const Type*>(data()); }
};

class TTFDataBufferImpl : public TTFDataBuffer
{
public:
	TTFDataBufferImpl(size_t init_size) { setSize(init_size); }
	TTFDataBufferImpl(std::vector<uint8_t> buffer) : buffer(std::move(buffer)) { }

	char* data() override { return (char*)buffer.data(); }
	const char* data() const override { return (const char*)buffer.data(); }
	size_t size() const override { return buffer.size(); }
	size_t capacity() const override { return buffer.capacity(); }
	void setSize(size_t size) override { buffer.resize(size); }
	void setCapacity(size_t capacity) override { buffer.reserve(capacity); }

private:
	std::vector<uint8_t> buffer;
};

inline std::shared_ptr<TTFDataBuffer> TTFDataBuffer::create(size_t size)
{
	return std::make_shared<TTFDataBufferImpl>(size);
}

inline std::shared_ptr<TTFDataBuffer> TTFDataBuffer::create(std::vector<uint8_t> buffer)
{
	return std::make_shared<TTFDataBufferImpl>(std::move(buffer));
}

typedef Point PathPoint;

typedef uint8_t ttf_uint8;
typedef uint16_t ttf_uint16;
typedef uint32_t ttf_uint24; // 24-bit unsigned integer
typedef uint32_t ttf_uint32;

typedef int8_t ttf_int8;
typedef int16_t ttf_int16;
typedef int32_t ttf_int32;

typedef uint32_t ttf_Fixed; // 32-bit signed fixed-point number (16.16)
typedef uint16_t ttf_UFWORD; // uint16 that describes a quantity in font design units
typedef int16_t ttf_FWORD; // int16 that describes a quantity in font design units
typedef uint16_t ttf_F2DOT14; // 16-bit signed fixed number with the low 14 bits of fraction (2.14)
typedef uint64_t ttf_LONGDATETIME; // number of seconds since 12:00 midnight, January 1, 1904, UTC

typedef std::array<uint8_t, 4> ttf_Tag; // 4 byte identifier

typedef uint16_t ttf_Offset16; // Short offset to a table, same as uint16, NULL offset = 0x0000
typedef uint32_t ttf_Offset24; // 24-bit offset to a table, same as uint24, NULL offset = 0x000000
typedef uint32_t ttf_Offset32; // Long offset to a table, same as uint32, NULL offset = 0x00000000

typedef uint32_t ttf_Version16Dot16; // Packed 32-bit value with major and minor version numbers

struct CFFObjectData
{
	size_t Offset = 0;
	size_t Size = 0;
};

class TrueTypeFileReader
{
public:
	TrueTypeFileReader() = default;
	TrueTypeFileReader(const void* data, size_t size) : data(static_cast<const uint8_t*>(data)), size(size) { }

	bool IsEndOfData() const { return pos == size; }
	size_t Size() const { return size; }
	void Seek(size_t newpos);
	size_t Position() const { return pos; }

	void Read(void* output, size_t count);
	ttf_uint8 ReadUInt8();
	ttf_uint16 ReadUInt16();
	ttf_uint24 ReadUInt24();
	ttf_uint32 ReadUInt32();
	ttf_int8 ReadInt8();
	ttf_int16 ReadInt16();
	ttf_int32 ReadInt32();
	ttf_Fixed ReadFixed();
	ttf_UFWORD ReadUFWORD();
	ttf_FWORD ReadFWORD();
	ttf_F2DOT14 ReadF2DOT14();
	ttf_LONGDATETIME ReadLONGDATETIME();
	ttf_Tag ReadTag();
	ttf_Offset16 ReadOffset16();
	ttf_Offset24 ReadOffset24();
	ttf_Offset32 ReadOffset32();
	ttf_Version16Dot16 ReadVersion16Dot16();

	// CFF data:
	uint8_t ReadCard8() { return ReadUInt8(); }
	uint16_t ReadCard16() { return ReadUInt16(); }
	uint32_t ReadCard24() { return ReadUInt24(); }
	uint32_t ReadCard32() { return ReadUInt32(); }
	std::vector<CFFObjectData> ReadIndex();
	uint8_t ReadOffsetSize();
	uint32_t ReadOffset(uint8_t offsetSize);
	static bool IsOperand(uint8_t b0) { return (b0 >= 28 && b0 <= 30) || (b0 >= 32 && b0 <= 254); }
	static bool IsOperator(uint8_t b0) { return b0 <= 21; }
	int ReadOperator(uint8_t b0);
	double ReadOperand(uint8_t b0);

private:
	const uint8_t* data = nullptr;
	size_t size = 0;
	size_t pos = 0;

	friend class TrueTypeFileWriter;
};

class TrueTypeFileWriter
{
public:
	TrueTypeFileWriter();

	void Write(TrueTypeFileReader& src, size_t count);
	void Write(const void* src, size_t count);
	void WriteUInt8(ttf_uint8 value);
	void WriteUInt16(ttf_uint16 value);
	void WriteUInt24(ttf_uint24 value);
	void WriteUInt32(ttf_uint32 value);
	void WriteInt8(ttf_int8 value);
	void WriteInt16(ttf_int16 value);
	void WriteInt32(ttf_int32 value);
	void WriteFixed(ttf_Fixed value);
	void WriteUFWORD(ttf_UFWORD value);
	void WriteFWORD(ttf_FWORD value);
	void WriteF2DOT14(ttf_F2DOT14 value);
	void WriteLONGDATETIME(ttf_LONGDATETIME value);
	void WriteTag(ttf_Tag value);
	void WriteOffset16(ttf_Offset16 value);
	void WriteOffset24(ttf_Offset24 value);
	void WriteOffset32(ttf_Offset32 value);
	void WriteVersion16Dot16(ttf_Version16Dot16 value);

	size_t Position() const { return pos; }
	void Seek(size_t newpos) { pos = newpos; }

	const std::shared_ptr<TTFDataBuffer>& Data() const { return data; }

private:
	size_t pos = 0;
	std::shared_ptr<TTFDataBuffer> data;
};

struct TTF_TableRecord
{
	ttf_Tag tableTag = {};
	ttf_uint32 checksum = {};
	ttf_Offset32 offset = {};
	ttf_uint32 length = {};

	void Load(TrueTypeFileReader& reader);

	TrueTypeFileReader GetReader(const void* filedata, size_t filesize) const
	{
		if ((size_t)offset + length > filesize)
			throw std::runtime_error("Invalid TTF table directory record");

		return TrueTypeFileReader((uint8_t*)filedata + offset, length);
	}
};

struct TTF_TableDirectory
{
	ttf_uint32 sfntVersion = {};
	ttf_uint16 numTables = {};
	std::vector<TTF_TableRecord> tableRecords;

	// To do: Apple TTF fonts allow 'true' and 'typ1' for sfntVersion as well.
	bool ContainsTTFOutlines() const { return sfntVersion == 0x00010000; }
	bool ContainsCFFData() const { return sfntVersion == 0x4F54544F; }

	void Load(TrueTypeFileReader& reader);

	const TTF_TableRecord& GetRecord(const char* tag) const
	{
		for (const auto& record : tableRecords)
		{
			if (memcmp(record.tableTag.data(), tag, 4) == 0)
			{
				return record;
			}
		}
		throw std::runtime_error(std::string("Could not find required '") + tag + "' table entry");
	}

	bool TableExists(const char* tag) const
	{
		for (const auto& record : tableRecords)
		{
			if (memcmp(record.tableTag.data(), tag, 4) == 0)
			{
				return true;
			}
		}
		return false;
	}

	TrueTypeFileReader GetReader(const void* filedata, size_t filesize, const char* tag) const
	{
		return GetRecord(tag).GetReader(filedata, filesize);
	}
};

struct TTC_Header
{
	ttf_Tag ttcTag = {};
	ttf_uint16 majorVersion = {};
	ttf_uint16 minorVersion = {};
	ttf_uint32 numFonts = {};
	std::vector<ttf_Offset32> tableDirectoryOffsets;

	// majorVersion = 2, minorVersion = 0:
	ttf_uint32 dsigTag = {};
	ttf_uint32 dsigLength = {};
	ttf_uint32 dsigOffset = {};

	void Load(TrueTypeFileReader& reader);
};

struct TTF_EncodingRecord
{
	ttf_uint16 platformID = {};
	ttf_uint16 encodingID = {};
	ttf_Offset32 subtableOffset = {};
};

struct TTF_CMap // 'cmap' Character to glyph mapping
{
	ttf_uint16 version = {};
	ttf_uint16 numTables = {};
	std::vector<TTF_EncodingRecord> encodingRecords; // [numTables]

	void Load(TrueTypeFileReader& reader);

	TTF_EncodingRecord GetEncoding(ttf_uint16 platformID, ttf_uint16 encodingID) const
	{
		for (const TTF_EncodingRecord& record : encodingRecords)
		{
			if (record.platformID == platformID && record.encodingID == encodingID)
			{
				return record;
			}
		}
		return {};
	}
};

struct TTF_GlyphRange
{
	ttf_uint32 startCharCode = 0;
	ttf_uint32 endCharCode = 0;
	ttf_uint32 startGlyphID = 0;
};

struct TTF_CMapSubtable0 // Byte encoding table
{
	ttf_uint16 length = {};
	ttf_uint16 language = {};
	std::vector<ttf_uint8> glyphIdArray;

	void Load(TrueTypeFileReader& reader);
};

struct TTF_CMapSubtable4 // Segment mapping to delta values (U+0000 to U+FFFF)
{
	ttf_uint16 length = {};
	ttf_uint16 language = {};
	ttf_uint16 segCount = {};
	std::vector<ttf_uint16> endCode;
	ttf_uint16 reservedPad = {};
	std::vector<ttf_uint16> startCode;
	std::vector<ttf_int16> idDelta;
	std::vector<ttf_uint16> idRangeOffsets;
	std::vector<ttf_uint16> glyphIdArray;

	void Load(TrueTypeFileReader& reader);
};

struct TTF_CMapSubtable12 // Segmented coverage (U+0000 to U+10FFFF)
{
	ttf_uint16 reserved;
	ttf_uint32 length;
	ttf_uint32 language;
	ttf_uint32 numGroups;
	std::vector<TTF_GlyphRange> groups;

	void Load(TrueTypeFileReader& reader);
};

typedef TTF_CMapSubtable12 TTF_CMapSubtable13; // Many-to-one range mappings

struct TTF_FontHeader // 'head' Font header
{
	ttf_uint16 majorVersion = {};
	ttf_uint16 minorVersion = {};
	ttf_Fixed fontRevision = {};
	ttf_uint32 checksumAdjustment = {};
	ttf_uint32 magicNumber = {};
	ttf_uint16 flags = {};
	ttf_uint16 unitsPerEm = {};
	ttf_LONGDATETIME created = {};
	ttf_LONGDATETIME modified = {};
	ttf_int16 xMin = {};
	ttf_int16 yMin = {};
	ttf_int16 xMax = {};
	ttf_int16 yMax = {};
	ttf_uint16 macStyle = {};
	ttf_uint16 lowestRecPPEM = {};
	ttf_int16 fontDirectionHint = {};
	ttf_int16 indexToLocFormat = {};
	ttf_int16 glyphDataFormat = {};

	void Load(TrueTypeFileReader& reader);
};

struct TTF_HorizontalHeader // 'hhea' Horizontal header
{
	ttf_uint16 majorVersion = {};
	ttf_uint16 minorVersion = {};
	ttf_FWORD ascender = {};
	ttf_FWORD descender = {};
	ttf_FWORD lineGap = {};
	ttf_UFWORD advanceWidthMax = {};
	ttf_FWORD minLeftSideBearing = {};
	ttf_FWORD minRightSideBearing = {};
	ttf_FWORD xMaxExtent = {};
	ttf_int16 caretSlopeRise = {};
	ttf_int16 caretSlopeRun = {};
	ttf_int16 caretOffset = {};
	ttf_int16 reserved0 = {};
	ttf_int16 reserved1 = {};
	ttf_int16 reserved2 = {};
	ttf_int16 reserved3 = {};
	ttf_int16 metricDataFormat = {};
	ttf_uint16 numberOfHMetrics = {};

	void Load(TrueTypeFileReader& reader);
};

struct TTF_MaximumProfile;

struct TTF_HorizontalMetrics // 'hmtx' Horizontal metrics
{
	struct longHorMetric
	{
		ttf_uint16 advanceWidth = {};
		ttf_int16 lsb = {};
	};
	std::vector<longHorMetric> hMetrics; // [hhea.numberOfHMetrics]
	std::vector<ttf_int16> leftSideBearings; // [maxp.numGlyphs - hhea.numberOfHMetrics]

	void Load(const TTF_HorizontalHeader& hhea, const TTF_MaximumProfile& maxp, TrueTypeFileReader& reader);
};

struct TTF_MaximumProfile // 'maxp' Maximum profile
{
	// v0.5 and v1:
	ttf_Version16Dot16 version = {};
	ttf_uint16 numGlyphs = {};

	// v1 only:
	ttf_uint16 maxPoints = {};
	ttf_uint16 maxContours = {};
	ttf_uint16 maxCompositePoints = {};
	ttf_uint16 maxCompositeContours = {};
	ttf_uint16 maxZones = {};
	ttf_uint16 maxTwilightPoints = {};
	ttf_uint16 maxStorage = {};
	ttf_uint16 maxFunctionDefs = {};
	ttf_uint16 maxInstructionDefs = {};
	ttf_uint16 maxStackElements = {};
	ttf_uint16 maxSizeOfInstructions = {};
	ttf_uint16 maxComponentElements = {};
	ttf_uint16 maxComponentDepth = {};

	void Load(TrueTypeFileReader& reader);
};

class TTCFontName
{
public:
	std::string FamilyName;     // Arial
	std::string SubfamilyName;  // Regular
	std::string FullName;       // Arial Regular
	std::string UniqueID;
	std::string VersionString;
	std::string PostscriptName;
};

struct TTF_NamingTable // 'name' Naming table
{
	struct NameRecord
	{
		ttf_uint16 platformID = {};
		ttf_uint16 encodingID = {};
		ttf_uint16 languageID = {};
		ttf_uint16 nameID = {};
		ttf_uint16 length = {};
		ttf_Offset16 stringOffset = {};
		std::string text;
	};

	struct LangTagRecord
	{
		ttf_uint16 length = {};
		ttf_Offset16 langTagOffset = {};
	};

	// v0 and v1:
	ttf_uint16 version = {};
	ttf_uint16 count = {};
	ttf_Offset16 storageOffset = {};
	std::vector<NameRecord> nameRecord; // [count]

	// v1 only:
	ttf_uint16 langTagCount = {};
	std::vector<LangTagRecord> langTagRecord; // [langTagCount]

	void Load(TrueTypeFileReader& reader);

	TTCFontName GetFontName() const;
};

struct TTF_OS2Windows // 'OS/2' Windows specific metrics
{
	ttf_uint16 version = {};
	ttf_int16 xAvgCharWidth = {};
	ttf_uint16 usWeightClass = {};
	ttf_uint16 usWidthClass = {};
	ttf_uint16 fsType = {};
	ttf_int16 ySubscriptXSize = {};
	ttf_int16 ySubscriptYSize = {};
	ttf_int16 ySubscriptXOffset = {};
	ttf_int16 ySubscriptYOffset = {};
	ttf_int16 ySuperscriptXSize = {};
	ttf_int16 ySuperscriptYSize = {};
	ttf_int16 ySuperscriptXOffset = {};
	ttf_int16 ySuperscriptYOffset = {};
	ttf_int16 yStrikeoutSize = {};
	ttf_int16 yStrikeoutPosition = {};
	ttf_int16 sFamilyClass = {};
	ttf_uint8 panose[10] = {};
	ttf_uint32 ulUnicodeRange1 = {};
	ttf_uint32 ulUnicodeRange2 = {};
	ttf_uint32 ulUnicodeRange3 = {};
	ttf_uint32 ulUnicodeRange4 = {};
	ttf_Tag achVendID = {};
	ttf_uint16 fsSelection = {};
	ttf_uint16 usFirstCharIndex = {};
	ttf_uint16 usLastCharIndex = {};
	ttf_int16 sTypoAscender = {};
	ttf_int16 sTypoDescender = {};
	ttf_int16 sTypoLineGap = {};
	ttf_uint16 usWinAscent = {}; // may be missing in v0 due to bugs in Apple docs
	ttf_uint16 usWinDescent = {}; // may be missing in v0 due to bugs in Apple docs
	ttf_uint32 ulCodePageRange1 = {}; // v1
	ttf_uint32 ulCodePageRange2 = {};
	ttf_int16 sxHeight = {}; // v2, v3 and v4
	ttf_int16 sCapHeight = {};
	ttf_uint16 usDefaultChar = {};
	ttf_uint16 usBreakChar = {};
	ttf_uint16 usMaxContext = {};
	ttf_uint16 usLowerOpticalPointSize = {}; // v5
	ttf_uint16 usUpperOpticalPointSize = {};

	void Load(TrueTypeFileReader& reader);
};

// Simple glyph flags:
#define TTF_ON_CURVE_POINT 0x01
#define TTF_X_SHORT_VECTOR 0x02
#define TTF_Y_SHORT_VECTOR 0x04
#define TTF_REPEAT_FLAG 0x08
#define TTF_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
#define TTF_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
#define TTF_OVERLAP_SIMPLE = 0x40

// Composite glyph flags:
#define TTF_ARG_1_AND_2_ARE_WORDS 0x0001
#define TTF_ARGS_ARE_XY_VALUES 0x0002
#define TTF_ROUND_XY_TO_GRID 0x0004
#define TTF_WE_HAVE_A_SCALE 0x0008
#define TTF_MORE_COMPONENTS 0x0020
#define TTF_WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define TTF_WE_HAVE_A_TWO_BY_TWO 0x0080
#define TTF_WE_HAVE_INSTRUCTIONS 0x0100
#define TTF_USE_MY_METRICS 0x0200
#define TTF_OVERLAP_COMPOUND 0x0400
#define TTF_SCALED_COMPONENT_OFFSET 0x0800
#define TTF_UNSCALED_COMPONENT_OFFSET 0x1000

struct TTF_IndexToLocation // 'loca' Index to location
{
	std::vector<ttf_Offset32> offsets;

	void Load(const TTF_FontHeader& head, const TTF_MaximumProfile& maxp, TrueTypeFileReader& reader);
};

struct TTF_Point
{
	float x;
	float y;
};

struct TTF_SimpleGlyph
{
	std::vector<int> endPtsOfContours;
	std::vector<ttf_uint8> flags;
	std::vector<TTF_Point> points;
};

struct CFFHeader
{
	uint8_t major = 0;
	uint8_t minor = 0;
	uint8_t headerSize = 0;
	uint8_t offsetSize = 0;
};

struct CFFTopDict
{
	std::string version;
	std::string Notice;
	std::string Copyright;
	std::string FullName;
	std::string FamilyName;
	std::string Weight;
	bool isFixedPitch = false;
	double ItalicAngle = 0.0;
	double UnderlinePosition = -100.0;
	double UnderlineThickness = 50.0;
	double PaintType = 0.0;
	double CharstringType = 0.0; // must be 2 for OpenType fonts
	std::array<double, 6> FontMatrix = { 0.001, 0.0, 0.0, 0.001, 0.0, 0.0 };
	double UniqueID = 0.0;
	std::array<double, 4> FontBBox = { 0.0, 0.0, 0.0, 0.0 };
	double StrokeWidth = 0.0;
	std::vector<double> XUID;
	size_t charset = 0; // charset offset
	size_t Encoding = 0; // 0 = standard encoding, 1 = expert encoding, otherwise its an encoding offset
	size_t CharStrings = 0; // CharStrings index offset
	CFFObjectData PrivateDict;
	double SyntheticBase = 0.0; // synthetic base font index
	std::string Postscript;
	std::string BaseFontName;
	std::vector<double> BaseFontBlend;

	// CID fonts only:
	std::string Registry;
	std::string Ordering;
	double Supplement = 0.0;
	double CIDFontVersion = 0.0;
	double CIDFontRevision = 0.0;
	double CIDFontType = 0.0;
	double CIDCount = 8720.0;
	double UIDBase = 0.0;
	size_t FDArray = 0; // Font dict index offset
	size_t FDSelect = 0; // FDSelect offset
	std::string FontName;
};

struct CFFPrivateDict
{
	std::vector<double> BlueValues;
	std::vector<double> OtherValues;
	std::vector<double> FamilyBlues;
	std::vector<double> FamilyOtherBlues;
	double BlueScale = 0.039625;
	double BlueShift = 7.0;
	double BlueFuzz = 1.0;
	double StdHW = 0.0;
	double StdVW = 0.0;
	std::vector<double> StemSnapH;
	std::vector<double> StemSnapV;
	bool ForceBold = false;
	double LanguageGroup = 0.0;
	double ExpansionFactor = 0.06;
	double initialRandomSeed = 0.0;
	size_t Subrs = 0; // local subroutines offset
	double defaultWidthX = 0.0;
	double norminalWidthX = 0.0;
};

struct TTF_CFF // Compact Font Format
{
	TTF_TableRecord Record;
	CFFHeader Header;
	std::vector<CFFObjectData> NameIndex;
	std::vector<CFFObjectData> TopDictIndex;
	std::vector<CFFObjectData> StringIndex;
	std::vector<CFFObjectData> GlobalSubroutines;

	// OpenType fonts are only allowed to have one CFF font
	CFFTopDict TopDict;
	CFFPrivateDict PrivateDict;
	std::vector<CFFObjectData> CharStrings; // glyphs
	std::vector<CFFObjectData> LocalSubroutines;

	void Load(TrueTypeFileReader& reader);

	std::string GetName(TrueTypeFileReader& reader, int index);
	std::string GetString(TrueTypeFileReader& reader, int index);

	CFFTopDict LoadTopDict(TrueTypeFileReader& reader, int index);
	CFFPrivateDict LoadPrivateDict(TrueTypeFileReader& reader, CFFObjectData obj);
};

class TrueTypeGlyph
{
public:
	int advanceWidth = 0;
	int leftSideBearing = 0;
	int yOffset = 0;

	int width = 0;
	int height = 0;
	std::unique_ptr<uint8_t[]> grayscale;
};

class TrueTypeTextMetrics
{
public:
	double ascent = 0.0;
	double descent = 0.0;
	double lineGap = 0.0;
	double capHeight = 0.0;
	double stemV = 0.0;
	double italicAngle = 0.0;
	double defaultWidth = 0.0;
	struct
	{
		double minX = 0.0;
		double maxX = 0.0;
		double minY = 0.0;
		double maxY = 0.0;
	} bbox;
};

class TrueTypeFont
{
public:
	TrueTypeFont(std::shared_ptr<TTFDataBuffer> data, int ttcFontIndex = 0);

	static std::vector<TTCFontName> GetFontNames(const std::shared_ptr<TTFDataBuffer>& data);

	TTCFontName GetFontName() const;
	TrueTypeTextMetrics GetTextMetrics(double height) const;
	uint32_t GetGlyphIndex(uint32_t codepoint) const;
	TrueTypeGlyph LoadGlyph(uint32_t glyphIndex, double height) const;

	double GetAdvanceWidth(uint32_t glyphIndex, double height) const;

	const std::shared_ptr<TTFDataBuffer>& GetData() const { return data; }

	std::shared_ptr<TTFDataBuffer> CreatePdfSubsetFont(const std::vector<uint16_t>& glyphs);

private:
	TrueTypeGlyph LoadTTFGlyph(uint32_t glyphIndex, double height) const;
	void LoadCharacterMapEncoding(TrueTypeFileReader& reader);
	void LoadGlyph(TTF_SimpleGlyph& glyph, uint32_t glyphIndex, int compositeDepth = 0) const;
	static float F2DOT14_ToFloat(ttf_F2DOT14 v);

	TrueTypeGlyph LoadCFFGlyph(uint32_t glyphIndex, double height) const;

	std::shared_ptr<TTFDataBuffer> data;

	TTC_Header ttcHeader;
	TTF_TableDirectory directory;

	// Required for all OpenType fonts:
	TTF_CMap cmap;
	TTF_FontHeader head;
	TTF_HorizontalHeader hhea;
	TTF_HorizontalMetrics hmtx;
	TTF_MaximumProfile maxp;
	TTF_NamingTable name;
	TTF_OS2Windows os2;

	// TrueType outlines:
	TTF_TableRecord glyf; // Parsed on a per glyph basis using offsets from other tables
	TTF_IndexToLocation loca;

	// CFF outlines
	TTF_CFF cff;

	std::vector<TTF_GlyphRange> Ranges;
	std::vector<TTF_GlyphRange> ManyToOneRanges;
};
