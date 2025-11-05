#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class CheckboxLabel;
class TextEdit;
struct FStartupSelectionInfo;

class ReleasePage : public Widget
{
public:
	ReleasePage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnLanguageChanged(int i);
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;

	TextEdit* Notes = nullptr;
	CheckboxLabel* ShowThis = nullptr;
};
