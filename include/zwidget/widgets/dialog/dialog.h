#pragma once

#include <zwidget/core/widget.h>

class PushButton;

// Standard dialog buttons
enum class DialogButton : uint32_t
{
    Ok =       (1 << 0),
    Cancel =   (1 << 1),
    Apply =    (1 << 2),
    Yes =      (1 << 3),
    YesToAll = (1 << 4),
    No =       (1 << 5),
    NoToAll =  (1 << 6),
    Abort =    (1 << 7),
    Ignore =   (1 << 8),
    Retry =    (1 << 9),
};

inline DialogButton operator |(DialogButton lhs, DialogButton rhs)
{
    return static_cast<DialogButton>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline DialogButton operator &(DialogButton lhs, DialogButton rhs)
{
    return static_cast<DialogButton>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline bool AllFlags(DialogButton value, DialogButton flags) { return (value & flags) == flags; }
inline bool AnyFlags(DialogButton value, DialogButton flags) { return (uint32_t)(value | flags) != 0; }

class Dialog : public Widget
{
public:
    explicit Dialog(Widget* owner);

    //  Shows the dialog to the user. Returns the exitcode when the dialog is closed.
    int Exec();

    // Causes the dialog to close with the exitcode code.
    void ExitLoop(int code);

    // Causes the dialog to close with an Accepted exitcode (= 1)
    void Accept();

    // Causes the dialog to close with an Rejected exitcode (= 0)
    void Reject();

    // Handles the case when the dialog is closed with the close button on the window decorations
    // By default, this causes the dialog to be "Rejected"
    void OnClose() override;

protected:
    // Creates a single Standard Dialog Button
    PushButton* AddDialogButton(DialogButton button);

private:
    int m_ExitCode = 0;
};
