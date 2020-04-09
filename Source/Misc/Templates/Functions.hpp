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

//template <typename... T>
//constexpr auto makeArray(T&&... values) ->
//        std::array<typename std::decay<typename std::common_type<T...>::type>::type, sizeof...(T)>
//{
//    return { std::forward<T>(values)... };
//}


//template <std::size_t N>
//constexpr std::array<char, N> makeArray(const char* literal[N])
//{
//    return ;
//}

namespace makeArrayDetails {
template <class T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N>
    makeArrayDetailsImplementation(T (&a)[N], std::index_sequence<I...>)
{
    return { {a[I]...} };
}
}

template <class T, std::size_t N>
constexpr std::array<std::remove_cv_t<T>, N> makeArray(T (&a)[N])
{
    return makeArrayDetails::makeArrayDetailsImplementation(a, std::make_index_sequence<N>{});
}



} //namespace KompotEngine
