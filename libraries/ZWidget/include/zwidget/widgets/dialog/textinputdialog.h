#pragma once

#include "dialog.h"

class LineEdit;
class TextLabel;
class PushButton;

// A Dialog with a single LineEdit widget and Ok/Cancel buttons
class TextInputDialog : public Dialog
{
public:
    TextInputDialog(Widget* owner, const std::string& message = "Enter text down below:", const std::string& windowTitle = "Input Text");
    // Prompts a simple TextInputDialog, and returns what's written inside it, or an empty string if the dialog is rejected.
    static std::string TextInput(Widget* owner, const std::string& message = "Enter text down below:", const std::string& windowTitle = "Input Text");

    // Returns the text inputted in LineEdit
    std::string GetText() const;
private:
    TextLabel* m_TextLabel;
    LineEdit* m_LineEdit;

    PushButton* m_OkButton;
    PushButton* m_CancelButton;
};