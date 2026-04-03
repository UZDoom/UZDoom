
#include "core/theme.h"
#include "core/widget.h"
#include "core/canvas.h"
#include "core/font.h"
#include "theme_style_tokenizer.h"
#include <stdexcept>

void WidgetStyle::SetBool(const std::string& state, const std::string& propertyName, bool value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetInt(const std::string& state, const std::string& propertyName, int value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetDouble(const std::string& state, const std::string& propertyName, double value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetString(const std::string& state, const std::string& propertyName, const std::string& value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetColor(const std::string& state, const std::string& propertyName, const Colorf& value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetImage(const std::string& state, const std::string& propertyName, const std::shared_ptr<Image>& value)
{
	StyleProperties[state][propertyName] = value;
}

const WidgetStyle::PropertyVariant* WidgetStyle::FindProperty(const std::string& state, const std::string& propertyName) const
{
	const WidgetStyle* style = this;
	do
	{
		// Look for property in the specific state
		auto stateIt = style->StyleProperties.find(state);
		if (stateIt != style->StyleProperties.end())
		{
			auto it = stateIt->second.find(propertyName);
			if (it != stateIt->second.end())
				return &it->second;
		}

		// Fall back to the widget main style
		if (state != std::string())
		{
			stateIt = style->StyleProperties.find(std::string());
			if (stateIt != style->StyleProperties.end())
			{
				auto it = stateIt->second.find(propertyName);
				if (it != stateIt->second.end())
					return &it->second;
			}
		}

		style = style->ParentStyle;
	} while (style);
	return nullptr;
}

std::shared_ptr<Font> WidgetStyle::GetFont(const std::string& state)
{
	auto& font = Fonts[state];
	if (!font)
	{
		std::string fontName = GetString(state, "font-family");
		double size = GetDouble(state, "font-size");
		font = Font::Create(fontName, size);
	}
	return font;
}

bool WidgetStyle::GetBool(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<bool>(*prop) : false;
}

int WidgetStyle::GetInt(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<int>(*prop) : 0;
}

double WidgetStyle::GetDouble(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<double>(*prop) : 0.0;
}

std::string WidgetStyle::GetString(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<std::string>(*prop) : std::string();
}

Colorf WidgetStyle::GetColor(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<Colorf>(*prop) : Colorf::transparent();
}

std::shared_ptr<Image> WidgetStyle::GetImage(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<std::shared_ptr<Image>>(*prop) : std::shared_ptr<Image>();
}

/////////////////////////////////////////////////////////////////////////////

void BasicWidgetStyle::Paint(Widget* widget, Canvas* canvas, Size size)
{
	Colorf bgcolor = widget->GetStyleColor("background-color");
	if (bgcolor.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, size.width, size.height), bgcolor);

	Colorf borderleft = widget->GetStyleColor("border-left-color");
	Colorf bordertop = widget->GetStyleColor("border-top-color");
	Colorf borderright = widget->GetStyleColor("border-right-color");
	Colorf borderbottom = widget->GetStyleColor("border-bottom-color");

	double borderwidth = widget->GridFitSize(1.0);

	if (bordertop.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, size.width, borderwidth), bordertop);
	if (borderbottom.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, size.height - borderwidth, size.width, borderwidth), borderbottom);
	if (borderleft.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, borderwidth, size.height), borderleft);
	if (borderright.a > 0.0f)
		canvas->fillRect(Rect::xywh(size.width - borderwidth, 0.0, borderwidth, size.height), borderright);

	auto image = widget->GetStyleImage("border-image-source");
	if (image)
	{
		BorderGeometry geo;
		geo.box = Rect::xywh(0.0, 0.0, size.width, size.height);
		geo.border.left = widget->GetStyleDouble("border-left-width");
		geo.border.right = widget->GetStyleDouble("border-right-width");
		geo.border.top = widget->GetStyleDouble("border-top-width");
		geo.border.bottom = widget->GetStyleDouble("border-bottom-width");

		BorderImage style;
		style.source = image;
		style.width.left = widget->GetStyleDouble("border-left-image-width");
		style.width.top = widget->GetStyleDouble("border-top-image-width");
		style.width.right = widget->GetStyleDouble("border-right-image-width");
		style.width.bottom = widget->GetStyleDouble("border-bottom-image-width");
		style.slice.left = BorderImageValue(widget->GetStyleDouble("border-left-image-slice"), BorderImageValueType::number);
		style.slice.top = BorderImageValue(widget->GetStyleDouble("border-top-image-slice"), BorderImageValueType::number);
		style.slice.right = BorderImageValue(widget->GetStyleDouble("border-right-image-slice"), BorderImageValueType::number);
		style.slice.bottom = BorderImageValue(widget->GetStyleDouble("border-bottom-image-slice"), BorderImageValueType::number);
		style.slice.center = widget->GetStyleBool("border-center-image-slice");

		BorderImageRenderer::render(canvas, geo, style);
	}
}

/////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<WidgetTheme> CurrentTheme;

WidgetStyle* WidgetTheme::RegisterStyle(std::unique_ptr<WidgetStyle> widgetStyle, const std::string& widgetClass)
{
	auto& style = Styles[widgetClass];
	style = std::move(widgetStyle);
	return style.get();
}

WidgetStyle* WidgetTheme::GetStyle(const std::string& widgetClass)
{
	auto it = Styles.find(widgetClass);
	if (it != Styles.end())
		return it->second.get();
	if (widgetClass != "widget")
		return GetStyle("widget");
	return nullptr;
}

void WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme> theme)
{
	CurrentTheme = std::move(theme);
}

