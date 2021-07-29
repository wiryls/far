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

    template<::far::cep::char_type C, ::far::cep::matched_iter<C> I>
    struct retain : std::pair<I, I>
    {
        using std::pair<I, I>::pair;
    };

    template<::far::cep::char_type C, ::far::cep::matched_iter<C> I>
    struct remove : std::pair<I, I>
    {
        using std::pair<I, I>::pair;
    };

    template<::far::cep::char_type C>
    using insert = std::basic_string_view<C>;

    template<::far::cep::char_type C, std::bidirectional_iterator I>
    using change = std::variant
        < std::monostate
        , retain<C, I>
        , remove<C, I>
        , insert<C> >;

    //// iterator
    template<::far::cep::char_type C>
    class iterator;

    //// faregex
    template<::far::cep::char_type C>
    class faregex;

    template<std::ranges::forward_range P, std::ranges::forward_range R>
    faregex(P &&, R &&, bool = false) -> faregex<std::ranges::range_value_t<P>>;
}

namespace far { namespace detail
{
    //// helpers

    namespace cpo
    {
        struct end
        {
        private:
            template<std::ranges::range R>
            struct zero_suffixed : std::false_type {};

            template<typename C, std::size_t N>
            requires (N > 0)
            struct zero_suffixed<C(&)[N]> : std::true_type
            {
                static constexpr std::ptrdiff_t N = N;
            };

        public:
            template<std::ranges::range R>
            auto constexpr operator()(R && r) const noexcept(noexcept(std::ranges::end(r)))
            {
                if constexpr  (zero_suffixed<R>::value)
                    return r + zero_suffixed<R>::N - 1;
                else
                    return std::ranges::end(std::forward<R>(r));
            }
        };
    }

    inline constexpr auto begin = std::ranges::begin;
    inline constexpr auto end   = cpo::end{};
}}

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
    public:
        auto constexpr operator ()() -> change<C, I>
        {
            // Note: because coroutines from C++20 are hard to use and have
            // poor performance (in contrast to for-loop), so I choose duff's
            // device.
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
                if (prev != last)
                    return retain<C, I>{ prev, last };

            [[fallthrough]];
            case state::stopped:

                return std::monostate{};
            }
        }

    public:
        constexpr generator(std::shared_ptr<shared> const& context, I first, I last)
            noexcept(noexcept(first = last, first = std::move(last)))
            : stat(state::starting)
            , ctxt(context)
            , prev(first)
            , last(last)
            , head(std::move(first), std::move(last), ctxt->pattern)
            , tail()
            , buff()
        {}

    private:
        enum struct state
        {
            starting,
            after_retain,
            after_remove,
            after_insert,
            stopped,
        };

        state                   stat;
        std::shared_ptr<shared> ctxt;
        I                       prev;
        I                       last;
        std::regex_iterator<I>  head;
        std::regex_iterator<I>  tail;
        std::basic_string<C>    buff;
    };

public:
    template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
    faregex(P const & pattern, R const & replace, bool ignore_case = false)
        : data(std::make_shared<shared>
            // note: C++20 needed
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0960r3.html
            ( std::basic_regex<C>
                ( detail::begin(pattern)
                , detail::end  (pattern)
                , static_cast<std::regex_constants::syntax_option_type>
                    ( (ignore_case ? std::regex_constants::icase : 0)
                    | (std::regex_constants::ECMAScript) ) )
            , std::basic_string<C>
                ( detail::begin(replace)
                , detail::end  (replace) ) ) )
    {}

public:
    template<::far::cep::matched<C> R>
    auto constexpr operator()(R & container) const -> generator<std::ranges::iterator_t<R>>
    {
        return this->operator()
            ( detail::begin(container)
            , detail::end  (container) );
    }

    template<::far::cep::matched_iter<C> I>
    auto constexpr operator()(I first, I last) const -> generator<I>
    {
        return generator<I>(data, std::move(first), std::move(last));
    }

public:
    std::shared_ptr<shared> data;
};
