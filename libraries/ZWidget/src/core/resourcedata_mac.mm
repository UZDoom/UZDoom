#include "zwidget/core/resourcedata.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

#import <Cocoa/Cocoa.h>
#import <CoreText/CoreText.h>

extern "C"
{
	typedef const void* CFTypeRef;
	void CFRelease(CFTypeRef cf);
}

std::vector<SingleFontData> ResourceData::LoadSystemFont()
{
	SingleFontData fontData;
	@autoreleasepool
	{
		NSFont* systemFont = [NSFont systemFontOfSize:13.0]; // Use a default size
		if (!systemFont)
			throw std::runtime_error("Failed to get system font");

		CTFontRef ctFont = (__bridge CTFontRef)systemFont;
		CFURLRef fontURL = (CFURLRef)CTFontCopyAttribute(ctFont, kCTFontURLAttribute);
		if (!fontURL)
			throw std::runtime_error("Failed to get font URL from system font");

		// __bridge_transfer transfers ownership to ARC, so no manual CFRelease is needed
		NSString* fontPath = (NSString*)CFURLCopyFileSystemPath(fontURL, kCFURLPOSIXPathStyle);
		if (!fontPath)
			throw std::runtime_error("Failed to convert font URL to file path");

		// Read the font file data
		try
		{
			fontData.fontdata = ReadAllBytes(std::string([fontPath UTF8String]));
		}
		catch (const std::exception& e)
		{
			CFRelease(fontURL);
			throw std::runtime_error(std::string("Error reading system font file: ") + e.what());
		}
		catch (...)
		{
			CFRelease(fontURL);
			throw;
		}

		CFRelease(fontURL);
	}
	return { fontData };
}

std::vector<SingleFontData> ResourceData::LoadMonospaceSystemFont()
{
	SingleFontData fontData;
	@autoreleasepool
	{
		NSFont* systemFont = nil;
		if (@available(macOS 10.15, *)) {
			systemFont = [NSFont monospacedSystemFontOfSize:13.0 weight:NSFontWeightRegular]; // Use a default size
		}


		if (!systemFont) {
			// Fallback for older macOS versions
			systemFont = [NSFont systemFontOfSize:13.0];
		}

		if (!systemFont) 
			// Double check after fallback
			throw std::runtime_error("Failed to get system font");

		CTFontRef ctFont = (__bridge CTFontRef)systemFont;
		CFURLRef fontURL = (CFURLRef)CTFontCopyAttribute(ctFont, kCTFontURLAttribute);
		if (!fontURL)
			throw std::runtime_error("Failed to get font URL from system font");

		NSString* fontPath = (NSString*)CFURLCopyFileSystemPath(fontURL, kCFURLPOSIXPathStyle);
		if (!fontPath)
			throw std::runtime_error("Failed to convert font URL to file path");

		// Read the font file data
		try
		{
			fontData.fontdata = ReadAllBytes(std::string([fontPath UTF8String]));
		}
		catch (const std::exception& e)
		{
			CFRelease(fontURL);
			throw std::runtime_error(std::string("Error reading system font file: ") + e.what());
		}
		catch (...)
		{
			CFRelease(fontURL);
			throw;
		}

		CFRelease(fontURL);
	}
	return { std::move(fontData) };
}

double ResourceData::GetSystemFontSize()
{
	return [NSFont systemFontSize];
}

class ResourceLoaderMac : public ResourceLoader
{
public:
	std::vector<SingleFontData> LoadFont(const std::string& name)
	{
		if (name == "system")
			return ResourceData::LoadSystemFont();
		else if (name == "monospace")
			return ResourceData::LoadMonospaceSystemFont();
		else
			return { SingleFontData{ReadAllBytes(name + ".ttf"), ""} };
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file)
			throw std::runtime_error("Could not open: " + filename);

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
			throw std::runtime_error("Could not read: " + filename);

		return buffer;
	}
};

struct ResourceDefaultLoader
{
	ResourceDefaultLoader() { loader = std::make_unique<ResourceLoaderMac>(); }
	std::unique_ptr<ResourceLoader> loader;
};

static std::unique_ptr<ResourceLoader>& GetLoader()
{
	static ResourceDefaultLoader loader;
	return loader.loader;
}

ResourceLoader* ResourceLoader::Get()
{
	return GetLoader().get();
}

void ResourceLoader::Set(std::unique_ptr<ResourceLoader> instance)
{
	GetLoader() = std::move(instance);
}
