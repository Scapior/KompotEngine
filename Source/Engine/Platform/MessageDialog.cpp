/*
 *  MessageDialog.cpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "MessageDialog.hpp"

#ifdef ENGINE_OS_WINDOWS

    #include <Windows.h>
using namespace Kompot::Platform::MessageDialog;

Result Kompot::Platform::MessageDialog::Show(
    const std::wstring_view& title,
    const std::wstring_view& text,
    ButtonOptions buttonOption,
    IconOptions iconOptions,
    DefaultButtonOptions defaultButtonOptions)
{
    const UINT optionsMask =
        0 | (iconOptions == IconOptions::Error ? MB_ICONERROR : 0) | (iconOptions == IconOptions::Information ? MB_ICONINFORMATION : 0) |
        (iconOptions == IconOptions::Question ? MB_ICONQUESTION : 0) | (iconOptions == IconOptions::Warning ? MB_ICONWARNING : 0) |

        (buttonOption == ButtonOptions::AbortRetryIgnore ? MB_ABORTRETRYIGNORE : 0) |
        (buttonOption == ButtonOptions::CancelTryAgainContinue ? MB_CANCELTRYCONTINUE : 0) | (buttonOption == ButtonOptions::Help ? MB_HELP : 0) |
        (buttonOption == ButtonOptions::Ok ? MB_OK : 0) | (buttonOption == ButtonOptions::OkCancel ? MB_OKCANCEL : 0) |
        (buttonOption == ButtonOptions::RetryCancel ? MB_RETRYCANCEL : 0) | (buttonOption == ButtonOptions::YesNo ? MB_YESNO : 0) |
        (buttonOption == ButtonOptions::YesNoCancel ? MB_YESNOCANCEL : 0) |

        (defaultButtonOptions == DefaultButtonOptions::First ? MB_DEFBUTTON1 : 0) |
        (defaultButtonOptions == DefaultButtonOptions::Second ? MB_DEFBUTTON2 : 0) |
        (defaultButtonOptions == DefaultButtonOptions::Third ? MB_DEFBUTTON3 : 0) |
        (defaultButtonOptions == DefaultButtonOptions::Fourth ? MB_DEFBUTTON4 : 0);
    const auto messageDialogResult = ::MessageBoxW(
        NULL,
        text.data(),
        title.data(),
        optionsMask // MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
    );

    return static_cast<Result>(messageDialogResult);
}
#endif
