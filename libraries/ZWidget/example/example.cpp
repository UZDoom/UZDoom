#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <zwidget/core/widget.h>
#include <zwidget/core/resourcedata.h>
#include <zwidget/core/image.h>
#include <zwidget/core/theme.h>
#include <zwidget/window/window.h>
#include <zwidget/widgets/dropdown/dropdown.h>
#include <zwidget/widgets/textedit/textedit.h>
#include <zwidget/widgets/mainwindow/mainwindow.h>
#include <zwidget/widgets/dialog/messagebox.h>
#include <zwidget/widgets/layout/vboxlayout.h>
#include <zwidget/widgets/layout/hboxlayout.h>
#include <zwidget/widgets/listview/listview.h>
#include <zwidget/widgets/imagebox/imagebox.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/lineedit/lineedit.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>
#include <zwidget/widgets/dialog/textinputdialog.h>
#include "stylesheet.h"

// ************************************************************
// Prototypes
// ************************************************************

class LauncherWindowTab1 : public Widget
{
public:
	LauncherWindowTab1(Widget parent);
private:
	TextEdit* Text = nullptr;
};

class LauncherWindowTab2 : public Widget
{
public:
	LauncherWindowTab2(Widget parent);
private:
	TextLabel* WelcomeLabel = nullptr;
	TextLabel* VersionLabel = nullptr;
	TextLabel* SelectLabel = nullptr;
	TextLabel* GeneralLabel = nullptr;
	TextLabel* ExtrasLabel = nullptr;
	CheckboxLabel* FullscreenCheckbox = nullptr;
	CheckboxLabel* DisableAutoloadCheckbox = nullptr;
	CheckboxLabel* DontAskAgainCheckbox = nullptr;
	CheckboxLabel* LightsCheckbox = nullptr;
	CheckboxLabel* BrightmapsCheckbox = nullptr;
	CheckboxLabel* WidescreenCheckbox = nullptr;
	ListView* GamesList = nullptr;
};

class LauncherWindowTab3 : public Widget
{
public:
	LauncherWindowTab3(Widget parent);
private:
	TextLabel* Label = nullptr;
	Dropdown* Choices = nullptr;
	PushButton* Popup = nullptr;
	PushButton* QuestionPopup = nullptr;
	PushButton* TextInputPopup = nullptr;
};

class LauncherWindow : public Widget
{
public:
	LauncherWindow();
private:
	void OnClose() override;

	ImageBox* Logo = nullptr;
	TabWidget* Pages = nullptr;
	PushButton* ExitButton = nullptr;

	LauncherWindowTab1* Tab1 = nullptr;
	LauncherWindowTab2* Tab2 = nullptr;
	LauncherWindowTab3* Tab3 = nullptr;

	std::shared_ptr<CustomCursor> Cursor;
};

// ************************************************************
// UI implementation
// ************************************************************

LauncherWindow::LauncherWindow(): Widget(nullptr, WidgetType::Window)
{
	SetWindowTitle("ZWidget Demo");

	try
	{
		SetWindowIcon({
			Image::LoadResource("surreal-engine-icon-16.png"),
			Image::LoadResource("surreal-engine-icon-24.png"),
			Image::LoadResource("surreal-engine-icon-32.png"),
			Image::LoadResource("surreal-engine-icon-48.png"),
			Image::LoadResource("surreal-engine-icon-64.png"),
			Image::LoadResource("surreal-engine-icon-128.png"),
			Image::LoadResource("surreal-engine-icon-256.png")
			});

		Cursor = CustomCursor::Create({
			CustomCursorFrame(Image::LoadResource("Pentagram01.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram02.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram03.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram04.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram05.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram06.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram07.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram08.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram09.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram10.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram11.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram12.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram13.png"), 0.2),
			CustomCursorFrame(Image::LoadResource("Pentagram14.png"), 0.2)
			}, Point(16, 16));
	}
	catch (...)
	{
	}

	Logo = new ImageBox(this);
	ExitButton = new PushButton(this);
	Pages = new TabWidget(this);

	Logo->SetCursor(Cursor);

	Tab1 = new LauncherWindowTab1(this);
	Tab2 = new LauncherWindowTab2(this);
	Tab3 = new LauncherWindowTab3(this);

	Pages->AddTab(Tab1, "Welcome");
	Pages->AddTab(Tab2, "VKDoom");
	Pages->AddTab(Tab3, "ZWidgets");

	ExitButton->SetText("Exit");

	ExitButton->OnClick = []{
		DisplayWindow::ExitLoop();
	};

	auto mainLayout = new VBoxLayout();

	auto buttonBar = new Widget(this);
	auto buttonAreaLayout = new HBoxLayout();
	buttonAreaLayout->AddStretch();
	buttonAreaLayout->AddWidget(ExitButton);
	buttonBar->SetLayout(buttonAreaLayout);
	buttonBar->SetNoncontentSizes(20, 0, 20, 10);

	mainLayout->AddWidget(Logo);
	mainLayout->AddWidget(Pages);
	mainLayout->AddWidget(buttonBar);

	SetLayout(mainLayout);

	try
	{
		Logo->SetImage(Image::LoadResource("banner.png"));
	}
	catch (...)
	{
	}
}

