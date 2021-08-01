#pragma once

#include <concepts>
#include <type_traits> // std::true_type
#include <utility>     // std::iter_value_t
#include <iterator>    // std::input_or_output_iterator
#include <ranges>      // std::ranges::end
#include <regex>       // std::regex_traits
#include <string>      // std::char_traits

namespace far { namespace cep
{
    /// common concepts of far

    template<typename C>
    concept char_type
        // note:
        // should I use "std::convertible_to" or just "std::same_as"?
        = std::same_as<typename std::regex_traits<C>::char_type, C>
       && std::same_as<typename std:: char_traits<C>::char_type, C>
        ;

    // iterators for any char sequence
    template<typename I, typename C>
    concept char_iter
        = char_type<C>
       && std::input_or_output_iterator<I>
       && std::same_as<std::iter_value_t<I>, C>
        ;

    // iterators of pattern and template string for matching
    template<typename I, typename C>
    concept matcher_iter
        = char_iter<I, C>
       && std::forward_iterator<I>
        ;

    // iterators of matched source string
    template<typename I, typename C>
    concept matched_iter
        = char_iter<I, C>
       && std::bidirectional_iterator<I>
        ;

    // containers for matching
    template<typename R, typename C>
    concept matcher
        = std::ranges::range<R>
       && matcher_iter<std::ranges::iterator_t<R>, C>
        ;

    // containers of matched
    template<typename R, typename C>
    concept matched
        = std::ranges::range<R>
       && matched_iter<std::ranges::iterator_t<R>, C>
        ;
}}

namespace far { namespace aux
{
    namespace detail
    {
        //// rewite std::ranges::end("string literal") to that - 1
        struct end
        {
        private:
            template<std::ranges::range R>
            struct zero_suffixed : std::false_type {};

            template<typename C, std::size_t N>
            requires (N > 0) && requires (C const (&c)[N]) { {c + N } -> std::contiguous_iterator; }
            struct zero_suffixed<C(&)[N]> : std::true_type
            {
                static constexpr std::ptrdiff_t N = N;
            };

        public:
            template<std::ranges::range R>
            auto constexpr operator()(R && r) const
            noexcept(noexcept(std::ranges::end(std::forward<R>(r))))
            {
                if constexpr (zero_suffixed<R>::value)
                    return std::forward<R>(r) + zero_suffixed<R>::N - 1;
                else
                    return std::ranges::end(std::forward<R>(r));
            }
        };
    }

    inline constexpr auto begin = std::ranges::begin;
    inline constexpr auto end   = detail::end{};
}}
