#pragma once

/// Note:
/// this file is base on a RUST version:
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/diff.rs and
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/faregex.rs

#include <concepts>
#include <execution> // std::execution::unseq
#include <algorithm> // std::equal
#include <iterator>
#include <ranges>
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <regex>

namespace far { namespace cep
{
    /// common concepts for far

    template<typename C>
    concept char_type
        = std::same_as<typename std:: char_traits<C>::char_type, C>
       && std::same_as<typename std::regex_traits<C>::char_type, C>;

    template<typename S, typename C>
    concept string_like
       = std::ranges::forward_range<S>
      && std::convertible_to
            < std::ranges::range_value_t<S>
            , typename std::char_traits<C>::char_type >;
}}

namespace far
{
    //// changes
    template
        < far::cep::char_type C
        , std::bidirectional_iterator I >
    using change = std::variant
        < std::pair<I, I>
        , std::pair<I, I>
        , std::basic_string<C> >;

    //// iterator
    template<far::cep::char_type C>
    class iterator;

    //// faregex (with deduction guide)
    template<far::cep::char_type C>
    class faregex;

    template<std::ranges::forward_range P, std::ranges::forward_range R>
    faregex(P &&, R &&, bool = false) -> faregex<std::ranges::range_value_t<P>>;
}

//// implementation starts from here

template<far::cep::char_type C>
class far::faregex
{
public:
    template<far::cep::string_like<C> P, far::cep::string_like<C> R>
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

template<far::cep::char_type C>
template<far::cep::string_like<C> P, far::cep::string_like<C> R> inline
far::faregex<C>::faregex(P && pattern, R && replace, bool ignore_case)
    : pattern
        ( std::ranges::begin(std::forward<P>(pattern))
        , std::ranges::end  (std::forward<P>(pattern))
        , static_cast<std::regex::flag_type>
            ( (ignore_case ? std::regex::icase : 0)
            | (std::regex::ECMAScript) ) )
    , replace
        ( std::ranges::begin(std::forward<R>(replace))
        , std::ranges::end  (std::forward<R>(replace)) )
{}

template<far::cep::char_type C>
template<std::ranges::bidirectional_range R> inline auto
far::faregex<C>::faregex::operator()(R && container) -> void
{
    this->operator()
        ( std::ranges::begin(std::forward<R>(container))
        , std::ranges::end  (std::forward<R>(container)) );
}

template<far::cep::char_type C>
template<std::bidirectional_iterator I> inline auto
far::faregex<C>::faregex::operator()(I const & first, I const & last) -> void
{
    using iterator_t = std::regex_iterator<I, C>;

    auto head = iterator_t(first, last, pattern);
    auto tail = iterator_t();

    auto prev = first;
    auto buff = std::basic_string<C>();

    for (; head != tail; ++head)
    {
        auto & matches = *head;
        if (!matches.empty()) [[likely]]
        {
            buff.clear();
            matches.format(std::back_inserter(buff), replace);

            auto remove = static_cast<std::pair<I, I>>(matches[0]);
            auto retain = std::make_pair(prev, remove.first);
            auto insert = std::make_pair(buff.begin(), buff.end());

            if (std::equal
                ( std::execution::unseq
                , insert.first, insert.second
                , remove.first, remove.second ) ) [[unlikely]]
                continue;

            if (retain.first != retain.second)
            {
                // TODO: output retain
            }

            if (remove.first != remove.second)
            {
                // TODO: output remove
            }

            if (insert.first != insert.second)
            {
                // TODO: output insert
            }

            prev = remove.second;
        }
    }
}
