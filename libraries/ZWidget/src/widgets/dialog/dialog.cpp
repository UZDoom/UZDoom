#include <zwidget/widgets/dialog/dialog.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>

Dialog::Dialog(Widget* owner) : Widget(owner, WidgetType::Dialog)
{
}

int Dialog::Exec()
{
    Show();
    DisplayWindow::RunLoop();
    Hide();
    return m_ExitCode;
}

void Dialog::ExitLoop(const int code)
{
    m_ExitCode = code;
    DisplayWindow::ExitLoop();
}

void Dialog::Accept()
{
    ExitLoop(1);
}

void Dialog::Reject()
{
    ExitLoop(0);
}

void Dialog::OnClose()
{
    Reject();
}

PushButton* Dialog::AddDialogButton(DialogButton button)
{
    auto btn = new PushButton(this);

    switch (button)
    {
    case DialogButton::Ok:
        btn->SetText("Ok");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Ok) ); };
        break;
    case DialogButton::Cancel:
        btn->SetText("Cancel");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Cancel) ); };
        break;
    case DialogButton::Apply:
        btn->SetText("Apply");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Apply) ); };
        break;
    case DialogButton::Yes:
        btn->SetText("Yes");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Yes) ); };
        break;
    case DialogButton::YesToAll:
        btn->SetText("Yes To All");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::YesToAll) ); };
        break;
    case DialogButton::No:
        btn->SetText("No");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::No) ); };
        break;
    case DialogButton::NoToAll:
        btn->SetText("NoToAll");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::NoToAll) ); };
        break;
    case DialogButton::Abort:
        btn->SetText("Abort");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Abort) ); };
        break;
    case DialogButton::Ignore:
        btn->SetText("Ignore");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Ignore) ); };
        break;
    case DialogButton::Retry:
        btn->SetText("Retry");
        btn->OnClick = [this]() { this->ExitLoop( static_cast<int>(DialogButton::Retry) ); };
        break;
    }

    return btn;
}