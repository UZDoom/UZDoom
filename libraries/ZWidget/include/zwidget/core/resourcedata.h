#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

struct SingleFontData
{
	std::vector<uint8_t> fontdata;
	std::string language;
};

class ResourceLoader
{
public:
	static ResourceLoader* Get();
	static void Set(std::unique_ptr<ResourceLoader> instance);

	virtual ~ResourceLoader() = default;
	virtual std::vector<SingleFontData> LoadFont(const std::string& name) = 0;
	virtual std::vector<uint8_t> ReadAllBytes(const std::string& filename) = 0;
};

class ResourceData
{
public:
	static std::vector<SingleFontData> LoadFont(const std::string& name) { return ResourceLoader::Get()->LoadFont(name); }
	static std::vector<uint8_t> ReadAllBytes(const std::string& filename) { return ResourceLoader::Get()->ReadAllBytes(filename); }

	static std::vector<SingleFontData> LoadSystemFont();
	static std::vector<SingleFontData> LoadMonospaceSystemFont();
	static double GetSystemFontSize();
};
