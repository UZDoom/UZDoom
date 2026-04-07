#pragma once

#include "core/font.h"

class FontImpl : public Font
{
public:
	FontImpl(const std::string& name, double height) : Name(name), Height(height)
	{
	}

	const std::string& GetName() const override
	{
		return Name;
	}

	double GetHeight() const override
	{
		return Height;
	}

	std::string Name;
	double Height = 0.0;
};
