#pragma once

// this file is a new version of faregex.hpp

#include <type_traits>
#include <concepts>
#include <iterator>
#include <execution>
#include <algorithm>
#include <ranges>
#include <optional>
#include <string>
#include <regex>

namespace far { namespace scan
{
    // char types that scanner supports.
    template<typename C>
    concept scannable
        = std::same_as<typename std::regex_traits<C>::char_type, C>
       && std::same_as<typename std:: char_traits<C>::char_type, C>
        ;

    // iterators of scannable.
    template<typename I, typename C>
    concept iterative
        = scannable<C>
       && std::input_or_output_iterator<I>
       && std::same_as<std::iter_value_t<I>, C>
        ;

    // forward iterators of scannable.
    // - required by std::regex constructor.
    template<typename I, typename C>
    concept forward_iterative
        = iterative<I, C>
       && std::forward_iterator<I>
        ;

    // bidirectional iterators of scannable.
    // - required by std::default_searcher.
    template<typename I, typename C>
    concept bidirectional_iterative
        = iterative<I, C>
       && std::forward_iterator<I>
        ;

    // ranges of scannable.
    template<typename S, typename C>
    concept sequence
        = std::ranges::range<S>
       && iterative<std::ranges::iterator_t<S>, C>
        ;

    // forward ranges of scannable.
    // - used when constructing scanner.
    template<typename S, typename C>
    concept forward_sequence
        = std::ranges::range<S>
       && forward_iterative<std::ranges::iterator_t<S>, C>
        ;

    // bidirectional ranges of scannable.
    // - used when constructing scanner.
    template<typename S, typename C>
    concept bidirectional_sequence
        = std::ranges::range<S>
       && bidirectional_iterative<std::ranges::iterator_t<S>, C>
        ;

    // a function type to output result.
    template<typename F, typename I>
    concept output = requires (F fun, I first, I last)
    {
        { fun(first, last) } -> std::same_as<void>;
    };
}}

namespace far { namespace scan { namespace aux
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
            requires (C const (&c)[N])
            {
                { c + N } -> std::contiguous_iterator;
            }
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
}}}

namespace far { namespace scan
{
    //// helpers

    template<::far::scan::scannable C>
    struct icase_comparator
    {
        std::locale locale;
        auto operator()(C lhs, C rhs) const noexcept -> bool
        {
            return std::tolower(lhs, locale) == std::tolower(rhs, locale);
        }
    };

    template<::far::scan::scannable C, std::invocable<C, C> F = std::equal_to<>>
    struct basic_or_icase_core
    {
    public: // types
        using string          = std::basic_string<C>;
        using string_iterator = typename string::const_iterator;
        using prefered        = std::boyer_moore_searcher<string_iterator, std::hash<C>, F>;
        using fallback        = std::    default_searcher<string_iterator, F>;
        using switcher        = std::pair<prefered, fallback>;

    public: // data
        string   pattern;
        string   replace;
        switcher chooser;

    public: // constructors and assignments
        template<::far::scan::sequence<C> P, ::far::scan::sequence<C> R>
        basic_or_icase_core(P const & p, R const & r)
            : pattern(::far::scan::aux::begin(p), ::far::scan::aux::end(p))
            , replace(::far::scan::aux::begin(r), ::far::scan::aux::end(r))
            , chooser(bind(pattern))
        {}
        basic_or_icase_core(basic_or_icase_core && rhs)
            : pattern(std::move(rhs.pattern))
            , replace(std::move(rhs.replace))
            , chooser(bind(pattern))
        {}
        basic_or_icase_core(basic_or_icase_core const & rhs)
            : pattern(rhs.pattern)
            , replace(rhs.replace)
            , chooser(bind(pattern))
        {}
        basic_or_icase_core & operator=(basic_or_icase_core && rhs)
        {
            pattern = std::move(rhs.pattern);
            replace = std::move(rhs.replace);
            chooser = bind(pattern);
            return this;
        }
        basic_or_icase_core & operator=(basic_or_icase_core const & rhs)
        {
            pattern = rhs.pattern;
            replace = rhs.replace;
            chooser = bind(pattern);
            return this;
        }

    private:
        auto static bind(string const & that) -> switcher
        {
            // Note: searcher may have a reference to pattern, so I store pattern
            // as a member. Be careful that do not change pattern. Any reallocation
            // may break our searcher, as well as move a (SSOed) small string.
            return std::make_pair
                ( prefered(that.begin(), that.end(), std::hash<C>{}, F{})
                , fallback(that.begin(), that.end(), F{}) );
        }
    };

