#pragma once

#include "dialog.h"

class PushButton;
class TextLabel;

enum class MessageBoxIcon
{
    None,
    Information,
    Question,
    Warning,
    Error
};

/* Generic MessageBox class */
class MessageBox : public Dialog
{
public:
    explicit MessageBox(Widget* owner, const std::string& windowTitle, const std::string& message, MessageBoxIcon icon, DialogButton buttons);
    static DialogButton Information(Widget* owner, const std::string& message, const std::string& windowTitle = "Information");
    static DialogButton Question(Widget* owner, const std::string& message, const std::string& windowTitle = "Question");
    static DialogButton Warning(Widget* owner, const std::string& message, const std::string& windowTitle = "Warning");
    static DialogButton Error(Widget* owner, const std::string& message, const std::string& windowTitle = "Error");

private:
    TextLabel* m_MessageLabel;
    std::vector<PushButton*> m_DialogButtons;
};