void LauncherWindow::OnClose()
{
	DisplayWindow::ExitLoop();
}

LauncherWindowTab1::LauncherWindowTab1(Widget parent): Widget(nullptr)
{
	Text = new TextEdit(this);

	Text->SetText(
		"Welcome to VKDoom\n\n"
		"Click the tabs to look at other widgets\n\n"
		"Also, this text is editable\n"
	);

	Text->SetStretching(true);

	auto layout = new VBoxLayout();

	layout->AddWidget(Text);

	SetLayout(layout);
}

LauncherWindowTab2::LauncherWindowTab2(Widget parent): Widget(nullptr)
{
	WelcomeLabel = new TextLabel(this);
	VersionLabel = new TextLabel(this);
	SelectLabel = new TextLabel(this);
	GeneralLabel = new TextLabel(this);
	ExtrasLabel = new TextLabel(this);
	FullscreenCheckbox = new CheckboxLabel(this);
	DisableAutoloadCheckbox = new CheckboxLabel(this);
	DontAskAgainCheckbox = new CheckboxLabel(this);
	LightsCheckbox = new CheckboxLabel(this);
	BrightmapsCheckbox = new CheckboxLabel(this);
	WidescreenCheckbox = new CheckboxLabel(this);
	GamesList = new ListView(this);

	auto label = new TextLabel();
	label->SetText("Label:");
	label->SetFixedWidth(100.0);
	auto lineedit = new LineEdit(nullptr);

	WelcomeLabel->SetText("Welcome to VKDoom");
	VersionLabel->SetText("Version 0xdeadbabe.");
	SelectLabel->SetText("Select which game file (IWAD) to run.");

	GamesList->AddItem({ "Doom", "The OG game" });
	GamesList->AddItem({ "Doom 2: Electric Boogaloo", "For musicians" });
	GamesList->AddItem({ "Doom 3D", "Not just 2.5D anymore!" });
	GamesList->AddItem({ "Doom 4: The Quest for Peace", "Did they find it?"});
	GamesList->AddItem({ "Doom on Ice", "Ice ice baby!" });
	GamesList->AddItem({ "The Doom", "The one and only" });
	GamesList->AddItem({ "Doom 2", "Moar monsters" });

	GamesList->ShowHeader(true);
	GamesList->SetColumn(0, "Game", 250.0);
	GamesList->SetColumn(1, "Description", 500.0);

	GeneralLabel->SetText("General");
	ExtrasLabel->SetText("Extra Graphics");
	FullscreenCheckbox->SetText("Fullscreen");
	DisableAutoloadCheckbox->SetText("Disable autoload");
	DontAskAgainCheckbox->SetText("Don't ask me again");
	LightsCheckbox->SetText("Lights");
	BrightmapsCheckbox->SetText("Brightmaps");
	WidescreenCheckbox->SetText("Widescreen");

	auto layout = new VBoxLayout();
	layout->AddWidget(WelcomeLabel);
	layout->AddWidget(VersionLabel);
	layout->AddWidget(SelectLabel);
	layout->AddWidget(GamesList);

	auto line = new HBoxLayout();
	line->AddWidget(label);
	line->AddWidget(lineedit);
	layout->AddLayout(line);

	auto leftPanel = new VBoxLayout();
	leftPanel->AddWidget(FullscreenCheckbox);
	leftPanel->AddWidget(DisableAutoloadCheckbox);
	leftPanel->AddWidget(DontAskAgainCheckbox);

	auto rightPanel = new VBoxLayout();
	rightPanel->AddWidget(LightsCheckbox);
	rightPanel->AddWidget(BrightmapsCheckbox);
	rightPanel->AddWidget(WidescreenCheckbox);

	auto panelLine = new HBoxLayout();
	panelLine->AddLayout(leftPanel);
	panelLine->AddLayout(rightPanel);

	layout->AddWidget(ExtrasLabel);
	layout->AddLayout(panelLine);

	//layout->AddStretch();

	SetLayout(layout);
}

