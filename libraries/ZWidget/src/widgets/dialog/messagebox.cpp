#include <zwidget/widgets/dialog/messagebox.h>

#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/core/layout.h>

#include "zwidget/widgets/layout/hboxlayout.h"
#include "zwidget/widgets/layout/vboxlayout.h"

MessageBox::MessageBox(Widget* owner, const std::string& windowTitle, const std::string& message, MessageBoxIcon icon, DialogButton buttons)
        : Dialog(owner), m_MessageLabel(new TextLabel(this)), m_DialogButtons({})
{
    SetWindowTitle(windowTitle);

    if (owner && !owner->GetWindowIcon().empty())
    {
        /* Inherit the window icon from the owner */
        SetWindowIcon(owner->GetWindowIcon());
    }

    m_MessageLabel->SetText(message);

    if (AllFlags(buttons, DialogButton::Ok))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Ok));
    if (AllFlags(buttons, DialogButton::Cancel))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Cancel));
    if (AllFlags(buttons, DialogButton::Apply))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Apply));
    if (AllFlags(buttons, DialogButton::Yes))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Yes));
    if (AllFlags(buttons, DialogButton::YesToAll))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::YesToAll));
    if (AllFlags(buttons, DialogButton::No))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::No));
    if (AllFlags(buttons, DialogButton::NoToAll))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::NoToAll));
    if (AllFlags(buttons, DialogButton::Abort))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Abort));
    if (AllFlags(buttons, DialogButton::Ignore))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Ignore));
    if (AllFlags(buttons, DialogButton::Retry))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Retry));

    /* Set up layout */

    auto mainLayout = new VBoxLayout();

    auto messageLayout = new HBoxLayout();
    // TODO: Add an icon widget here too
    messageLayout->AddWidget(m_MessageLabel);

    auto buttonsLayout = new HBoxLayout();
    buttonsLayout->AddStretch();
    for (const auto button : m_DialogButtons)
        buttonsLayout->AddWidget(button);
    buttonsLayout->AddStretch();

    mainLayout->AddLayout(messageLayout);
    mainLayout->AddStretch();
    mainLayout->AddLayout(buttonsLayout);

    SetLayout(mainLayout);
    SetNoncontentSizes(15, 10, 15, 10);
}

DialogButton MessageBox::Information(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Information, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Question(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Question, DialogButton::Yes | DialogButton::No);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Warning(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Warning, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Error(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Error, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}