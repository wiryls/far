#pragma once

/// Note:
/// this file is base on a RUST version:
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/diff.rs and
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/faregex.rs

#include <execution>
#include <algorithm>
#include <functional>
#include <utility>
#include <memory>
#include <iterator>
#include <ranges>
#include <optional>
#include <variant>
#include <string>
#include <string_view>

#include "foundation.hpp"

namespace far { namespace detail
{
    template<std::input_or_output_iterator I>
    struct iter_pair : std::pair<I, I>
    {
        using std::pair<I, I>::pair;

        auto constexpr begin() const noexcept -> I
        {
            return this->first;
        }

        auto constexpr end() const noexcept -> I
        {
            return this->second;
        }
    };
}}

namespace far
{
    //// changes

    template<::far::cep::char_type C, ::far::cep::matched_iter<C> I>
    struct retain : detail::iter_pair<I>
    {
        using detail::iter_pair<I>::iter_pair;
    };

    template<::far::cep::char_type C, ::far::cep::matched_iter<C> I>
    struct remove : detail::iter_pair<I>
    {
        using detail::iter_pair<I>::iter_pair;
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

    enum option
    {
        normal_mode = 0b01,
        ignore_case = 0b10,
    };

    template<std::ranges::forward_range P, std::ranges::forward_range R>
    faregex(P &&, R &&, option = 0) -> faregex<std::ranges::range_value_t<P>>;
}

//// implementation starts from here

template<::far::cep::char_type C>
class far::faregex
{
public:
    enum struct mode
    {
        normal,
        regexp,
    };

    template<mode M>
    struct requirement;

    template<>
    struct requirement<mode::normal>
    {
        struct comparator
        {
            std::locale locale;
            auto operator()(C lhs, C rhs) const noexcept -> bool
            {
                return std::tolower(lhs, locale) == std::tolower(rhs, locale);
            }
        };

        using prefered_searcher = std::boyer_moore_searcher
            < typename std::basic_string_view<C>::iterator >;
        using fallback_searcher = std::default_searcher
            < typename std::basic_string_view<C>::iterator >;
        using icase_prefered_searcher = std::boyer_moore_searcher
            < typename std::basic_string_view<C>::iterator, std::hash<C>, comparator >;
        using icase_fallback_searcher = std::default_searcher
            < typename std::basic_string_view<C>::iterator, comparator >;

        using default_group = std::tuple<      prefered_searcher,       fallback_searcher>;
        using   icase_group = std::tuple<icase_prefered_searcher, icase_fallback_searcher>;
        using         group = std::variant<default_group, icase_group>;

        group                searcher;
        std::basic_string<C> replace;
    };

    template<>
    struct requirement<mode::regexp>
    {
        std::basic_regex <C> pattern;
        std::basic_string<C> replace;
    };

    template<mode M, ::far::cep::matched_iter<C> I>
    struct generator;

    template<::far::cep::matched_iter<C> I>
    struct generator<mode::normal, I>
    {
    public:
        auto constexpr operator ()() -> change<C, I>
        {
            // Note: because coroutines from C++20 are hard to use and have
            // poor performance (in contrast to for-loop), I choose duff's
            // device.
            // https://www.reddit.com/r/cpp/comments/gqi0io
            // https://stackoverflow.com/questions/57726401

            switch (stage)
            {
            case state::starting:

                for (; head != tail; )
                {
                    using ucase = requirement<mode::normal>::default_group;
                    using icase = requirement<mode::normal>::  icase_group;
                    using   tag = std::iterator_traits<I>::iterator_category;
                    enum { index = !std::is_base_of_v<std::random_access_iterator_tag, tag> };

                    if /**/ (auto u = std::get_if<ucase>(&(context->searcher)))
                        next = std::get<index>(*u)(head, tail);
                    else if (auto i = std::get_if<icase>(&(context->searcher)))
                        next = std::get<index>(*i)(head, tail);
                    else
                        break;

                    if (head != next.first)
                    {
                        stage = state::after_retain;
                        return retain<C, I>{ head, next.first };
                    }

            [[fallthrough]];
            case state::after_retain:

                    if (next.first != next.second)
                    {
                        stage = state::after_remove;
                        return remove<C, I>(next);

            [[fallthrough]];
            case state::after_remove:

                        if (!context->replace.empty())
                        {
                            stage = state::after_insert;
                            return insert<C>(context->replace);
                        }
                    }

            [[fallthrough]];
            case state::after_insert:

                    head = next.second;
                }

                if (head != tail)
                {
                    stage = state::stopped;
                    return retain<C, I>{ head, tail };
                }

            [[fallthrough]];
            case state::stopped:
            default:

                return std::monostate{};
            }
        }

    public:
        using context_pointer = std::shared_ptr<requirement<mode::normal> const>;
        constexpr generator(context_pointer const & context, I first, I last)
            noexcept(noexcept(first = last, first = std::move(last)))
            : stage(state::starting)
            , context(context)
            , next()
            , head(std::move(first))
            , tail(std::move(last))
        {
            if (context == nullptr || head == tail)
                stage = state::stopped;
        }

    private:
        enum struct state { starting, after_retain, after_remove, after_insert, stopped };

        state           stage;
        context_pointer context;
        std::pair<I, I> next;
        I               head;
        I               tail;
    };

