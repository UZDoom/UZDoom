
#include "cocoa_open_folder_dialog.h"

#if __APPLE__
#import "AppKitWrapper.h"
#endif // __APPLE__



CocoaOpenFolderDialog::CocoaOpenFolderDialog(DisplayWindow* owner)
{
    panel = (__bridge void*)[NSOpenPanel openPanel];
    [((__bridge NSOpenPanel*)panel) setCanChooseFiles:NO];
    [((__bridge NSOpenPanel*)panel) setCanChooseDirectories:YES];
    [((__bridge NSOpenPanel*)panel) setAllowsMultipleSelection:NO];
}

void CocoaOpenFolderDialog::SetInitialDirectory(const std::string& path)
{
    [((__bridge NSOpenPanel*)panel) setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]]];
}

bool CocoaOpenFolderDialog::Show()
{
    if ([((__bridge NSOpenPanel*)panel) runModal] == NSModalResponseOK)
    {
        _selectedPath = [[[(__bridge NSOpenPanel*)panel URL] path] UTF8String];
        return true;
    }
    return false;
}

void CocoaOpenFolderDialog::SetTitle(const std::string& title)
{
    [((__bridge NSOpenPanel*)panel) setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

std::string CocoaOpenFolderDialog::SelectedPath() const
{
    return _selectedPath;
}




