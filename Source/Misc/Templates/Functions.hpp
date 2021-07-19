/*
 *  Functions.hpp
 *  Copyright (C) 2020 by Maxim Stoyanov
 *  scapior.github.io
 */

#pragma once
#include <array>
#include <type_traits>
#include <utility>

namespace Kompot::TemplateUtils
{
template<typename T, std::size_t N, std::size_t M>
constexpr const auto concatArrays(const std::array<T, N>&& ar1, const std::array<T, M>&& ar2)
{
    std::array<T, N + M - 1> result;
    std::copy(ar1.cbegin(), ar1.cend(), result.begin());
    std::copy(ar2.cbegin(), ar2.cend(), result.begin() + N - 1);
    return result;
}

} // namespace Kompot::TemplateUtils