    template<typename T>
    struct to_predicate
    {
        using type = T &;
    };

    template<typename T>
    requires (sizeof(T) <= sizeof(void *)
        &&   (std::is_trivially_copy_constructible_v<T>))
        &&   (std::is_trivially_destructible_v<T>)
    struct to_predicate<T>
    {
        using type = T;
    };

    template<typename T>
    using predicate = typename to_predicate<T>::type;

    template
        < ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R /* retain */
        , ::far::scan::output<I> O /* remove */
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N /* insert */
        >
    struct collector
    {
        predicate<std::remove_reference_t<R>> retain;
        predicate<std::remove_reference_t<O>> remove;
        predicate<std::remove_reference_t<N>> insert;
    };

    enum struct execution_status
    {
        stopped,
        startup,
        after_retain,
        after_remove,
        after_insert,
        before_stopped,
    };
}}

namespace far { namespace scan
{
    //// shared

    enum struct mode
    {
        basic,
        icase,
        regex,
    };

    //// core (immutable)

    template<mode M, ::far::scan::scannable C>
    struct core;

    template<::far::scan::scannable C>
    struct core<mode::basic, C> : basic_or_icase_core<C>
    {
        template<::far::scan::sequence<C> P, ::far::scan::sequence<C> R>
        core(P const & pattern, R const & replace)
            : basic_or_icase_core<C>(pattern, replace)
        {}
    };

    template<::far::scan::scannable C>
    struct core<mode::icase, C> : basic_or_icase_core<C, icase_comparator<C>>
    {
        template<::far::scan::sequence<C> P, ::far::scan::sequence<C> R>
        core(P const & pattern, R const & replace)
            : basic_or_icase_core<C, icase_comparator<C>>(pattern, replace)
        {}
    };

    template<::far::scan::scannable C>
    struct core<mode::regex, C>
    {
        template<::far::scan::forward_sequence<C> P, ::far::scan::sequence<C> R>
        core(P const & pattern, R const & replace, bool ignore_case)
            : pattern()
            , replace(aux::begin(replace), aux::end(replace))
            , error()
        {
            try
            {
                using option_type = std::regex_constants::syntax_option_type;
                auto constexpr basic = std::regex_constants::ECMAScript;
                auto constexpr icase = std::regex_constants::icase;

                auto o = static_cast<option_type>(basic | (ignore_case ? icase : 0));
                this->pattern = std::basic_regex<C>(aux::begin(pattern), aux::end(pattern), o);
            }
            catch (std::regex_error const & ex)
            {
                error = ex.code();
            }
        }

        std::basic_regex <C> pattern;
        std::basic_string<C> replace;
        std::optional<std::regex_constants::error_type> error;
    };

    //// generator

    template
        < mode M
        , ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R /* retain */
        , ::far::scan::output<I> O /* remove */
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N /* insert */
        >
    class generator;

    // attention:
    // lifetime of references must be longer than this generator.
    template
        < mode M
        , ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R
        , ::far::scan::output<I> O
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N
        >
    generator(core<M, C> const &, R &, O &, N &, I, I) -> generator
        < M
        , C
        , I
        , std::remove_reference_t<R>
        , std::remove_reference_t<O>
        , std::remove_reference_t<N>
        >;

