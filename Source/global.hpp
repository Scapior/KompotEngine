#pragma once

#if defined(_DEBUG) || !defined(NDEBUF)
#define ENGINE_DEBUG
#endif

#include "Misc/Log.hpp"
#include <cstdint> // for uint64_t, etc
#include <thread>
#include <chrono>

using namespace std::string_literals; // "foo"s
using namespace std::chrono_literals; // 2s, 5 ms

static const int8_t ENGINE_VESRION_MAJOR = 0;
static const int8_t ENGINE_VESRION_MINOR = 0;
static const int8_t ENGINE_VESRION_PATCH = 0;
static const std::string ENGINE_NAME = "KompotEngine"s;


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
