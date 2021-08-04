#pragma once

/// Note:
/// this file is base on a RUST version:
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/diff.rs and
/// https://github.com/wiryls/far/blob/deprecated/gtk4-rs/src/far/faregex.rs

#include <concepts>
#include <type_traits>
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
            requires (N > 0) &&
            requires (C const (&c)[N]) { {c + N } -> std::contiguous_iterator; }
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
    //// change
    namespace change
    {
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
        using variant = std::variant
            < std::monostate
            , retain<C, I>
            , remove<C, I>
            , insert<C> >;
    }

    enum change_type
    {
        none   = 0,
        retain = 1,
        remove = 2,
        insert = 3,
    };

    //// faregex
    template<::far::cep::char_type C>
    class faregex;

    enum option : int
    {
        normal_mode = 0b01,
        ignore_case = 0b10,
    };

    template<std::ranges::forward_range P, std::ranges::forward_range R>
    faregex(P &&, R &&, option = option{}) -> faregex<std::ranges::range_value_t<P>>;
}

//// implementation starts from here

template<::far::cep::char_type C>
class far::faregex
{
private:

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

        using string_type = std::basic_string<C>;
        using string_iterator_type = typename string_type::const_iterator;

        using       prefered_searcher = std::boyer_moore_searcher<string_iterator_type>;
        using       fallback_searcher = std::    default_searcher<string_iterator_type>;
        using icase_prefered_searcher = std::boyer_moore_searcher<string_iterator_type, std::hash<C>, comparator>;
        using icase_fallback_searcher = std::    default_searcher<string_iterator_type, comparator>;
        using default_group = std::tuple<      prefered_searcher,       fallback_searcher>;
        using   icase_group = std::tuple<icase_prefered_searcher, icase_fallback_searcher>;
        using         group = std::variant<default_group, icase_group>;

        template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
        requirement(P const & pattern, R const & replace, bool ignore_case)
            : pattern(aux::begin(pattern), aux::end(pattern))
            , replace(aux::begin(replace), aux::end(replace))
            // Note: searcher may have a reference to pattern, so I store pattern as a member.
            // Be careful that do not change pattern. Any reallocation may break our searcher,
            // as well as move a (SSOed) small string.
            , searcher(make_group(this->pattern, ignore_case))
        {}

        // Disable copy and move in order to avoid breaking searcher.
        // Otherwise we need to rebind searcher to pattern manually.
        requirement            (requirement      &&) = delete;
        requirement & operator=(requirement      &&) = delete;
        requirement            (requirement const &) = delete;
        requirement & operator=(requirement const &) = delete;

        auto static make_group(string_type const & string, bool ignore_case) -> group
        {
            if (ignore_case)
            {
                auto compare = comparator{ std::locale() };
                return std::make_tuple
                    ( icase_prefered_searcher(string.begin(), string.end(), std::hash<C>(), compare)
                    , icase_fallback_searcher(string.begin(), string.end(), compare) );
            }
            else
            {
                return std::make_tuple
                    ( prefered_searcher(string.begin(), string.end())
                    , fallback_searcher(string.begin(), string.end()) );
            }
        }

        string_type pattern;
        string_type replace;
        group       searcher;
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
        auto constexpr operator ()() -> change::variant<C, I>
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

                    if /**/ (auto const u = std::get_if<ucase>(&(context->searcher)))
                        next = std::get<index>(*u)(head, tail);
                    else if (auto const i = std::get_if<icase>(&(context->searcher)))
                        next = std::get<index>(*i)(head, tail);
                    else
                        break;

                    if (next.first == next.second)
                        break;

                    if (head != next.first)
                    {
                        stage = state::after_retain;
                        return change::retain<C, I>{ head, next.first };
                    }

            [[fallthrough]];
            case state::after_retain:

                    {
                        stage = state::after_remove;
                        return change::remove<C, I>(next);
                    }

            [[fallthrough]];
            case state::after_remove:

                    if (!context->replace.empty())
                    {
                        stage = state::after_insert;
                        return change::insert<C>(context->replace);
                    }

            [[fallthrough]];
            case state::after_insert:

