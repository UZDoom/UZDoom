#include "zwidget/core/resourcedata.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cmath>
#include <gio/gio.h>
#include <fontconfig/fontconfig.h>

static std::vector<uint8_t> ReadAllBytes(const std::string& filename)
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

static std::vector<SingleFontData> GetGtkUIFont(const std::string& propertyName)
{
	// Ask GTK what the UI font is:

	GSettings *settings = g_settings_new ("org.gnome.desktop.interface");
	gchar* str = g_settings_get_string(settings, propertyName.c_str());
	if (!str)
		throw std::runtime_error("Could not get gtk font property");
	std::string fontname = str;
	g_free(str);

	// Find the font filename using fontconfig:

	std::string filename;
	FcConfig* config = FcInitLoadConfigAndFonts();
	if (config)
	{
		FcPattern* pat = FcNameParse((const FcChar8*)(fontname.c_str()));
		if (pat)
		{
			FcConfigSubstitute(config, pat, FcMatchPattern);
			FcDefaultSubstitute(pat);
			FcResult res;
			FcPattern* font = FcFontMatch(config, pat, &res);
			if (font)
			{
				FcChar8* file = nullptr;
				if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
					filename = (const char*)file;
				FcPatternDestroy(font);
			}
			FcPatternDestroy(pat);
		}
	}
	if (filename.empty())
		throw std::runtime_error("Could not find font filename for: " + fontname);

	SingleFontData fontdata;
	fontdata.fontdata = ReadAllBytes(filename);
	return { std::move(fontdata) };
}

std::vector<SingleFontData> ResourceData::LoadSystemFont()
{
	return GetGtkUIFont("font-name");
}

std::vector<SingleFontData> ResourceData::LoadMonospaceSystemFont()
{
	return GetGtkUIFont("monospace-font-name");
}

double ResourceData::GetSystemFontSize()
{
	return 11.0;
}

class ResourceLoaderUnix : public ResourceLoader
{
public:
	std::vector<SingleFontData> LoadFont(const std::string& name) override
	{
		if (name == "system")
			return ResourceData::LoadSystemFont();
		else if (name == "monospace")
			return ResourceData::LoadMonospaceSystemFont();
		else
			return { SingleFontData{ReadAllBytes(name + ".ttf"), ""} };
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& filename) override
	{
		return ::ReadAllBytes(filename);
	}
};

struct ResourceDefaultLoader
{
	ResourceDefaultLoader() { loader = std::make_unique<ResourceLoaderUnix>(); }
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
