/*
 *  MessageDialog.hpp
 *  Copyright (C) 2021 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include <string>


namespace Kompot::Platform::MessageDialog
{
enum class Result : uint8_t
{
    None,
    Abort,
    Cancel,
    Continue,
    Ignore,
    No,
    Ok,
    Retry,
    TryAgain,
    Yes
};

enum class ButtonOptions : uint8_t
{
    Ok,
    AbortRetryIgnore,
    CancelTryAgainContinue,
    Help,
    OkCancel,
    RetryCancel,
    YesNo,
    YesNoCancel
};

enum class DefaultButtonOptions : uint8_t
{
    First,
    Second,
    Third,
    Fourth,
};

enum class IconOptions : uint8_t
{
    None,
    Question,
    Information,
    Warning,
    Error
};

Result Show(
    const std::wstring_view& title,
    const std::wstring_view& text,
    ButtonOptions buttonOption,
    IconOptions iconOptions                   = IconOptions::None,
    DefaultButtonOptions defaultButtonOptions = DefaultButtonOptions::First);

} // namespace Kompot::Platform::MessageDialog