WidgetTheme* WidgetTheme::GetTheme()
{
	return CurrentTheme.get();
}

/////////////////////////////////////////////////////////////////////////////

SimpleTheme::SimpleTheme(const ThemeColors& colors)
{
	auto bgMain   = colors.bgMain;   // background
	auto fgMain   = colors.fgMain;   //
	auto bgLight  = colors.bgLight;  // headers / inputs
	auto fgLight  = colors.fgLight;  //
	auto bgAction = colors.bgAction; // interactive elements
	auto fgAction = colors.fgAction; //
	auto bgHover  = colors.bgHover;  // hover / highlight
	auto fgHover  = colors.fgHover;  //
	auto bgActive = colors.bgActive; // click
	auto fgActive = colors.fgActive; //
	auto border   = colors.border;   // around elements
	auto divider  = colors.divider;  // between elements

	auto none   = Colorf::transparent();

	auto widget = RegisterStyle(std::make_unique<BasicWidgetStyle>(), "widget");
	/*auto textlabel =*/ RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "textlabel");
	auto pushbutton = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "pushbutton");
	auto lineedit = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "lineedit");
	auto textedit = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "textedit");
	auto listview = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "listview");
	auto listviewheader = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "listview-header");
	auto listviewbody = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "listview-body");
	auto dropdown = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "dropdown");
	auto scrollbar = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "scrollbar");
	auto tabbar = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "tabbar");
	auto tabbar_tab = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "tabbar-tab");
	auto tabbar_spacer = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "tabbar-spacer");
	auto tabwidget_stack = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "tabwidget-stack");
	auto checkbox_label = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "checkbox-label");
	auto menubar = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "menubar");
	auto menubaritem = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "menubaritem");
	auto menu = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "menu");
	auto menuitem = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "menuitem");
	auto toolbar = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "toolbar");
	auto toolbarbutton = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "toolbarbutton");
	auto statusbar = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), "statusbar");

	widget->SetString("font-family", "system");
	widget->SetDouble("font-size", 13.0);
	widget->SetColor("color", fgMain);
	widget->SetColor("window-background", bgMain);
	widget->SetColor("window-border", bgMain);
	widget->SetColor("window-caption-color", bgLight);
	widget->SetColor("window-caption-text-color", fgLight);

	pushbutton->SetDouble("noncontent-left", 10.0);
	pushbutton->SetDouble("noncontent-top", 5.0);
	pushbutton->SetDouble("noncontent-right", 10.0);
	pushbutton->SetDouble("noncontent-bottom", 5.0);
	pushbutton->SetColor("color", fgAction);

#if 1
	pushbutton->SetColor("background-color", bgAction);
	pushbutton->SetColor("border-left-color", border);
	pushbutton->SetColor("border-top-color", border);
	pushbutton->SetColor("border-right-color", border);
	pushbutton->SetColor("border-bottom-color", border);
	pushbutton->SetColor("hover", "color", fgHover);
	pushbutton->SetColor("hover", "background-color", bgHover);
	pushbutton->SetColor("down", "color", fgActive);
	pushbutton->SetColor("down", "background-color", bgActive);
#else
	pushbutton->SetImage("border-image-source", Image::LoadResource("button_normal.png"));
	pushbutton->SetImage("hover", "border-image-source", Image::LoadResource("button_hot.png"));
	pushbutton->SetDouble("border-top-width", 6.0);
	pushbutton->SetDouble("border-bottom-width", 6.0);
	pushbutton->SetDouble("border-left-width", 5.0);
	pushbutton->SetDouble("border-right-width", 5.0);
	pushbutton->SetDouble("border-top-image-slice", 6.0);
	pushbutton->SetDouble("border-bottom-image-slice", 6.0);
	pushbutton->SetDouble("border-left-image-slice", 5.0);
	pushbutton->SetDouble("border-right-image-slice", 5.0);
	pushbutton->SetDouble("border-top-image-width", 6.0);
	pushbutton->SetDouble("border-bottom-image-width", 6.0);
	pushbutton->SetDouble("border-left-image-width", 5.0);
	pushbutton->SetDouble("border-right-image-width", 5.0);
	pushbutton->SetBool("border-center-image-slice", true);
