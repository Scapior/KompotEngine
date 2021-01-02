/*
 *  Functions.hpp
 *  Copyright (C) 2020 by Maxim Stoyanov
 *  scapior.github.io
 */

#pragma once
#include <array>
#include <utility>
#include <type_traits>

template<typename T, std::size_t N>
constexpr auto operator+(const T element, const std::array<T, N> array)
{
    return std::array<T, 1>{element} + array;
}

template<typename T, std::size_t N>
constexpr auto operator+(const std::array<T, N> array, const T element)
{
    return array + std::array<T, 1>{element};
}

template<typename T, std::size_t NA, std::size_t NB>
constexpr auto operator+(const std::array<T, NA> arrA, const std::array<T, NB> arrB)
{
    std::array<T, NA + NB> result{};
    std::size_t i = 0;
    for (; i < arrA.size(); ++i)
    {
        result[i] = arrA[i];
    }
    for (std::size_t j = 0; j < arrB.size(); ++j)
    {
        result[i++] = arrB[j];
    }
    return result;
}

namespace TemplateUtils
{
// ToDo in the future: in C++20 I can swap the cycle to the std::copy
template<typename T, std::size_t N>
constexpr auto makeArray(const T (&array)[N])
{
    std::array<T, N> result{};
    for (std::size_t i = 0; i < N; ++i)
    {
        result[i] = array[i];
    }
    return result;
}

} // namespace TemplateUtils
