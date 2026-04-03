#pragma once

#include "zwidget/window/window.h"
#include "zwidget/systemdialogs/open_folder_dialog.h"

#include <string>
#include <vector>

class CocoaOpenFolderDialog : public OpenFolderDialog
{
public:
    CocoaOpenFolderDialog(DisplayWindow* owner);

    void SetInitialDirectory(const std::string& path) override;
    void SetTitle(const std::string &title) override;

    bool Show() override;
    std::string SelectedPath() const override;

private:
    // DisplayWindow* _owner = nullptr;
    std::string _initialDirectory;
    std::string _title;
    std::string _selectedPath;
    void* panel = nullptr;
};