#endif

	lineedit->SetDouble("noncontent-left", 5.0);
	lineedit->SetDouble("noncontent-top", 3.0);
	lineedit->SetDouble("noncontent-right", 5.0);
	lineedit->SetDouble("noncontent-bottom", 3.0);
	lineedit->SetColor("color", fgLight);
	lineedit->SetColor("background-color", bgLight);
	lineedit->SetColor("border-left-color", border);
	lineedit->SetColor("border-top-color", border);
	lineedit->SetColor("border-right-color", border);
	lineedit->SetColor("border-bottom-color", border);
	lineedit->SetColor("selection-color", bgHover);
	lineedit->SetColor("no-focus-selection-color", bgHover);

	textedit->SetString("font-family", "monospace");
	textedit->SetDouble("noncontent-left", 8.0);
	textedit->SetDouble("noncontent-top", 8.0);
	textedit->SetDouble("noncontent-right", 8.0);
	textedit->SetDouble("noncontent-bottom", 8.0);
	textedit->SetColor("color", fgLight);
	textedit->SetColor("background-color", bgLight);
	textedit->SetColor("border-left-color", border);
	textedit->SetColor("border-top-color", border);
	textedit->SetColor("border-right-color", border);
	textedit->SetColor("border-bottom-color", border);
	textedit->SetColor("selection-color", bgHover);

	listview->SetDouble("noncontent-left", 10.0);
	listview->SetDouble("noncontent-top", 10.0);
	listview->SetDouble("noncontent-right", 3.0);
	listview->SetDouble("noncontent-bottom", 10.0);
	listview->SetColor("background-color", bgLight);
	listview->SetColor("border-left-color", border);
	listview->SetColor("border-top-color", border);
	listview->SetColor("border-right-color", border);
	listview->SetColor("border-bottom-color", border);

	listviewheader->SetDouble("noncontent-left", 2.0);
	listviewheader->SetDouble("noncontent-right", 2.0);
	listviewheader->SetColor("background-color", bgAction);
	listviewheader->SetColor("color", fgLight);

	listviewbody->SetDouble("noncontent-top", 2.0);
	listviewbody->SetColor("color", fgLight);
	listviewbody->SetColor("selection-color", bgHover);

	dropdown->SetDouble("noncontent-left", 5.0);
	dropdown->SetDouble("noncontent-top", 5.0);
	dropdown->SetDouble("noncontent-right", 5.0);
	dropdown->SetDouble("noncontent-bottom", 5.0);
	dropdown->SetColor("color", fgLight);
	dropdown->SetColor("background-color", bgLight);
	dropdown->SetColor("border-left-color", border);
	dropdown->SetColor("border-top-color", border);
	dropdown->SetColor("border-right-color", border);
	dropdown->SetColor("border-bottom-color", border);
	dropdown->SetColor("arrow-color", border);

	scrollbar->SetColor("track-color", divider);
	scrollbar->SetColor("thumb-color", border);

	tabbar->SetDouble("spacer-left", 20.0);
	tabbar->SetDouble("spacer-right", 20.0);
	tabbar->SetColor("background-color", bgLight);

	tabbar_tab->SetDouble("noncontent-left", 15.0);
	tabbar_tab->SetDouble("noncontent-right", 15.0);
	tabbar_tab->SetDouble("noncontent-top", 1.0);
	tabbar_tab->SetDouble("noncontent-bottom", 1.0);
	tabbar_tab->SetColor("color", fgMain);
	tabbar_tab->SetColor("background-color", bgMain);
	tabbar_tab->SetColor("border-left-color", divider);
	tabbar_tab->SetColor("border-top-color", divider);
	tabbar_tab->SetColor("border-right-color", divider);
	tabbar_tab->SetColor("border-bottom-color", border);
	tabbar_tab->SetColor("hover", "color", fgAction);
	tabbar_tab->SetColor("hover", "background-color", bgAction);
	tabbar_tab->SetColor("active", "background-color", bgMain);
	tabbar_tab->SetColor("active", "border-left-color", border);
	tabbar_tab->SetColor("active", "border-top-color", border);
	tabbar_tab->SetColor("active", "border-right-color", border);
	tabbar_tab->SetColor("active", "border-bottom-color", none);

	tabbar_spacer->SetDouble("noncontent-bottom", 1.0);
	tabbar_spacer->SetColor("border-bottom-color", border);

	tabwidget_stack->SetDouble("noncontent-left", 20.0);
	tabwidget_stack->SetDouble("noncontent-top", 5.0);
	tabwidget_stack->SetDouble("noncontent-right", 20.0);
	tabwidget_stack->SetDouble("noncontent-bottom", 5.0);

	checkbox_label->SetColor("checked-outer-border-color", border);
	checkbox_label->SetColor("checked-inner-border-color", bgMain);
	checkbox_label->SetColor("checked-color", fgMain);
	checkbox_label->SetColor("unchecked-outer-border-color", border);
	checkbox_label->SetColor("unchecked-inner-border-color", bgMain);

	menubar->SetColor("background-color", bgLight);
	toolbar->SetColor("background-color", bgLight);
	statusbar->SetColor("background-color", bgLight);

	toolbarbutton->SetColor("hover", "color", fgHover);
	toolbarbutton->SetColor("hover", "background-color", bgHover);
	toolbarbutton->SetColor("down", "color", fgActive);
	toolbarbutton->SetColor("down", "background-color", bgActive);

	menubaritem->SetColor("color", fgMain);
	menubaritem->SetColor("hover", "color", fgHover);
	menubaritem->SetColor("hover", "background-color", bgHover);
	menubaritem->SetColor("down", "color", fgActive);
	menubaritem->SetColor("down", "background-color", bgActive);

	menu->SetDouble("noncontent-left", 5.0);
	menu->SetDouble("noncontent-top", 5.0);
	menu->SetDouble("noncontent-right", 5.0);
	menu->SetDouble("noncontent-bottom", 5.0);
	menu->SetColor("color", fgMain);
	menu->SetColor("background-color", bgMain);
	menu->SetColor("border-left-color", border);
	menu->SetColor("border-top-color", border);
	menu->SetColor("border-right-color", border);
	menu->SetColor("border-bottom-color", border);

	menuitem->SetColor("hover", "color", fgHover);
	menuitem->SetColor("hover", "background-color", bgHover);
	menuitem->SetColor("down", "color", fgActive);
	menuitem->SetColor("down", "background-color", bgActive);
}

/////////////////////////////////////////////////////////////////////////////

