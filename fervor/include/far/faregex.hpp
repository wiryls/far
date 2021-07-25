#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <string>
#include <regex>

namespace far
{
    //// common concepts

    template<typename C>
    concept char_type
        = std::same_as<typename std:: char_traits<C>::char_type, C>
       && std::same_as<typename std::regex_traits<C>::char_type, C>;

    template<typename T, typename C>
    concept string_type
        = std::ranges::forward_range<T>
       && std::convertible_to
            < std::ranges::range_value_t<T>
            , typename std::char_traits<C>::char_type >;

    //// faregex with deduction guide

    template<far::char_type C>
    class faregex;

    template<std::ranges::forward_range P, std::ranges::forward_range R>
    faregex(P &&, R &&, bool = false) -> faregex<std::ranges::range_value_t<P>>;
}

template<far::char_type C>
class far::faregex
{
public:
    template<far::string_type<C> P, far::string_type<C> R>
    faregex(P && pattern, R && replace, bool ignore_case = false);

public:
    template<std::ranges::bidirectional_range R>
    auto operator()(R && something) -> void;

    template<std::bidirectional_iterator I>
    auto operator()(I const & first, I const & last) -> void;

private:
    std::basic_regex <C> pattern;
    std::basic_string<C> replace;
};

template<far::char_type C>
template<far::string_type<C> P, far::string_type<C> R> inline
far::faregex<C>::faregex(P && pattern, R && replace, bool ignore_case)
    : pattern
        ( std::ranges::begin(std::forward<P>(pattern))
        , std::ranges::end  (std::forward<P>(pattern))
        , static_cast<std::regex::flag_type>((ignore_case ? std::regex::icase : 0) | std::regex::ECMAScript) )
    , replace
        ( std::ranges::begin(std::forward<R>(replace))
        , std::ranges::end  (std::forward<R>(replace)) )
{}


template<far::char_type C>
template<std::ranges::bidirectional_range R> inline auto
far::faregex<C>::faregex::operator()(R && container) -> void
{
    this->operator()
        ( std::ranges::begin(std::forward<R>(container))
        , std::ranges::end  (std::forward<R>(container)) );
}

template<far::char_type C>
template<std::bidirectional_iterator I> inline auto
far::faregex<C>::faregex::operator()(I const & first, I const & last) -> void
{
    using iterator_t = std::regex_iterator<I, C>;

    auto head = iterator_t(first, last, pattern);
    auto tail = iterator_t();
    for ( ; head != tail; ++head)
    {
        auto const & match = *head;
        (void) match;
    }
}
