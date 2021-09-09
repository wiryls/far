#pragma once

// this file is a new version of faregex.hpp

#include <type_traits>
#include <concepts>
#include <functional>
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

namespace far { namespace scan { namespace detail
{

}}}

namespace far { namespace scan
{
    //// shared

    enum struct mode
    {
        basic,
        icase,
        regex,
    };

    //// immutable

    template<mode M, ::far::scan::scannable C>
    struct immutable_field;

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
    struct basic_or_icase_immutable_field
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
        basic_or_icase_immutable_field(P const & p, R const & r)
            : pattern(::far::scan::aux::begin(p), ::far::scan::aux::end(p))
            , replace(::far::scan::aux::begin(r), ::far::scan::aux::end(r))
            , chooser(bind(pattern))
        {}
        basic_or_icase_immutable_field(basic_or_icase_immutable_field && rhs)
            : pattern(std::move(rhs.pattern))
            , replace(std::move(rhs.replace))
            , chooser(bind(pattern))
        {}
        basic_or_icase_immutable_field(basic_or_icase_immutable_field const & rhs)
            : pattern(rhs.pattern)
            , replace(rhs.replace)
            , chooser(bind(pattern))
        {}
        basic_or_icase_immutable_field & operator=(basic_or_icase_immutable_field && rhs)
        {
            pattern = std::move(rhs.pattern);
            replace = std::move(rhs.replace);
            chooser = bind(pattern);
            return this;
        }
        basic_or_icase_immutable_field & operator=(basic_or_icase_immutable_field const & rhs)
        {
            pattern = rhs.pattern;
            replace = rhs.replace;
            chooser = bind(pattern);
            return this;
        }

    public: // operator
        template<::far::scan::bidirectional_iterative I>
        auto inline operator()(I && head, I && last)
        {
            using tag = std::iterator_traits<I>::iterator_category;
            if constexpr (std::is_base_of_v<std::random_access_iterator_tag, tag>)
                return chooser. first(std::forward<I>(head), std::forward<I>(last));
            else
                return chooser.second(std::forward<I>(head), std::forward<I>(last));
        }

    private: // helper
        auto static bind(string const & that) -> switcher
        {
            // Note: searcher may have a reference to pattern, so I store pattern
            // as a member. Be careful that do not change pattern. Any reallocation
            // may break our searcher, as well as move a (SSOed) small string.
            return std::make_pair
                ( prefered(that.begin(), that.end(), std::hash<C>(), F())
                , fallback(that.begin(), that.end(), F()) );
        }
    };

    template<::far::scan::scannable C>
    struct immutable_field<mode::basic, C> : basic_or_icase_immutable_field<C>
    {
        template<::far::scan::sequence<C> P, ::far::scan::sequence<C> R>
        immutable_field(P const & p, R const & r)
            : basic_or_icase_immutable_field<C>(p, r)
        {}
    };

    template<::far::scan::scannable C>
    struct immutable_field<mode::icase, C>
        : basic_or_icase_immutable_field<C, icase_comparator<C>>
    {
        template<::far::scan::sequence<C> P, ::far::scan::sequence<C> R>
        immutable_field(P const & p, R const & r)
            : basic_or_icase_immutable_field<C, icase_comparator<C>>(p, r)
        {}
    };

    template<::far::scan::scannable C>
    struct immutable_field<mode::regex, C>
    {
        template<::far::scan::forward_sequence<C> P, ::far::scan::sequence<C> R>
        immutable_field(P const & pattern, R const & replace, bool ignore_case)
            : pattern()
            , replace(aux::begin(replace), aux::end(replace))
            , error()
        {
            try
            {
                using option_type = std::regex_constants::syntax_option_type;
                auto constexpr basic = std::regex_constants::ECMAScript;
                auto constexpr icase = std::regex_constants::icase;

                auto option = static_cast<option_type>(basic | (ignore_case ? icase : 0));
                pattern = std::basic_regex<C>(aux::begin(pattern), aux::end(pattern), option);
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

    //// mutable

    template<mode M, ::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    struct mutable_field;

    enum struct status
    {
        stopped,
        startup,
        after_retain,
        after_remove,
        after_insert,
        before_stopped,
    };

    template<::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    struct basic_or_icase_mutable_field
    {
        using iterator = I;

        status                        stat;
        std::pair<iterator, iterator> curr;
        iterator                      head;
        iterator                      tail;
    };

    template<::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    struct mutable_field<mode::basic, C, I> : basic_or_icase_mutable_field<C, I>
    {
        using basic_or_icase_mutable_field<C, I>::basic_or_icase_mutable_field;
    };

    template<::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    struct mutable_field<mode::icase, C, I> : basic_or_icase_mutable_field<C, I>
    {
        using basic_or_icase_mutable_field<C, I>::basic_or_icase_mutable_field;
    };

    template<::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    struct mutable_field<mode::regex, C, I>
    {
        using iterator = I;

        status                        stat;
        iterator                      curr;
        iterator                      last;
        std::regex_iterator<iterator> head;
        std::regex_iterator<iterator> tail;
        std::basic_string<C>          buff;
    };

    //// next

    template
        < mode M
        , ::far::scan::scannable C
        , ::far::scan::bidirectional_iterative<C> I
        , ::far::scan::output<I> R /* retain */
        , ::far::scan::output<I> O /* remove */
        , ::far::scan::output<typename std::basic_string<C>::const_iterator> N /* insert */
        >
    requires (M == mode::basic || M == mode::icase)
    auto next
        ( immutable_field<M, C> const & im
        ,   mutable_field<M, C, I>    & mu
        , R && retain
        , O && remove
        , N && insert
        ) -> bool
    {
        // Note: because coroutines from C++20 are hard to use and have
        // poor performance (in contrast to for-loop), I choose duff's
        // device.
        // https://stackoverflow.com/questions/57726401
        // https://www.reddit.com/r/cpp/comments/gqi0io

        //switch (stat)
        //{
        //case stage::starting:

        //    for (; head != tail; )
        //    {
        //        using input = std::iterator_traits<I>::iterator_category;
        //        if constexpr (std::is_base_of_v<std::random_access_iterator_tag, input>)
        //            next = (ctxt->searcher. first)(head, tail);
        //        else
        //            next = (ctxt->searcher.second)(head, tail);

        //        if (next.first == next.second)
        //            break;

        //        if (head != next.first)
        //        {
        //            stat = stage::after_retain;
        //            return change::retain<C, I>{ head, next.first };
        //        }

        //[[fallthrough]];
        //case stage::after_retain:

        //        {
        //    stat = stage::after_remove;
        //            return change::remove<C, I>(next);
        //        }

        //[[fallthrough]];
        //case stage::after_remove:

        //        if (!ctxt->replace.empty())
        //        {
        //            stat = stage::after_insert;
        //            return change::insert<C>(ctxt->replace);
        //        }

        //[[fallthrough]];
        //case stage::after_insert:

        //        head = next.second;
        //    }


        //[[fallthrough]];
        //case stage::before_stopped:

        //    if (head != tail)
        //    {
        //        stat = stage::stopped;
        //        return change::retain<C, I>{ head, tail };
        //    }

        //[[fallthrough]];
        //case stage::stopped:
        //default:

        //    return std::monostate{};
        //}


        return false;
    }

    template<mode M, ::far::scan::scannable C, ::far::scan::bidirectional_iterative<C> I>
    requires (M == mode::regex)
    auto next() -> bool
    {
        return false;
    }
}}
