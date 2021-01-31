/*
 *  DebugUtils.cpp
 *  Copyright (C) 2020 by Maxim Stoianov
 *  Licensed under the MIT license.
 */

#include "DebugUtils.hpp"
#include <Engine/Log/Log.hpp>
#include <Misc/StringUtils/StringUtils.hpp>
#include <limits>

#if defined(ENGINE_OS_WINDOWS)

    #include <Windows.h>
    #include <DbgHelp.h>
#elif defined(ENGINE_OS_LINUX)
    #include <execinfo.h>
#endif

std::string DebugUtils::getLastPlatformError()
{
#if defined(ENGINE_OS_WINDOWS)
    const auto errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return std::string();
    }

    LPSTR messageBuffer = nullptr;
    std::size_t size    = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorMessageID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL);

    const std::string result(messageBuffer, size);
    ::LocalFree(messageBuffer);

    return result;
#elif defined(ENGINE_OS_LINUX)
    return {};
#else
    return {};
#endif
}

std::string DebugUtils::getCallstack()
{
#if defined(ENGINE_OS_WINDOWS)
    ::SymInitialize(GetCurrentProcess(), nullptr, true);

    static const auto TRACE_MAX_FUNCTION_NAME_LENGTH = 1024;
    constexpr auto unsignedShortMax                  = std::numeric_limits<unsigned short>::max();

    void* stack[unsignedShortMax];
    const auto capturedCallstackFramesCount = ::CaptureStackBackTrace(0, unsignedShortMax, stack, nullptr);

    std::string result;
    result.reserve(64 * 1024 * 1024);

    for (int i = 0; i < capturedCallstackFramesCount; i++)
    {
        DWORD_PTR frame = reinterpret_cast<DWORD_PTR>(stack[i]);

        const auto bufferSize = (sizeof(SYMBOL_INFO) + TRACE_MAX_FUNCTION_NAME_LENGTH * sizeof(wchar_t) + sizeof(ULONG64) - 1) / sizeof(ULONG64);
        ULONG64 buffer[bufferSize];
        memset(buffer, 0, sizeof(buffer));

        DWORD64 displacement = 0;

        PSYMBOL_INFO symbolInfo  = reinterpret_cast<PSYMBOL_INFO>(&buffer[0]);
        symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbolInfo->MaxNameLen   = TRACE_MAX_FUNCTION_NAME_LENGTH - 1;

        const auto hasSymbol = ::SymFromAddr(GetCurrentProcess(), frame, &displacement, symbolInfo);

        // Attempt to retrieve line number information.
        DWORD line_displacement = 0;
        IMAGEHLP_LINE64 line    = {};
        line.SizeOfStruct       = sizeof(IMAGEHLP_LINE64);
        const auto hasLine      = ::SymGetLineFromAddr64(GetCurrentProcess(), frame, &line_displacement, &line);

        if (hasSymbol)
        {
            result.append(symbolInfo->Name)
                .append(" [0x")
                .append(StringUtils::hexFromPointer(stack[i]))
                .append("+")
                .append(StringUtils::fromIntiger(displacement))
                .append("]");
        }
        else
        {
            result.append("(No symbol) [0x").append(StringUtils::hexFromPointer(stack[i])).append("]");
        }
        if (hasLine)
        {
            result.append(" (").append(line.FileName).append(":").append(StringUtils::fromIntiger(line.LineNumber)).append(")");
        }
        result.append("\n");
    }

    return result;
#elif defined(ENGINE_OS_LINUX)

    static constexpr int32_t STACK_MAX_SIZE = 1024;
    void* stack[STACK_MAX_SIZE];

    const int32_t stackEntriesCount = backtrace(stack, STACK_MAX_SIZE);
    if (stackEntriesCount < 1)
    {
        Log::getInstance() << getLastPlatformError() << std::endl;
        return {};
    }

    char** stackRawText = backtrace_symbols(stack, stackEntriesCount);
    if (stackRawText == nullptr)
    {
        Log::getInstance() << getLastPlatformError() << std::endl;
        return {};
    }

    std::size_t resultLenght = static_cast<std::size_t>(stackEntriesCount);
    for (int32_t i = 0; i < stackEntriesCount; ++i)
    {
        resultLenght += strlen(stackRawText[i]);
    }

    std::string result;
    result.reserve(resultLenght);
    for (int32_t i = 0; i < stackEntriesCount; ++i)
    {
        result.append(stackRawText[i]).append("\n");
    }

    return result;
#endif
}
