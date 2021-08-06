#pragma once

#include <concepts>
#include <random>
#include <iterator>
#include <type_traits>

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