    template
        < mode M
        , ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R /* retain */
        , ::far::scan::output<I> O /* remove */
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N /* insert */
        >
    requires (M == mode::basic || M == mode::icase)
    class generator<M, C, I, R, O, N> : collector<C, I, R, O, N>
    {
    public:
        auto operator()() -> bool
        {
            // Note: function objects are passed by value when passed to and from functions.
            // https://stackoverflow.com/questions/17626667

            // Note: because coroutines from C++20 are hard to use and have
            // poor performance (in contrast to for-loop), I choose duff's
            // device.
            // https://stackoverflow.com/questions/57726401
            // https://www.reddit.com/r/cpp/comments/gqi0io

            using fun = collector<C, I, R, O, N>;
            switch (stat)
            {
            case execution_status::startup:

                while (head != tail)
                {
                    using tag = std::iterator_traits<I>::iterator_category;
                    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, tag>)
                        curr = (rule.chooser. first)(head, tail);
                    else
                        curr = (rule.chooser.second)(head, tail);

                    if (curr.first == curr.second)
                        break;

                    if (head != curr.first)
                    {
                        stat = execution_status::after_retain;
                        fun::retain(head, curr.first);
                        return true;
                    }

            [[fallthrough]];
            case execution_status::after_retain:

                    {
                        stat = execution_status::after_remove;
                        fun::remove(curr.first, curr.second);
                        return true;
                    }

            [[fallthrough]];
            case execution_status::after_remove:

                    if (!rule.replace.empty())
                    {
                        stat = execution_status::after_insert;
                        fun::insert(rule.replace.begin(), rule.replace.end());
                        return true;
                    }

            [[fallthrough]];
            case execution_status::after_insert:

                    head = curr.second;
                }

            [[fallthrough]];
            case execution_status::before_stopped:

                if (head != tail)
                {
                    stat = execution_status::stopped;
                    fun::retain(head, tail);
                    return false;
                }

            [[fallthrough]];
            case execution_status::stopped:
            default:

                return false;
            }
        }

    public:
        constexpr generator
            ( core<M, C> const & rule
            , R & retain
            , O & remove
            , N & insert
            , I first
            , I last
            ) noexcept
            : collector<C, I, R, O, N>{ retain, remove, insert }
            , rule(rule)
            , head(first)
            , tail(last)
        {
            /**/ if (first == last)
                stat = execution_status::stopped;
            else if (rule.pattern == rule.replace)
                stat = execution_status::before_stopped;
        }

    private:
        using iterator = I;

        core<M, C> const &            rule;
        iterator                      head;
        iterator                      tail;
        std::pair<iterator, iterator> curr{};
        execution_status              stat{ execution_status::startup };
    };

    template
        < ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R /* retain */
        , ::far::scan::output<I> O /* remove */
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N /* insert */
        >
    class generator<mode::regex, C, I, R, O, N> : collector<C, I, R, O, N>
    {
    public:
        auto operator()() -> bool
        {
            using fun = collector<C, I, R, O, N>;
            switch (stat)
            {
            case execution_status::startup:

                for (; head != tail; ++ head)
                {
                    if (head->empty()) [[unlikely]]
                        continue;

                    buff.clear();
                    head->format(std::back_inserter(buff), rule.replace);

                    {
                        auto constexpr e = std::execution::unseq;
                        auto const &   m = (*head)[0];
                        auto const &   b = buff;

                        if (m.first == m.second ||
                            std::equal(e, b.begin(), b.end(), m.first, m.second)) [[unlikely]]
                            continue;

                        if (curr != m.first)
                        {
                            stat = execution_status::after_retain;
                            fun::retain(curr, m.first);
                            return true;
                        }
                    }

            [[fallthrough]];
            case execution_status::after_retain:

                    {
                        auto const & match = (*head)[0];
                        stat = execution_status::after_remove;
                        fun::remove(match.first, match.second);
                        return true;
                    }

            [[fallthrough]];
            case execution_status::after_remove:

                    if (!buff.empty())
                    {
                        stat = execution_status::after_insert;
                        fun::insert(buff.begin(), buff.end());
                        return true;
                    }

            [[fallthrough]];
            case execution_status::after_insert:

                    curr = (*head)[0].second;
                }

                stat = execution_status::stopped;
                if (curr != last)
                {
                    fun::retain(curr, last);
                    return false;
                }

            [[fallthrough]];
            case execution_status::stopped:
            default:

                return false;
            }
        }

    public:
        constexpr generator
            ( core<mode::regex, C> const & rule
            , R & retain
            , O & remove
            , N & insert
            , I first
            , I last
            ) noexcept
            : collector<C, I, R, O, N>{ retain, remove, insert }
            , rule(rule)
            , curr(first)
            , last(last)
            , head()
        {
            if (first == last)
                stat = execution_status::stopped;
            else
                head = std::regex_iterator<I>(first, last, rule.pattern);
        }

    private:
        using iterator = I;

        core<mode::regex, C> const &  rule;
        iterator                      curr;
        iterator                      last;
        std::regex_iterator<iterator> head;
        std::regex_iterator<iterator> tail{ std::regex_iterator<iterator>() };
        std::basic_string<C>          buff{};
        execution_status              stat{ execution_status::startup };
    };

    
}}
