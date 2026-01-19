#pragma once

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>
#include "rect.h"
#include "colorf.h"

class Widget;
class Canvas;
class Font;
class Image;

class WidgetStyle
{
public:
	WidgetStyle(WidgetStyle* parentStyle = nullptr) : ParentStyle(parentStyle) { }

	virtual ~WidgetStyle() = default;
	virtual void Paint(Widget* widget, Canvas* canvas, Size size) = 0;

	std::shared_ptr<Font> GetFont(const std::string& state);

	void SetBool(const std::string& state, const std::string& propertyName, bool value);
	void SetInt(const std::string& state, const std::string& propertyName, int value);
	void SetDouble(const std::string& state, const std::string& propertyName, double value);
	void SetString(const std::string& state, const std::string& propertyName, const std::string& value);
	void SetColor(const std::string& state, const std::string& propertyName, const Colorf& value);
	void SetImage(const std::string& state, const std::string& propertyName, const std::shared_ptr<Image>& value);

	void SetBool(const std::string& propertyName, bool value) { SetBool(std::string(), propertyName, value); }
	void SetInt(const std::string& propertyName, int value) { SetInt(std::string(), propertyName, value); }
	void SetDouble(const std::string& propertyName, double value) { SetDouble(std::string(), propertyName, value); }
	void SetString(const std::string& propertyName, const std::string& value) { SetString(std::string(), propertyName, value); }
	void SetColor(const std::string& propertyName, const Colorf& value) { SetColor(std::string(), propertyName, value); }
	void SetImage(const std::string& propertyName, const std::shared_ptr<Image>& value) { SetImage(std::string(), propertyName, value); }

private:
	// Note: do not call these directly. Use widget->GetStyleXX instead since a widget may explicitly override a class style
	bool GetBool(const std::string& state, const std::string& propertyName) const;
	int GetInt(const std::string& state, const std::string& propertyName) const;
	double GetDouble(const std::string& state, const std::string& propertyName) const;
	std::string GetString(const std::string& state, const std::string& propertyName) const;
	Colorf GetColor(const std::string& state, const std::string& propertyName) const;
	std::shared_ptr<Image> GetImage(const std::string& state, const std::string& propertyName) const;

	WidgetStyle* ParentStyle = nullptr;
	typedef std::variant<bool, int, double, std::string, Colorf, std::shared_ptr<Font>, std::shared_ptr<Image>> PropertyVariant;
	std::unordered_map<std::string, std::unordered_map<std::string, PropertyVariant>> StyleProperties;
	std::unordered_map<std::string, std::shared_ptr<Font>> Fonts;

	const PropertyVariant* FindProperty(const std::string& state, const std::string& propertyName) const;

	friend class Widget;
};

class BasicWidgetStyle : public WidgetStyle
{
public:
	using WidgetStyle::WidgetStyle;
	void Paint(Widget* widget, Canvas* canvas, Size size) override;
};

class WidgetTheme
{
public:
	virtual ~WidgetTheme() = default;
	
	WidgetStyle* RegisterStyle(std::unique_ptr<WidgetStyle> widgetStyle, const std::string& widgetClass);
	WidgetStyle* GetStyle(const std::string& widgetClass);

	static void SetTheme(std::unique_ptr<WidgetTheme> theme);
	static WidgetTheme* GetTheme();

private:
	std::unordered_map<std::string, std::unique_ptr<WidgetStyle>> Styles;
};

class SimpleTheme : public WidgetTheme
{
public:
	struct ThemeColors
	{
		const Colorf bgMain;   // background
		const Colorf fgMain;   //
		const Colorf bgLight;  // headers / inputs
		const Colorf fgLight;  //
		const Colorf bgAction; // interactive elements
		const Colorf fgAction; //
		const Colorf bgHover;  // hover / highlight
		const Colorf fgHover;  //
		const Colorf bgActive; // click
		const Colorf fgActive; //
		const Colorf border;   // around elements
		const Colorf divider;  // between elements
	};

	SimpleTheme(const ThemeColors& colors);
};

class DarkWidgetTheme : public SimpleTheme
{
public:
	DarkWidgetTheme();
};

class LightWidgetTheme : public SimpleTheme
{
public:
	LightWidgetTheme();
};
