#pragma once

#include "zwidget/window/window.h"
#include "zwidget/systemdialogs/save_file_dialog.h"

#include <string>
#include <vector>

class CocoaSaveFileDialog : public SaveFileDialog
{
public:
    CocoaSaveFileDialog(DisplayWindow* owner);

    void AddFilter(const std::string &filter_description, const std::string &filter_extension) override;
    void SetInitialDirectory(const std::string& path) override;
    void SetFilename(const std::string &filename) override;
    void SetDefaultExtension(const std::string& extension) override;
    void ClearFilters() override;
    void SetFilterIndex(int filter_index) override;
    void SetTitle(const std::string &title) override;

    bool Show() override;
    std::string Filename() const override;

private:
    // DisplayWindow* _owner = nullptr;
    std::vector<std::pair<std::string, std::string>> _filters;
    std::string _initialDirectory;
    std::string _filename;
    std::string _defaultExtension;
    // int _filterIndex = 0;
    std::string _title;
    void* panel = nullptr;
};
