/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once

/** \file global.hpp
\brief The file with often user functions or global macros.
The file contains
*/

/*! \def ENGINE_BUILD_SERVER
\brief A macro that be defined when target build is a server.
If you need to build a server define this macro.
If this macro not defined, will be defined ENGINE_BUILD_CLIENT.
*/

/*! \def ENGINE_BUILD_CLIENT
\brief A macro that be defined when target build is a client.
Details.
*/

/*! \fn int open(const char *pathname,int flags)
\brief Opens a file descriptor.
\param pathname The name of the descriptor.
\param flags Opening flags.
*/

/* Build mode */
#ifndef ENGINE_BUILD_SERVER
#define ENGINE_BUILD_CLIENT
#endif

#if defined(_DEBUG) || !defined(NDEBUF)
#define ENGINE_DEBUG
#endif

/* Platform macros */
#if defined(__linux) ||  defined (__linux__)
#define ENGINE_PLATFORM_LINUX
#endif

#ifdef __win32
#define ENGINE_PLATFORM_WIN32
#endif

/* Default includes and defines */

#include "Misc/Log.hpp"
#include <cstdint> // for uint64_t, etc
#include <thread>
#include <chrono>
#include <string>
#include <assert.h>

using namespace std::string_literals; // "foo"s
using namespace std::chrono_literals; // 2s, 5 ms

static const int8_t ENGINE_VESRION_MAJOR = 0;
static const int8_t ENGINE_VESRION_MINOR = 0;
static const int8_t ENGINE_VESRION_PATCH = 1;
static const char * ENGINE_NAME = "KompotEngine";

constexpr int8_t operator "" _8t(unsigned long long value)
{
      return static_cast<int8_t>(value);
}

constexpr int16_t operator "" _16t(unsigned long long value)
{
      return static_cast<int16_t>(value);
}

constexpr int32_t operator "" _32t(unsigned long long value)
{
      return static_cast<int32_t>(value);
}

constexpr int64_t operator "" _64t(unsigned long long value)
{
      return static_cast<int64_t>(value);
}

constexpr uint8_t operator "" _u8t(unsigned long long value)
{
      return static_cast<uint8_t>(value);
}

constexpr uint16_t operator "" _u16t(unsigned long long value)
{
      return static_cast<uint16_t>(value);
}

constexpr uint32_t operator "" _u32t(unsigned long long value)
{
      return static_cast<uint32_t>(value);
}

constexpr uint64_t operator "" _u64t(unsigned long long value)
{
      return static_cast<uint64_t>(value);
}