    template<::far::cep::matched_iter<C> I>
    struct generator<mode::regexp, I>
    {
    public:
        auto constexpr operator ()() -> change<C, I>
        {
            switch (stage)
            {
            case state::starting:

                for (; head != tail; ++ head)
                {
                    if (head->empty()) [[unlikely]]
                        continue;

                    buffer.clear();
                    head->format(std::back_inserter(buffer), context->replace);

                    if  ( auto const & match = (*head)[0]
                        ; std::equal
                            ( std::execution::unseq
                            , buffer.begin(), buffer.end()
                            , match.first, match.second ) ) [[unlikely]]
                        continue;

                    if (auto const & match = (*head)[0]; current != match.first )
                    {
                        stage = state::after_retain;
                        return retain<C, I>{ current, match.first };
                    }

            [[fallthrough]];
            case state::after_retain:

                    if (auto const & match = (*head)[0]; match.first != match.second )
                    {
                        stage = state::after_remove;
                        return remove<C, I>{ match.first, match.second };
                    }

            [[fallthrough]];
            case state::after_remove:

                    if (!buffer.empty())
                    {
                        stage = state::after_insert;
                        return insert<C>(buffer);
                    }

            [[fallthrough]];
            case state::after_insert:

                    current = (*head)[0].second;
                }

                stage = state::stopped;
                if (current != done)
                    return retain<C, I>{ current, done };

            [[fallthrough]];
            case state::stopped:
            default:

                return std::monostate{};
            }
        }

    public:
        using context_pointer = std::shared_ptr<requirement<mode::regexp> const>;
        constexpr generator(context_pointer const & context, I first, I last)
            noexcept(noexcept(first = last, first = std::move(last)))
            : stage(state::starting)
            , context(context)
            , current(std::move(first))
            , done(std::move(last))
            , head()
            , tail()
            , buffer()
        {
            if (context == nullptr || current == done)
                stage = state::stopped;
            else
                head = std::regex_iterator<I>(current, done, context->pattern);
        }

    private:
        enum struct state { starting, after_retain, after_remove, after_insert, stopped };

        state                  stage;
        context_pointer        context;
        I                      current;
        I                      done;
        std::regex_iterator<I> head;
        std::regex_iterator<I> tail;
        std::basic_string<C>   buffer;
    };

public:

    template<::far::cep::matched<C> R>
    [[nodiscard]] auto constexpr operator()(R & container) const -> std::function<change<C, std::ranges::iterator_t<R>>()>
    {
        return this->operator()
            ( aux::begin(container)
            , aux::end  (container) );
    }

    template<::far::cep::matched_iter<C> I>
    [[nodiscard]] auto constexpr operator()(I first, I last) const -> std::function<change<C, I> ()>
    {
        using return_type = std::function<change<C, I> ()>;
        using normal_type = std::shared_ptr<requirement<mode::normal> const>;
        using regexp_type = std::shared_ptr<requirement<mode::regexp> const>;

        if /**/ (auto normal = std::get_if<normal_type>(&pointer))
            return generator<mode::normal, I>(*normal, std::move(first), std::move(last));
        else if (auto regexp = std::get_if<regexp_type>(&pointer))
            return generator<mode::regexp, I>(*regexp, std::move(first), std::move(last));
        else
            return []() -> change<C, I> { return std::monostate{}; };
    }

    [[nodiscard]] constexpr operator bool() const
    {
        return pointer.index() != 0;
    }

public:
    template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
    faregex(P const & pattern, R const & replace, option options = option(0))
        : pointer()
    {
        auto is_normal_mode = (options & option::normal_mode) != 0;
        auto do_ignore_case = (options & option::ignore_case) != 0;

        if (is_normal_mode)
        {
            using       faster = requirement<mode::normal>::      prefered_searcher;
            using       normal = requirement<mode::normal>::      fallback_searcher;
            using icase_faster = requirement<mode::normal>::icase_prefered_searcher;
            using icase_normal = requirement<mode::normal>::icase_fallback_searcher;
            using tag = typename std::iterator_traits<std::ranges::iterator_t<P>>::iterator_category;

            auto copied = std::basic_string<C>(aux::begin(replace), aux::end(replace));
            auto buffer = std::basic_string<C>();
            auto viewer = std::basic_string_view<C>();

            if constexpr (std::is_base_of_v<std::random_access_iterator_tag, tag>)
            {
                viewer = std::basic_string_view<C>(aux::begin(pattern), aux::end(pattern));
            }
            else
            {
                buffer = std::basic_string     <C>(aux::begin(pattern), aux::end(pattern));
                viewer = std::basic_string_view<C>(aux::begin( buffer), aux::end( buffer));
            }

            if (do_ignore_case)
            {
                auto compare = typename requirement<mode::normal>::comparator{ std::locale() };
                pointer = std::make_shared<requirement<mode::normal>>
                    ( std::make_tuple
                        ( icase_faster(viewer.begin(), viewer.end(), std::hash<C>(), compare)
                        , icase_normal(viewer.begin(), viewer.end(), compare) )
                    , copied );
            }
            else
            {
                pointer = std::make_shared<requirement<mode::normal>>
                    ( std::make_tuple
                        ( faster(viewer.begin(), viewer.end())
                        , normal(viewer.begin(), viewer.end()) )
                    , copied );
            }
        }
        else try
        {
            pointer = std::make_shared<requirement<mode::regexp>>
                // note: C++20 needed
                // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0960r3.html
                ( std::basic_regex<C>
                    ( aux::begin(pattern)
                    , aux::end  (pattern)
                    , static_cast<std::regex_constants::syntax_option_type>
                        ( (do_ignore_case ? std::regex_constants::icase : 0)
                        | (std::regex_constants::ECMAScript) ) )
                , std::basic_string<C>
                    ( aux::begin(replace)
                    , aux::end  (replace) ) );
        }
        catch ([[maybe_unused]] std::regex_error const& ex)
        {
            // TODO: build an error message
        }
    }

public:
    std::variant<
        std::monostate,
        std::shared_ptr<requirement<mode::normal> const>,
        std::shared_ptr<requirement<mode::regexp> const>
    > pointer;
};
