#include <zwidget/widgets/dialog/textinputdialog.h>

#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/lineedit/lineedit.h>
#include <zwidget/widgets/textlabel/textlabel.h>

#include <zwidget/widgets/layout/vboxlayout.h>
#include <zwidget/widgets/layout/hboxlayout.h>

TextInputDialog::TextInputDialog(Widget* owner, const std::string& message, const std::string& windowTitle)
    : Dialog(owner), m_TextLabel(new TextLabel(this)), m_LineEdit(new LineEdit(this)),
      m_OkButton(new PushButton(this)), m_CancelButton(new PushButton(this))
{
    SetWindowTitle(windowTitle);

    m_TextLabel->SetText(message);
    m_OkButton->SetText("Ok");
    m_CancelButton->SetText("Cancel");

    m_OkButton->OnClick = [this]
    {
        Accept();
    };

    m_CancelButton->OnClick = [this]
    {
        Reject();
    };

    auto mainLayout = new VBoxLayout();
    auto buttonsLayout = new HBoxLayout();

    buttonsLayout->AddStretch();
    buttonsLayout->AddWidget(m_OkButton);
    buttonsLayout->AddWidget(m_CancelButton);
    buttonsLayout->AddStretch();

    mainLayout->AddWidget(m_TextLabel);
    mainLayout->AddWidget(m_LineEdit);
    mainLayout->AddStretch();
    mainLayout->AddLayout(buttonsLayout);

    SetLayout(mainLayout);
    SetNoncontentSizes(15, 10, 15, 10);
}

std::string TextInputDialog::GetText() const
{
    return m_LineEdit->GetText();
}

std::string TextInputDialog::TextInput(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto dialog = TextInputDialog(owner, message, windowTitle);

    if (dialog.Exec())
        return dialog.GetText();

    return "";
}