DarkWidgetTheme::DarkWidgetTheme() : SimpleTheme({
	Colorf::fromRgb(0x2A2A2A), // background
	Colorf::fromRgb(0xE2DFDB), //
	Colorf::fromRgb(0x212121), // headers / inputs
	Colorf::fromRgb(0xE2DFDB), //
	Colorf::fromRgb(0x444444), // interactive elements
	Colorf::fromRgb(0xFFFFFF), //
	Colorf::fromRgb(0x003C88), // hover / highlight
	Colorf::fromRgb(0xFFFFFF), //
	Colorf::fromRgb(0x004FB4), // click
	Colorf::fromRgb(0xFFFFFF), //
	Colorf::fromRgb(0x646464), // around elements
	Colorf::fromRgb(0x555555)  // between elements
	})
{
}

/////////////////////////////////////////////////////////////////////////////

LightWidgetTheme::LightWidgetTheme() : SimpleTheme({
	Colorf::fromRgb(0xF0F0F0), // background
	Colorf::fromRgb(0x191919), //
	Colorf::fromRgb(0xFAFAFA), // headers / inputs
	Colorf::fromRgb(0x191919), //
	Colorf::fromRgb(0xC8C8C8), // interactive elements
	Colorf::fromRgb(0x000000), //
	Colorf::fromRgb(0xD2D2FF), // hover / highlight
	Colorf::fromRgb(0x000000), //
	Colorf::fromRgb(0xC7B4FF), // click
	Colorf::fromRgb(0x000000), //
	Colorf::fromRgb(0xA0A0A0), // around elements
	Colorf::fromRgb(0xB9B9B9)  // between elements
	})
{
}

/////////////////////////////////////////////////////////////////////////////

static ThemeStyleToken next_token(size_t& pos, const std::vector<ThemeStyleToken>& tokens, bool skip_whitespace = true)
{
	ThemeStyleToken token;
	do
	{
		if (pos != tokens.size())
		{
			token = tokens[pos];
			pos++;
		}
		else
		{
			token = ThemeStyleToken();
		}
	} while (token.type == ThemeStyleTokenType::whitespace);
	return token;
}

static bool equals(const std::string& a, const std::string& b)
{
	return ThemeStyleTokenizer::compare_case_insensitive(a, b);
}

