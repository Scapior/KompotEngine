/*
*  Functions.hpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#pragma once
#include <array>
#include <utility>
#include <type_traits>

namespace KompotEngine
{

namespace Details
{

template<typename T, std::size_t N, std::size_t... I, typename ReturnType = std::array<std::remove_cv_t<T>, N>>
constexpr ReturnType makeArrayDetails(T (&elements)[N], std::index_sequence<I...>)
{
    return { { elements[I]...} };
}

} // namespace Details

template <typename T, std::size_t N>
constexpr auto makeArray(T (&elementsArray)[N])
{
    return Details::makeArrayDetails(elementsArray, std::make_index_sequence<N>{});
}



} //namespace KompotEngine
