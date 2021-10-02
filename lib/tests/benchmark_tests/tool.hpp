#pragma once

#include <type_traits>
#include <iterator>
#include <concepts>
#include <ranges>
#include <random>

template<std::forward_iterator O, std::random_access_iterator I> inline auto
fill_random(O first, O last, I input_first, I input_last) -> void
requires requires (O o, I i) { *o = *i; }
{
    if (first != last && input_first != input_last)
    {
        using difference_type = typename std::iterator_traits<I>::difference_type;
        auto thread_local static engine = std::mt19937{std::random_device{}()};
        auto n = std::max(std::distance(input_first, input_last), static_cast<difference_type>(1));
        auto d = std::uniform_int_distribution<difference_type>(0, n - 1);

        for (; first != last; ++first)
            *first = *(input_first + d(engine));
    }
}

template<std::ranges::forward_range O, std::ranges::random_access_range I> inline auto
fill_random(O & output, I const & input) -> void
requires std::same_as<std::ranges::range_value_t<I>, std::ranges::range_value_t<O>>
{
    auto oh = std::ranges::begin(output);
    auto ot = std::ranges::  end(output);
    auto ih = std::ranges::begin( input);
    auto it = std::ranges::  end( input);

    using value_type = std::ranges::range_value_t<I>;
    if constexpr (
        std::ranges::contiguous_range<I> &&
        std::is_array_v<I> &&
        std::same_as<value_type, typename std::char_traits<value_type>::char_type> )
        -- it;

    fill_random(oh, ot, ih, it);
}
