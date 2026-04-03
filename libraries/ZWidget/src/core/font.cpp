
#include "font_impl.h"

std::shared_ptr<Font> Font::Create(const std::string& name, double height)
{
	return std::make_shared<FontImpl>(name, height);
}