static bool parse_color(const std::vector<ThemeStyleToken>& tokens, size_t& in_out_pos, Colorf& out_color)
{
	size_t pos = in_out_pos;
	ThemeStyleToken token = next_token(pos, tokens);
	if (token.type == ThemeStyleTokenType::ident)
	{
		if (equals(token.value, "transparent"))
		{
			out_color = Colorf(0.0f, 0.0f, 0.0f, 0.0f);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStyleTokenType::function && equals(token.value, "rgb"))
	{
		int color[3] = { 0, 0, 0 };
		for (int channel = 0; channel < 3; channel++)
		{
			token = next_token(pos, tokens);
			if (token.type == ThemeStyleTokenType::number)
			{
				int value = std::atoi(token.value.c_str());
				value = std::min(255, value);
				value = std::max(0, value);
				color[channel] = value;
			}
			else if (token.type == ThemeStyleTokenType::percentage)
			{
				float value = (float)std::atof(token.value.c_str()) / 100.0f;
				value = std::min(1.0f, value);
				value = std::max(0.0f, value);
				color[channel] = (int)(value * 255.0f);
			}
			else
			{
				return false;
			}

			if (channel < 2)
			{
				token = next_token(pos, tokens);
				if (token.type != ThemeStyleTokenType::delim || token.value != ",")
					return false;
			}
		}
		token = next_token(pos, tokens);
		if (token.type == ThemeStyleTokenType::bracket_end)
		{
			out_color = Colorf(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, 1.0f);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStyleTokenType::function && equals(token.value, "rgba"))
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int channel = 0; channel < 4; channel++)
		{
			token = next_token(pos, tokens);
			if (token.type == ThemeStyleTokenType::number)
			{
				if (channel < 3)
				{
					int value = std::atoi(token.value.c_str());
					value = std::min(255, value);
					value = std::max(0, value);
					color[channel] = value / 255.0f;
				}
				else
				{
					color[channel] = (float)std::atof(token.value.c_str());
				}
			}
			else if (token.type == ThemeStyleTokenType::percentage)
			{
				float value = (float)std::atof(token.value.c_str()) / 100.0f;
				value = std::min(1.0f, value);
				value = std::max(0.0f, value);
				color[channel] = value;
			}
			else
			{
				return false;
			}

			if (channel < 3)
			{
				token = next_token(pos, tokens);
				if (token.type != ThemeStyleTokenType::delim || token.value != ",")
					return false;
			}
		}
		token = next_token(pos, tokens);
		if (token.type == ThemeStyleTokenType::bracket_end)
		{
			out_color = Colorf(color[0], color[1], color[2], color[3]);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStyleTokenType::hash)
	{
		if (token.value.length() == 3)
		{
			float channels[3] = { 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 3; c++)
			{
				int v = 0;
				if (token.value[c] >= '0' && token.value[c] <= '9')
					v = token.value[c] - '0';
				else if (token.value[c] >= 'a' && token.value[c] <= 'f')
					v = token.value[c] - 'a' + 10;
				else if (token.value[c] >= 'A' && token.value[c] <= 'F')
					v = token.value[c] - 'A' + 10;
				else
					return false;
				v = (v << 4) + v;
				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 4)
		{
			float channels[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 4; c++)
			{
				int v = 0;
				if (token.value[c] >= '0' && token.value[c] <= '9')
					v = token.value[c] - '0';
				else if (token.value[c] >= 'a' && token.value[c] <= 'f')
					v = token.value[c] - 'a' + 10;
				else if (token.value[c] >= 'A' && token.value[c] <= 'F')
					v = token.value[c] - 'A' + 10;
				else
					return false;
				v = (v << 4) + v;
				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2], channels[3]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 6)
		{
			float channels[3] = { 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 3; c++)
			{
				int v = 0;
				if (token.value[c * 2] >= '0' && token.value[c * 2] <= '9')
					v = token.value[c * 2] - '0';
				else if (token.value[c * 2] >= 'a' && token.value[c * 2] <= 'f')
					v = token.value[c * 2] - 'a' + 10;
				else if (token.value[c * 2] >= 'A' && token.value[c * 2] <= 'F')
					v = token.value[c * 2] - 'A' + 10;
				else
					return false;

				v <<= 4;

				if (token.value[c * 2 + 1] >= '0' && token.value[c * 2 + 1] <= '9')
					v += token.value[c * 2 + 1] - '0';
				else if (token.value[c * 2 + 1] >= 'a' && token.value[c * 2 + 1] <= 'f')
					v += token.value[c * 2 + 1] - 'a' + 10;
				else if (token.value[c * 2 + 1] >= 'A' && token.value[c * 2 + 1] <= 'F')
					v += token.value[c * 2 + 1] - 'A' + 10;
				else
					return false;

				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 8)
		{
			float channels[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 4; c++)
			{
				int v = 0;
				if (token.value[c * 2] >= '0' && token.value[c * 2] <= '9')
					v = token.value[c * 2] - '0';
				else if (token.value[c * 2] >= 'a' && token.value[c * 2] <= 'f')
					v = token.value[c * 2] - 'a' + 10;
				else if (token.value[c * 2] >= 'A' && token.value[c * 2] <= 'F')
					v = token.value[c * 2] - 'A' + 10;
				else
					return false;

				v <<= 4;

				if (token.value[c * 2 + 1] >= '0' && token.value[c * 2 + 1] <= '9')
					v += token.value[c * 2 + 1] - '0';
				else if (token.value[c * 2 + 1] >= 'a' && token.value[c * 2 + 1] <= 'f')
					v += token.value[c * 2 + 1] - 'a' + 10;
				else if (token.value[c * 2 + 1] >= 'A' && token.value[c * 2 + 1] <= 'F')
					v += token.value[c * 2 + 1] - 'A' + 10;
				else
					return false;

				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2], channels[3]);
			in_out_pos = pos;
			return true;
		}
	}

	return false;
}

class ThemeStyleSelector // css selector link
{
public:
	std::string widget; // css element
	std::string theme; // css class
	std::string state; // css pseudo class
};

class ThemeStyleProperty
{
public:
	std::string name;
	std::vector<ThemeStyleToken> value;
	bool importantFlag = false;
};

class ThemeStyleRuleset
{
public:
	std::vector<ThemeStyleSelector> selectors;
	std::vector<ThemeStyleProperty> properties;
};

class ThemeSortedSelector
{
public:
	size_t specificity = 0;
	ThemeStyleSelector* selector = nullptr;
	ThemeStyleRuleset* ruleset = nullptr;

	bool operator<(const ThemeSortedSelector& other) const { return specificity < other.specificity; }
};

class ThemeStyleDocument
{
public:
	ThemeStyleDocument(const std::string& stylesheet)
	{
		read_stylesheet(stylesheet);
		create_sorted_selectors();
	}

	void create_sorted_selectors()
	{
		size_t a = 0;
		for (ThemeStyleRuleset& ruleset : rulesets)
		{
			for (ThemeStyleSelector& selector : ruleset.selectors)
			{
				size_t b = 0; // #id, not used
				size_t c = 0;
				size_t d = (selector.widget != "*") ? 1 : 0;
				if (selector.theme.empty())
					c++;
				if (selector.state.empty())
					c++;

				ThemeSortedSelector s;
				s.selector = &selector;
				s.ruleset = &ruleset;
				s.specificity = (a << 32) | (b << 24) | (c << 8) | d;
				selectors.push_back(std::move(s));
			}
			a++;
		}
		std::stable_sort(selectors.begin(), selectors.end());
	}

	void read_stylesheet(const std::string& stylesheet)
	{
		ThemeStyleTokenizer tokenizer(stylesheet);
		ThemeStyleToken token;

		while (true)
		{
			tokenizer.read(token, true);
			if (token.type == ThemeStyleTokenType::null)
			{
				break;
			}
			else if (token.type == ThemeStyleTokenType::atkeyword)
			{
				read_at_rule(tokenizer, token);
			}
			else if (token.type != ThemeStyleTokenType::whitespace)
			{
				read_statement(tokenizer, token);
			}
		}
	}

	void read_statement(ThemeStyleTokenizer& tokenizer, ThemeStyleToken& token)
	{
		ThemeStyleRuleset ruleset;
		while (true)
		{
			ThemeStyleSelector selector;
			if (read_selector(tokenizer, token, selector))
			{
				ruleset.selectors.push_back(std::move(selector));
				if (token.type == ThemeStyleTokenType::delim && token.value == ",")
				{
					tokenizer.read(token, true);
				}
				else if (token.type == ThemeStyleTokenType::curly_brace_begin)
				{
					break;
				}
				else
				{
					read_end_of_statement(tokenizer, token);
					return;
				}
			}
			else
			{
				read_end_of_statement(tokenizer, token);
				return;
			}
		}

		if (token.type == ThemeStyleTokenType::curly_brace_begin)
		{
			while (true)
			{
				tokenizer.read(token, true);

				ThemeStyleProperty prop;
				if (token.type == ThemeStyleTokenType::delim && token.value == "-")
				{
					tokenizer.read(token, true);
					if (token.type == ThemeStyleTokenType::ident)
					{
						prop.name = "-" + token.value;
						tokenizer.read(token, true);
					}
					else
					{
						bool important_flag = false;
						tokenizer.read_property_value(token, important_flag);
					}
				}
				else if (token.type == ThemeStyleTokenType::ident)
				{
					prop.name = token.value;
					tokenizer.read(token, true);
				}
				else if (token.type == ThemeStyleTokenType::curly_brace_end)
				{
					break;
				}
				else if (token.type == ThemeStyleTokenType::null)
				{
					return;
				}

				if (token.type == ThemeStyleTokenType::colon)
				{
					tokenizer.read(token, true);
					prop.value = tokenizer.read_property_value(token, prop.importantFlag);
					ruleset.properties.push_back(std::move(prop));
				}
				else
				{
					bool important_flag = false;
					tokenizer.read_property_value(token, important_flag);
				}
			}
			rulesets.push_back(std::move(ruleset));
		}
		else
		{
			read_end_of_statement(tokenizer, token);
		}
	}

	bool read_selector(ThemeStyleTokenizer& tokenizer, ThemeStyleToken& token, ThemeStyleSelector& out_selector)
	{
		while (token.type == ThemeStyleTokenType::whitespace)
			tokenizer.read(token, true);

		ThemeStyleSelector selector_link;
		if (token.type == ThemeStyleTokenType::ident)
		{
			// Simple Selector
			selector_link.widget = token.value;
			tokenizer.read(token, false);
		}
		else if (token.type == ThemeStyleTokenType::delim && token.value == "*")
		{
			// Universal Selector
			selector_link.widget = "*";
			tokenizer.read(token, false);
		}
		else if (token.type == ThemeStyleTokenType::hash ||
			token.type == ThemeStyleTokenType::colon ||
			token.type == ThemeStyleTokenType::square_bracket_begin ||
			(token.type == ThemeStyleTokenType::delim && token.value == "."))
		{
			// Implicit Universal Selector
			selector_link.widget = "*";
		}
		else
		{
			return false;
		}

		while (true)
		{
			if (token.type == ThemeStyleTokenType::colon)
			{
				tokenizer.read(token, false);
				if (token.type == ThemeStyleTokenType::ident)
				{
					if (!selector_link.state.empty())
						return false;
					selector_link.state = token.value;
				}
			}
			else if (token.type == ThemeStyleTokenType::delim && token.value == ".")
			{
				tokenizer.read(token, false);
				if (token.type == ThemeStyleTokenType::ident)
				{
					if (!selector_link.theme.empty())
						return false;
					selector_link.theme = token.value;
				}
				else
				{
					return false;
				}
			}
			else
			{
				break;
			}

			tokenizer.read(token, false);
		}

		while (token.type == ThemeStyleTokenType::whitespace)
			tokenizer.read(token, true);

		out_selector = selector_link;
		return true;
	}

	bool read_end_of_statement(ThemeStyleTokenizer& tokenizer, ThemeStyleToken& token)
	{
		int curly_count = 0;
		while (true)
		{
			if (token.type == ThemeStyleTokenType::null)
			{
				break;
			}
			else if (token.type == ThemeStyleTokenType::curly_brace_begin)
			{
				curly_count++;
			}
			else if (token.type == ThemeStyleTokenType::curly_brace_end)
			{
				curly_count--;
				if (curly_count <= 0)
					break;
			}
			else if (token.type == ThemeStyleTokenType::semi_colon)
			{
				if (curly_count == 0)
					break;
			}
			tokenizer.read(token, true);
		}

		return curly_count < 0;
	}

	void read_at_rule(ThemeStyleTokenizer& tokenizer, ThemeStyleToken& token)
	{
		// We have no at rules currently - skip it
		read_end_of_at_rule(tokenizer, token);
	}

	void read_end_of_at_rule(ThemeStyleTokenizer& tokenizer, ThemeStyleToken& token)
	{
		int curly_count = 0;
		while (true)
		{
			if (token.type == ThemeStyleTokenType::null)
			{
				break;
			}
			else if (token.type == ThemeStyleTokenType::semi_colon && curly_count == 0)
			{
				break;
			}
			else if (token.type == ThemeStyleTokenType::curly_brace_begin)
			{
				curly_count++;
			}
			else if (token.type == ThemeStyleTokenType::curly_brace_end)
			{
				curly_count--;
				if (curly_count == 0)
					break;
			}
			tokenizer.read(token, false);
		}
	}

	std::vector<ThemeStyleRuleset> rulesets;
	std::vector<ThemeSortedSelector> selectors;
};

StylesheetTheme::StylesheetTheme(const std::string& stylesheet, const std::string& usedThemeClass)
{
	ThemeStyleDocument doc(stylesheet);

	std::unordered_map<std::string, WidgetStyle*> styles;
	std::unordered_map<std::string, std::vector<ThemeStyleToken>> variables;
	auto widget = RegisterStyle(std::make_unique<BasicWidgetStyle>(), "widget");
	styles["widget"] = widget;

	for (const ThemeSortedSelector& s : doc.selectors)
	{
		if (!s.selector->theme.empty() && s.selector->theme != usedThemeClass)
			continue; // Not this theme

		if (s.selector->widget == "*")
		{
			// Only support variable declarations in universal selectors for now
			for (const ThemeStyleProperty& prop : s.ruleset->properties)
			{
				if (prop.name.size() > 2 && prop.name.substr(0, 2) == "--")
				{
					variables[prop.name] = prop.value;
				}
			}
		}
		else
		{
			std::string widgetName = s.selector->widget;
			std::string partName = s.selector->state;

			WidgetStyle* style = styles[widgetName];
			if (!style)
			{
				style = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), widgetName);
				styles[widgetName] = style;
			}

			for (const ThemeStyleProperty& prop : s.ruleset->properties)
			{
				if (prop.name.size() > 2 && prop.name.substr(0, 2) == "--")
					continue; // skip variable declarations

				const std::string& name = prop.name;

				// Grab property value tokens
				std::vector<ThemeStyleToken> origtokens = prop.value;

				// Insert variables (to do: move to function and clean this up!)
				std::vector<ThemeStyleToken> tokens;
				for (auto it = origtokens.begin(); it != origtokens.end(); ++it)
				{
					const ThemeStyleToken& t = *it;
					if (t.type == ThemeStyleTokenType::function && t.value == "var")
					{
						++it;
						if (it == origtokens.end() || (*it).type != ThemeStyleTokenType::delim || (*it).value != "-")
						{
							tokens.clear();
							break;
						}

						++it;
						if (it == origtokens.end() || (*it).type != ThemeStyleTokenType::ident)
						{
							tokens.clear();
							break;
						}
						const auto& variable = variables["-" + (*it).value];
						tokens.insert(tokens.end(), variable.begin(), variable.end());
						++it;
						if (it == origtokens.end() || (*it).type != ThemeStyleTokenType::bracket_end)
						{
							tokens.clear();
							break;
						}
					}
					else
					{
						tokens.push_back(t);
					}
				}

				if (tokens.empty())
					continue;

				// Parse the tokens:
				// To do: maybe use property parsers like ClanLib and UICore does?
				// That would allow for short form properties

				if (tokens[0].type == ThemeStyleTokenType::ident && (tokens[0].value == "true" || tokens[0].value == "false"))
				{
					if (tokens[0].value == "true")
					{
						style->SetBool(partName, name, true);
					}
					else if (tokens[0].value == "false")
					{
						style->SetBool(partName, name, false);
					}
				}
				else if (tokens[0].type == ThemeStyleTokenType::number)
				{
					style->SetDouble(partName, name, std::atof(tokens[0].value.c_str()));
				}
				else if (tokens.size() == 2 && tokens[0].type == ThemeStyleTokenType::delim && tokens[0].value == "-" && tokens[1].type == ThemeStyleTokenType::number)
				{
					style->SetDouble(partName, name, -std::atof(tokens[1].value.c_str()));
				}
				else if (tokens[0].type == ThemeStyleTokenType::string)
				{
					style->SetString(partName, name, tokens[0].value);
				}
				else if (tokens[0].type == ThemeStyleTokenType::uri)
				{
					style->SetImage(partName, name, Image::LoadResource(tokens[0].value));
				}
				else
				{
					size_t pos = 0;
					Colorf color;
					if (parse_color(tokens, pos, color))
					{
						style->SetColor(partName, name, color);
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void BorderImageRenderer::render(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style)
{
	BorderImageRenderer renderer(canvas, geometry, style);
	renderer.render();
}

BorderImageRenderer::BorderImageRenderer(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style) : canvas(canvas), geometry(geometry), style(style)
{
}

void BorderImageRenderer::render()
{
	std::shared_ptr<Image> image = style.source;
	if (image)
	{
		int slice_left = get_left_slice_value(image->GetWidth());
		int slice_right = get_right_slice_value(image->GetWidth());
		int slice_top = get_top_slice_value(image->GetHeight());
		int slice_bottom = get_bottom_slice_value(image->GetHeight());
		bool fill_center = style.slice.center;

		Box border_image_area = get_border_image_area();

		float grid_left = get_left_grid(border_image_area.get_width(), (float)slice_left);
		float grid_right = get_right_grid(border_image_area.get_width(), (float)slice_right);
		float grid_top = get_top_grid(border_image_area.get_height(), (float)slice_top);
		float grid_bottom = get_bottom_grid(border_image_area.get_height(), (float)slice_bottom);

		float x[4] = { border_image_area.left, border_image_area.left + grid_left, border_image_area.right - grid_right, border_image_area.right };
		float y[4] = { border_image_area.top, border_image_area.top + grid_top, border_image_area.bottom - grid_bottom, border_image_area.bottom };
		int sx[4] = { 0, slice_left, image->GetWidth() - slice_right, image->GetWidth() };
		int sy[4] = { 0, slice_top, image->GetHeight() - slice_bottom, image->GetHeight() };

		BorderImageRepeat repeat_x = style.repeat.x;
		BorderImageRepeat repeat_y = style.repeat.y;

		for (int yy = 0; yy < 3; yy++)
		{
			for (int xx = 0; xx < 3; xx++)
			{
				if ((xx != 1 && yy != 1) || fill_center)
					draw_area(image, x[xx], y[yy], x[xx + 1] - x[xx], y[yy + 1] - y[yy], sx[xx], sy[yy], sx[xx + 1] - sx[xx], sy[yy + 1] - sy[yy], repeat_x, repeat_y);
			}
		}
	}
}

BorderImageRenderer::TileRepeatInfo BorderImageRenderer::repeat_info(float x, float w, int sw, BorderImageRepeat repeat)
{
	TileRepeatInfo info;

	if (sw <= 0)
		return info;

	if (repeat == BorderImageRepeat::repeat)
	{
		info.count = (int)std::ceil(w / sw);
		info.start = x + (w - info.count * sw) * 0.5f;
		info.width = (float)sw;
	}
	else if (repeat == BorderImageRepeat::stretch)
	{
		info.start = x;
		info.width = w;
		info.count = 1;
	}
	else if (repeat == BorderImageRepeat::round)
	{
		info.start = x;
		info.count = std::max((int)std::round(w / sw), 1);
		info.width = sw * w / (sw * info.count);
	}
	else if (repeat == BorderImageRepeat::space)
	{
		info.start = x;
		info.width = (float)sw;
		info.count = (int)std::floor(w / sw);
		if (info.count > 1)
			info.space = w / (info.count - 1) - sw;
		else if (info.count == 1)
			info.start = x + (w - info.count * sw) * 0.5f;
	}

	return info;
}

void BorderImageRenderer::draw_area(const std::shared_ptr<Image>& image, float x, float y, float w, float h, int sx, int sy, int sw, int sh, BorderImageRepeat repeat_x, BorderImageRepeat repeat_y)
{
	TileRepeatInfo tile_x = repeat_info(x, w, sw, repeat_x);
	TileRepeatInfo tile_y = repeat_info(y, h, sh, repeat_y);

	Box clip = Box::xywh(x, y, w, h);
	Box src = Box::xywh((float)sx, (float)sy, (float)sw, (float)sh);

	for (int yy = 0; yy < tile_y.count; yy++)
	{
		float top = tile_y.start + (tile_y.width + tile_y.space) * yy;
		float bottom = tile_y.start + (tile_y.width + tile_y.space) * (yy + 1) - tile_y.space;

		for (int xx = 0; xx < tile_x.count; xx++)
		{
			float left = tile_x.start + (tile_x.width + tile_x.space) * xx;
			float right = tile_x.start + (tile_x.width + tile_x.space) * (xx + 1) - tile_x.space;

			Box dest(left, top, right, bottom);

			Box dest_clipped = dest;
			dest_clipped.clip(clip);

			float tleft = (dest_clipped.left - dest.left) / dest.get_width();
			float tright = (dest_clipped.right - dest.left) / dest.get_width();
			float ttop = (dest_clipped.top - dest.top) / dest.get_height();
			float tbottom = (dest_clipped.bottom - dest.top) / dest.get_height();

			Box src_clipped(mix(src.left, src.right, tleft), mix(src.top, src.bottom, ttop), mix(src.left, src.right, tright), mix(src.top, src.bottom, tbottom));

			canvas->drawImage(
				image,
				Rect::ltrb(src_clipped.left, src_clipped.top, src_clipped.right, src_clipped.bottom),
				Rect::ltrb(dest_clipped.left, dest_clipped.top, dest_clipped.right, dest_clipped.bottom));
		}
	}
}

BorderImageRenderer::Box BorderImageRenderer::get_border_image_area() const
{
	Box box = geometry.box;

	BorderImageValue outset_left = style.outset.left;
	BorderImageValue outset_right = style.outset.right;
	BorderImageValue outset_top = style.outset.top;
	BorderImageValue outset_bottom = style.outset.bottom;

	if (outset_left.is_length() || outset_left.is_number())
		box.left -= outset_left.number();

	if (outset_right.is_length() || outset_right.is_number())
		box.right += outset_right.number();

	if (outset_top.is_length() || outset_top.is_number())
		box.top -= outset_top.number();

	if (outset_bottom.is_length() || outset_bottom.is_number())
		box.bottom += outset_bottom.number();

	return box;
}

float BorderImageRenderer::get_left_grid(float image_area_width, float auto_width) const
{
	BorderImageValue border_image_width = style.width.left;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_width / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.left;
	else
		return auto_width;
}

float BorderImageRenderer::get_right_grid(float image_area_width, float auto_width) const
{
	BorderImageValue border_image_width = style.width.right;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_width / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.right;
	else
		return auto_width;
}

float BorderImageRenderer::get_top_grid(float image_area_height, float auto_height) const
{
	BorderImageValue border_image_width = style.width.top;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_height / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.top;
	else
		return auto_height;
}

float BorderImageRenderer::get_bottom_grid(float image_area_height, float auto_height) const
{
	BorderImageValue border_image_width = style.width.bottom;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_height / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.bottom;
	else
		return auto_height;
}

int BorderImageRenderer::get_left_slice_value(int image_width) const
{
	BorderImageValue border_image_slice = style.slice.left;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_width / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_width, v));
}

int BorderImageRenderer::get_right_slice_value(int image_width) const
{
	BorderImageValue border_image_slice = style.slice.right;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_width / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_width, v));
}

int BorderImageRenderer::get_top_slice_value(int image_height) const
{
	BorderImageValue border_image_slice = style.slice.top;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_height / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_height, v));
}

int BorderImageRenderer::get_bottom_slice_value(int image_height) const
{
	BorderImageValue border_image_slice = style.slice.bottom;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_height / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_height, v));
}