                    head = next.second;
                }


            [[fallthrough]];
            case state::before_stopped:

                if (head != tail)
                {
                    stage = state::stopped;
                    return change::retain<C, I>{ head, tail };
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
            noexcept(noexcept(first = last, first == last, first = std::move(last)))
            : stage
                { context == nullptr || first == last
                ? state::stopped
                : context->pattern == context->replace
                ? state::before_stopped
                : state::starting }
            , context(context)
            , next()
            , head(std::move(first))
            , tail(std::move(last))
        {}

    private:
        enum struct state { starting, after_retain, after_remove, after_insert, before_stopped, stopped };

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
        auto constexpr operator ()() -> change::variant<C, I>
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

                    {
                        auto const & match = (*head)[0];
                        if (match.first == match.second ||
                            std::equal(std::execution::unseq, buffer.begin(), buffer.end(), match.first, match.second) )
                            [[unlikely]]
                            continue;

                        if (current != match.first)
                        {
                            stage = state::after_retain;
                            return change::retain<C, I>{ current, match.first };
                        }
                    }

            [[fallthrough]];
            case state::after_retain:

                    {
                        auto const & match = (*head)[0];
                        stage = state::after_remove;
                        return change::remove<C, I>{ match.first, match.second };
                    }

            [[fallthrough]];
            case state::after_remove:

                    if (!buffer.empty())
                    {
                        stage = state::after_insert;
                        return change::insert<C>(buffer);
                    }

            [[fallthrough]];
            case state::after_insert:

                    current = (*head)[0].second;
                }

                stage = state::stopped;
                if (current != done)
                    return change::retain<C, I>{ current, done };

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

    template<::far::cep::matched_iter<C> I>
    struct iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = change::variant<C, I>;
        using pointer           = change::variant<C, I> const *;
        using reference         = change::variant<C, I> const &;

    public:
        using generator_type = std::function<value_type()>;
        explicit constexpr iterator(generator_type g = nullptr) noexcept
            : generator(g)
            , next(std::monostate{})
            , last(std::monostate{})
        {
            ++ *this;
        }

    public:
        auto constexpr operator*() const noexcept -> reference
        {
            return last;
        }

        auto constexpr operator->() const noexcept -> pointer
        {
            return &last;
        }

        auto constexpr operator++() noexcept -> iterator &
        {
            if (!static_cast<bool>(generator)) [[unlikely]]
                return *this;

            // generate last
            // generate next
            // if last can merge with next (#3)
            //    merge last with next
            //    generate next
            //    retry #3

            if (last.index() == far::none) [[unlikely]]
            {
                last = generator();
            }
            else
            {
                last = std::move(next);
            }

            if (last.index() == far::none)
            {
                generator = nullptr;
                return *this;
            }

            
            for (next = generator(); last.index() == next.index(); next = generator())
            {
                switch (last.index())
                {
                default:
                case far::none: // should never arrive here, or maybe throw ?
                {
                    last = std::monostate{};
                    next = std::monostate{};
                    generator = nullptr;
                    return *this;
                }
                case far::retain:
                {
                    auto l = std::get_if<far::retain>(&last);
                    auto r = std::get_if<far::retain>(&next);
                    if (l->second == r->first)
                        l->second = r->second;
                    else
                        return *this;
                }
                case far::remove:
                {
                    auto l = std::get_if<far::remove>(&last);
                    auto r = std::get_if<far::remove>(&next);
                    if (l->second == r->first)
                        l->second = r->second;
                    else
                        return *this;
                }
                case far::insert:
                {
                    auto l = std::get_if<far::insert>(&last);
                    auto r = std::get_if<far::insert>(&next);
                    if (l->end() == r->begin())
                        *l = std::string_view(l->begin(), r->end());
                    else
                        return *this;
                }
                }
            }

            return *this;
        }

        auto constexpr operator++(int) noexcept -> iterator
        {
            auto iter = *this;
            ++ *this;
            return iter;
        }

    private:
        friend auto constexpr operator==(iterator const & lhs, iterator const & rhs) -> bool
        {
            return (!static_cast<bool>(lhs.generator) && !static_cast<bool>(rhs.generator))
                // TODO: check whether generators are equal
                ;
        }

        friend auto constexpr operator!=(iterator const & lhs, iterator const & rhs) -> bool
        {
            return !(lhs == rhs);
        }

        generator_type  generator;
        value_type      next;
        value_type      last;
    };

public:

    template<::far::cep::matched<C> R>
    [[nodiscard]] auto constexpr operator()(R & container) const
        -> std::function<change::variant<C, std::ranges::iterator_t<R>>()>
    {
        return this->operator()(aux::begin(container), aux::end(container));
    }

    template<::far::cep::matched_iter<C> I>
    [[nodiscard]] auto constexpr operator()(I first, I last) const
        -> std::function<change::variant<C, I> ()>
    {
        using return_type = std::function<change::variant<C, I> ()>;
        using normal_type = std::shared_ptr<requirement<mode::normal> const>;
        using regexp_type = std::shared_ptr<requirement<mode::regexp> const>;

        if /**/ (auto normal = std::get_if<normal_type>(&pointer))
            return generator<mode::normal, I>(*normal, std::move(first), std::move(last));
        else if (auto regexp = std::get_if<regexp_type>(&pointer))
            return generator<mode::regexp, I>(*regexp, std::move(first), std::move(last));
        else
            return []() -> change::variant<C, I> { return std::monostate{}; };
    }

    [[nodiscard]] constexpr operator bool() const
    {
        return pointer.index() != 0;
    }

public:

    template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
    faregex(P const & pattern, R const & replace, option options = option{})
        : pointer()
    {
        auto is_normal_mode = (options & option::normal_mode) != 0;
        auto do_ignore_case = (options & option::ignore_case) != 0;

        if (is_normal_mode)
        {
            pointer = std::make_shared<requirement<mode::normal>>(pattern, replace, do_ignore_case);
        }
        else try
        {
            pointer = std::make_shared<requirement<mode::regexp>>
                // Note: C++20 needed
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
        std::shared_ptr<requirement<mode::normal> const>, // Safety: const is necessary.
        std::shared_ptr<requirement<mode::regexp> const>
    > pointer;
};
