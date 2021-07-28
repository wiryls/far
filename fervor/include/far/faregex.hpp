#pragma once

#include <concepts>
#include <execution> // std::execution::unseq
#include <algorithm> // std::equal
#include <iterator>
#include <ranges>
#include <memory>
#include <variant>
#include <string>
#include <string_view>
#include <regex>

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

namespace far
{
    //// changes

    template
        < ::far::cep::char_type C
        , ::far::cep::matched_iter<C> I
        > struct retain : std::pair<I, I>
    {
        using std::pair<I, I>::pair;
    };

    template
        < ::far::cep::char_type C
        , ::far::cep::matched_iter<C> I
        > struct remove : std::pair<I, I>
    {
        using std::pair<I, I>::pair;
    };

    template
        < ::far::cep::char_type C
        > using insert = std::basic_string_view<C>;

    template
        < ::far::cep::char_type C
        , std::bidirectional_iterator I
        > using change = std::variant
            < std::monostate
            , retain<C, I>
            , remove<C, I>
            , insert<C> >;

    //// iterator
    template
        < ::far::cep::char_type C
        > class iterator;

    //// faregex
    template
        < ::far::cep::char_type C
        > class faregex;

    template
        < std::ranges::forward_range P
        , std::ranges::forward_range R
        > faregex(P &&, R &&, bool = false) -> faregex<std::ranges::range_value_t<P>>;
}

//// implementation starts from here

template<::far::cep::char_type C>
class far::faregex
{
private:
    struct shared
    {
        std::basic_regex <C> pattern;
        std::basic_string<C> replace;
    };

public:
    template<::far::cep::matched_iter<C> I>
    struct generator
    {
    private:
        enum struct state
        {
            starting,
            after_retain,
            after_remove,
            after_insert,
            stopped,
        };

        using          buffer_type = std::basic_string<C>;
        using         context_type = std::shared_ptr<shared>;
        using target_iterator_type = I;
        using worker_iterator_type = std::regex_iterator<I, C>;

    public:
        generator(std::shared_ptr<shared> const & context, I && first, I && last);

    public:
        auto operator ()()->change<C, I>;

    private:
                       state stat;
                context_type ctxt;
        target_iterator_type prev;
        worker_iterator_type head;
        worker_iterator_type tail;
                 buffer_type buff;
    };

public:
    template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
    faregex(P const & pattern, R const & replace, bool ignore_case = false);

public:
    template<::far::cep::matched<C> R>
    auto operator()(R const & something) -> generator<std::ranges::iterator_t<R const>>;

    template<::far::cep::matched_iter<C> I>
    auto operator()(I && first, I && last) -> generator<I>;

private:
    std::shared_ptr<shared> data;
};

template<::far::cep::char_type C>
template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
inline far::faregex<C>::
faregex(P const & pattern, R const & replace, bool ignore_case)
    : data(std::make_shared<shared>
        // note: C++20 needed
        // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0960r3.html
        ( std::basic_regex<C>
            ( std::ranges::begin(pattern)
            , std::ranges::end  (pattern)
            , static_cast<std::regex::flag_type>
                ( (ignore_case ? std::regex::icase : 0)
                | (std::regex::ECMAScript) ) )
        , std::basic_string<C>
            ( std::ranges::begin(replace)
            , std::ranges::end  (replace) ) ) )
{}

template<::far::cep::char_type C>
template<::far::cep::matched<C> R>
inline auto far::faregex<C>::faregex::
operator()(R const & container) -> generator<std::ranges::iterator_t<R const>>
{
    return this->operator()
        ( std::ranges::begin(container)
        , std::ranges::end  (container) );
}

template<::far::cep::char_type C>
template<::far::cep::matched_iter<C> I>
inline auto far::faregex<C>::faregex::
operator()(I && first, I && last) -> generator<I>
{
    return generator<I>(data, std::forward<I>(first), std::forward<I>(last));
}

template<::far::cep::char_type C>
template<::far::cep::matched_iter<C> I>
inline far::faregex<C>::generator<I>::
generator(std::shared_ptr<::far::faregex<C>::shared> const & ctxt, I && first, I && last)
    : stat(state::starting)
    , ctxt(ctxt)
    , prev(std::forward<I>(first))
    , head(prev, std::forward<I>(last), ctxt->pattern)
    , tail()
    , buff()
{}

template<::far::cep::char_type C>
template<::far::cep::matched_iter<C> I>
inline auto far::faregex<C>::generator<I>::
operator ()() -> change<C, I>
{
    // note: because coroutines from C++20 are hard to use and have poor
    // performance (in contrast to for-loop), so I choose duff's device.
    // https://www.reddit.com/r/cpp/comments/gqi0io
    // https://stackoverflow.com/questions/57726401

    switch (stat)
    {
    default:
    case state::starting:

        for (; head != tail; ++ head)
        {
            if (head->empty()) [[unlikely]]
                continue;

            buff.clear();
            head->format(std::back_inserter(buff), ctxt->replace);

            if  ( auto const & match = (*head)[0]
                ; std::equal
                    ( std::execution::unseq
                    , buff.begin(), buff.end()
                    , match.first, match.second ) ) [[unlikely]]
                continue;

            if (auto const & match = (*head)[0]; prev != match.first )
            {
                stat = state::after_retain;
                return retain<C, I>{ prev, match.first };
            }

    [[fallthrough]];
    case state::after_retain:

            if (auto const & match = (*head)[0]; match.first != match.second )
            {
                stat = state::after_remove;
                return remove<C, I>{ match.first, match.second };
            }

    [[fallthrough]];
    case state::after_remove:

            if (!buff.empty())
            {
                stat = state::after_insert;
                return insert<C>(buff);
            }

    [[fallthrough]];
    case state::after_insert:

            prev = (*head)[0].second;
        }

        stat = state::stopped;

    [[fallthrough]];
    case state::stopped:

        return std::monostate{};
    }
}