LauncherWindowTab3::LauncherWindowTab3(Widget parent): Widget(nullptr)
{
	Label = new TextLabel(this);
	Choices = new Dropdown(this);
	Popup = new PushButton(this);
	QuestionPopup = new PushButton(this);
	TextInputPopup = new PushButton(this);

	Label->SetText("Oh my, even more widgets");
	Popup->SetText("Click me.");
	QuestionPopup->SetText("Pop up a question...");
	TextInputPopup->SetText("Pop up a Text Input Dialog");

	Choices->SetMaxDisplayItems(3);
	Choices->AddItem("First");
	Choices->AddItem("Second");
	Choices->AddItem("Third");
	Choices->AddItem("Fourth");
	Choices->AddItem("Fifth");
	Choices->AddItem("Sixth");

	Choices->OnChanged = [this](int index) {
		std::cout << "Selected " << index << ":" << Choices->GetItem(index) << std::endl;
	};

	Popup->OnClick = [this]{
		auto result = MessageBox::Information(this, "This is an Information MessageBox. You selected " + Choices->GetItem(Choices->GetSelectedItem()) + ".");
		std::cout << "Dialog returned: " << static_cast<uint32_t>(result) << std::endl;
	};

	QuestionPopup->OnClick = [this]{
		auto result = MessageBox::Question(this, "Choose one and choose wisely.");
		if (result == DialogButton::Yes)
		{
			std::cout << "The user choosed wisely." << std::endl;
		}
		else
		{
			std::cout << "No regrets..." << std::endl;
		}
	};

	TextInputPopup->OnClick = [this]
	{
		const auto text = TextInputDialog::TextInput(this);

		std::cout << "Text Input Result = " + text << std::endl;
	};

	auto layout = new VBoxLayout();
	layout->AddWidget(Label);
	layout->AddWidget(Choices);
	layout->AddWidget(Popup);
	layout->AddWidget(QuestionPopup);
	layout->AddWidget(TextInputPopup);

	SetLayout(layout);
}

// ************************************************************
// Shared code
// ************************************************************

enum class Backend
{
	Default, Win32, SDL2, X11, Wayland
};

enum class Theme
{
	Default, Light, Dark
};

class ExampleResourceLoader : public ResourceLoader
{
public:
	std::vector<SingleFontData> LoadFont(const std::string& name) override
	{
		if (name == "system" || name == "monospace")
		{
			SingleFontData fontdata;
			fontdata.fontdata = ReadAllBytes("OpenSans.ttf");
			return { std::move(fontdata) };
		}
		else
		{
			SingleFontData fontdata;
			fontdata.fontdata = ReadAllBytes(name + ".ttf");
			return { std::move(fontdata) };
		}
	}
	
	std::vector<uint8_t> ReadAllBytes(const std::string& filename) override
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

int example(Backend backend = Backend::Default, Theme theme = Theme::Default)
{
	ResourceLoader::Set(std::make_unique<ExampleResourceLoader>());

#if 1
	switch (theme)
	{
	case Theme::Default: WidgetTheme::SetTheme(std::make_unique<StylesheetTheme>(stylesheet, "dark")); break;
	case Theme::Dark:    WidgetTheme::SetTheme(std::make_unique<StylesheetTheme>(stylesheet, "dark")); break;
	case Theme::Light:   WidgetTheme::SetTheme(std::make_unique<StylesheetTheme>(stylesheet, "light")); break;
	}
#else
	// just for testing themes
	switch (theme)
	{
		case Theme::Default: WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>()); break;
		case Theme::Dark:    WidgetTheme::SetTheme(std::make_unique<DarkWidgetTheme>()); break;
		case Theme::Light:   WidgetTheme::SetTheme(std::make_unique<LightWidgetTheme>()); break;
	}
#endif

	// just for testing backends
	switch (backend)
	{
		case Backend::Default: DisplayBackend::Set(DisplayBackend::TryCreateBackend()); break;
		case Backend::Win32:   DisplayBackend::Set(DisplayBackend::TryCreateWin32());   break;
		case Backend::SDL2:    DisplayBackend::Set(DisplayBackend::TryCreateSDL2());    break;
		case Backend::X11:     DisplayBackend::Set(DisplayBackend::TryCreateX11());     break;
		case Backend::Wayland: DisplayBackend::Set(DisplayBackend::TryCreateWayland()); break;
	}

	auto launcher = new LauncherWindow();
	launcher->SetFrameGeometry((Widget::GetScreenSize().width - 615.0) * 0.5, (Widget::GetScreenSize().height - 668.0) * 0.5, 615.0, 668.0);
	launcher->Show();

	DisplayWindow::RunLoop();

	return 0;
}

// ************************************************************
// Platform-specific code
// ************************************************************

#ifdef WIN32

#include <Windows.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	SetProcessDPIAware();
	example();
}

#else

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

int main(int argc, const char** argv)
{
	Backend backend = Backend::Default;
	Theme theme = Theme::Default;

	for (auto i = 1; i < argc; i++)
	{
		std::string s = argv[i];

		if (s == "light") { theme = Theme::Light; continue; }
		if (s == "dark")  { theme = Theme::Dark;  continue; }

		if (s == "sdl2")    { backend = Backend::SDL2;    continue; }
		if (s == "x11")     { backend = Backend::X11;     continue; }
		if (s == "wayland") { backend = Backend::Wayland; continue; }
		if (s == "win32")   { backend = Backend::Win32;   continue; } // lol
	}

	example(backend, theme);
}

#endif
