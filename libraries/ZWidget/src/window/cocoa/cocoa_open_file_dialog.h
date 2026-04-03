#pragma once

#include "zwidget/window/window.h"
#include "zwidget/systemdialogs/open_file_dialog.h"

#include <string>
#include <vector>

class CocoaOpenFileDialog : public OpenFileDialog
{
public:
    CocoaOpenFileDialog(DisplayWindow* owner);

    void SetMultiSelect(bool multiselect) override;
    void AddFilter(const std::string &filter_description, const std::string &filter_extension) override;
    void SetInitialDirectory(const std::string& path) override;
    void SetFilename(const std::string &filename) override;
    void SetDefaultExtension(const std::string& extension) override;
    void ClearFilters() override;
    void SetFilterIndex(int filter_index) override;
    void SetTitle(const std::string &title) override;

    bool Show() override;
    std::string Filename() const override;
    std::vector<std::string> Filenames() const override;

private:
    // DisplayWindow* _owner = nullptr;
    // bool _multiselect = false;
    std::vector<std::pair<std::string, std::string>> _filters;
    std::string _initialDirectory;
    std::string _filename;
    std::string _defaultExtension;
    int _filterIndex = 0;
    std::string _title;
    std::vector<std::string> _filenames;
    void* panel = nullptr;

    // Helper method to run modal and collect results
    bool runModalAndGetResults();
};
