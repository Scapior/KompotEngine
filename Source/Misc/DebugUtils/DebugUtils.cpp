/*
*  DebugUtils.cpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#include "DebugUtils.hpp"
#include <Misc/Log.hpp>
#include <limits>

#if defined(ENGINE_OS_WINDOWS)
    #define NOMINMAX
    #include <Windows.h>
    #include <DbgHelp.h>
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
    std::size_t size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL
    );

    const std::string result(messageBuffer, size);
    LocalFree(messageBuffer);

    return result;
#elif defined (ENGINE_OS_LINUX)
    return {};
#else
    return {};
#endif
}

std::string DebugUtils::getCallstack()
{
#if defined(ENGINE_OS_WINDOWS)
    static const auto TRACE_MAX_FUNCTION_NAME_LENGTH = 1024;
    constexpr auto unsignedShortMax = std::numeric_limits<unsigned short>::max();

    void* stack[unsignedShortMax];
    const auto capturedCallstackFramesCount = ::CaptureStackBackTrace(0, unsignedShortMax, stack, nullptr);

    const HANDLE process = GetCurrentProcess();

    SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO)+(TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
    symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD displacement;
    IMAGEHLP_LINE64 *line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    for (int i = 0; i < capturedCallstackFramesCount; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(process, address, NULL, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, line))
        {
            char buff[1024];
            sprintf(buff, "\tat %s in %s: line: %lu: address: 0x%0X\n", symbol->Name, line->FileName, line->LineNumber, symbol->Address);
            std::string buffstr(buff);
            Log::getInstance() << buffstr << std::endl;
        }
        else
        {
            char buff[1024];
            sprintf(buff, "\tSymGetLineFromAddr64 returned error code %lu.\n", GetLastError());
            std::string buffstr(buff);
            Log::getInstance() << buffstr << std::endl;
            sprintf(buff, "\tat %s, address 0x%0X.\n", symbol->Name, symbol->Address);
            buffstr = std::string(buff);
            Log::getInstance() << buffstr << std::endl;
        }
    }

    if (capturedCallstackFramesCount)
    {

    }
    else
    {
        Log::getInstance() << "The callstack getting error: " << getLastPlatformError() << std::endl;
        return std::string();
    }

    return std::string();
#elif defined(ENGINE_OS_LINUX)
    // ToDo
    return {};
#endif
}


