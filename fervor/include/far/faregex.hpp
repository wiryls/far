#pragma once

/// Most code in this file is useless but still is reserved. I leave it
/// here in order to remind me not to over-design.

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

        auto constexpr begin() noexcept -> std::add_lvalue_reference_t<I>
        {
            return this->first;
        }

        auto constexpr end() noexcept -> std::add_lvalue_reference_t<I>
        {
            return this->second;
        }

        auto constexpr begin() const noexcept -> std::add_lvalue_reference_t<std::add_const_t<I>>
        {
            return this->first;
        }

        auto constexpr end() const noexcept -> std::add_lvalue_reference_t<std::add_const_t<I>>
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
        enum struct type : int
        {
            none   = 0,
            retain = 1,
            remove = 2,
            insert = 3,
        };

        template<typename T>
        requires std::same_as<type, std::remove_cvref_t<T>> && (!std::is_reference_v<T>)
        auto inline consteval operator&(T && t) noexcept
        {
            // Let us treat std::underlying_type_t as "address" of our enum.
            return static_cast<std::underlying_type_t<type>>(t);
        }

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
private: // shared data types

    enum struct mode
    {
        basic,
        icase,
        regex,
    };

    template<mode M>
    struct data;

    template<>
    struct data<mode::basic>
    {
        using buffer = std::basic_string<C>;
        using buffer_iterator = typename buffer::const_iterator;
        using prefered = std::boyer_moore_searcher<buffer_iterator>;
        using fallback = std::    default_searcher<buffer_iterator>;
        using group = std::pair<prefered, fallback>;

        template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
        data(P const & p, R const & r)
            : pattern(aux::begin(p), aux::end(p))
            , replace(aux::begin(r), aux::end(r))
            // Note: searcher may have a reference to pattern, so I store pattern as a member.
            // Be careful that do not change pattern. Any reallocation may break our searcher,
            // as well as move a (SSOed) small string.
            , searcher
                ( prefered(pattern.begin(), pattern.end())
                , fallback(pattern.begin(), pattern.end()) )
        {}

        // Disable copy and move in order to avoid breaking searcher.
        // Otherwise we need to rebind searcher to pattern manually.
        data            (data      &&) = delete;
        data & operator=(data      &&) = delete;
        data            (data const &) = delete;
        data & operator=(data const &) = delete;

        buffer pattern;
        buffer replace;
        group  searcher;
    };

    template<>
    struct data<mode::icase>
    {
        struct comparator
        {
            std::locale locale;
            auto operator()(C lhs, C rhs) const noexcept -> bool
            {
                return std::tolower(lhs, locale) == std::tolower(rhs, locale);
            }
        };

        using buffer = std::basic_string<C>;
        using buffer_iterator = typename buffer::const_iterator;
        using prefered = std::boyer_moore_searcher<buffer_iterator, std::hash<C>, comparator>;
        using fallback = std::    default_searcher<buffer_iterator, comparator>;
        using group = std::pair<prefered, fallback>;

        template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
        data(P const & p, R const & r, comparator c = comparator{ std::locale() })
            : pattern(aux::begin(p), aux::end(p))
            , replace(aux::begin(r), aux::end(r))
            , searcher
                ( prefered(pattern.begin(), pattern.end(), std::hash<C>(), c)
                , fallback(pattern.begin(), pattern.end(), c) )
        {}

        data            (data      &&) = delete;
        data & operator=(data      &&) = delete;
        data            (data const &) = delete;
        data & operator=(data const &) = delete;

        buffer pattern;
        buffer replace;
        group  searcher;
    };

    template<>
    struct data<mode::regex>
    {
        std::basic_regex <C> pattern;
        std::basic_string<C> replace;
    };

private: // generator style and generators

    template<mode M, ::far::cep::matched_iter<C> I, typename = void>
    struct generator;

    template<mode M, ::far::cep::matched_iter<C> I>
    struct generator<M, I, std::enable_if_t<M == mode::basic || M == mode::icase>>
    {
    public:
        auto constexpr operator ()() -> change::variant<C, I>
        {
            // Note: because coroutines from C++20 are hard to use and have
            // poor performance (in contrast to for-loop), I choose duff's
            // device.
            // https://www.reddit.com/r/cpp/comments/gqi0io
            // https://stackoverflow.com/questions/57726401

            switch (stat)
            {
            case stage::starting:

                for (; head != tail; )
                {
                    using input = std::iterator_traits<I>::iterator_category;
                    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, input>)
                        next = (ctxt->searcher. first)(head, tail);
                    else
                        next = (ctxt->searcher.second)(head, tail);

                    if (next.first == next.second)
                        break;

                    if (head != next.first)
                    {
                        stat = stage::after_retain;
                        return change::retain<C, I>{ head, next.first };
                    }

            [[fallthrough]];
            case stage::after_retain:

                    {
                stat = stage::after_remove;
                        return change::remove<C, I>(next);
                    }

            [[fallthrough]];
            case stage::after_remove:

                    if (!ctxt->replace.empty())
                    {
                        stat = stage::after_insert;
                        return change::insert<C>(ctxt->replace);
                    }

            [[fallthrough]];
            case stage::after_insert:

                    head = next.second;
                }


            [[fallthrough]];
            case stage::before_stopped:

                if (head != tail)
                {
                    stat = stage::stopped;
                    return change::retain<C, I>{ head, tail };
                }

            [[fallthrough]];
            case stage::stopped:
            default:

                return std::monostate{};
            }
        }

    public:
        using context = data<M> const;
        constexpr generator(context & ref, I first, I last)
            noexcept(noexcept(first = last, first == last, first = std::move(last)))
            : stat
                { first == last
                ? stage::stopped
                : ref.pattern == ref.replace
                ? stage::before_stopped
                : stage::starting }
            , ctxt(&ref)
            , next()
            , head(std::move(first))
            , tail(std::move(last))
        {}

    private:
        enum struct stage
        {
            starting,
            after_retain,
            after_remove,
            after_insert,
            before_stopped,
            stopped
        };

        stage           stat;
        context *       ctxt;
        std::pair<I, I> next;
        I               head;
        I               tail;
    };

    template<::far::cep::matched_iter<C> I>
    struct generator<mode::regex, I>
    {
    public:
        auto constexpr operator ()() -> change::variant<C, I>
        {
            switch (stat)
            {
            case stage::starting:

                for (; head != tail; ++ head)
                {
                    if (head->empty()) [[unlikely]]
                        continue;

                    buff.clear();
                    head->format(std::back_inserter(buff), ctxt->replace);

                    {
                        auto const & match = (*head)[0];
                        if (match.first == match.second ||
                            std::equal
                                ( std::execution::unseq
                                , buff.begin(), buff.end()
                                , match.first, match.second ) )
                            [[unlikely]]
                            continue;

                        if (curr != match.first)
                        {
                            stat = stage::after_retain;
                            return change::retain<C, I>{ curr, match.first };
                        }
                    }

            [[fallthrough]];
            case stage::after_retain:

                    {
                        auto const & match = (*head)[0];
                        stat = stage::after_remove;
                        return change::remove<C, I>{ match.first, match.second };
                    }

            [[fallthrough]];
            case stage::after_remove:

                    if (!buff.empty())
                    {
                        stat = stage::after_insert;
                        return change::insert<C>(buff);
                    }

            [[fallthrough]];
            case stage::after_insert:

                    curr = (*head)[0].second;
                }

                stat = stage::stopped;
                if (curr != done)
                    return change::retain<C, I>{ curr, done };

            [[fallthrough]];
            case stage::stopped:
            default:

                return std::monostate{};
            }
        }

    public:
        using context = data<mode::regex> const;
        constexpr generator(context & ref, I first, I last)
            noexcept(noexcept(first = last, first == last, first = std::move(last)))
            : stat(stage::starting)
            , ctxt(&ref)
            , curr(std::move(first))
            , done(std::move(last))
            , head()
            , tail()
            , buff()
        {
            if (curr == done)
                stat = stage::stopped;
            else
                head = std::regex_iterator<I>(curr, done, ref.pattern);
        }

    private:
        enum struct stage { starting, after_retain, after_remove, after_insert, stopped };

        stage                  stat;
        context *              ctxt;
        I                      curr;
        I                      done;
        std::regex_iterator<I> head;
        std::regex_iterator<I> tail;
        std::basic_string<C>   buff;
    };

public: // iterator style and iterator

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

        template<std::convertible_to<generator_type> T>
        requires!std::is_same_v<std::remove_cvref_t<T>, generator_type>
        explicit constexpr iterator(T && g) noexcept
            : generator(std::forward<T>(g))
            , next(std::monostate{})
            , last(std::monostate{})
            , identify(generator.target<T>())
        {
            ++ *this;
        }

        constexpr iterator() noexcept
            : generator()
            , next(std::monostate{})
            , last(std::monostate{})
            , identify(nullptr)
        {}

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
            using type = change::type;
            if (!static_cast<bool>(generator)) [[unlikely]]
                return *this;

            // generate last
            // generate next
            // if last can merge with next (#3)
            //    merge last with next
            //    generate next
            //    retry #3

            if (last.index() == &type::none) [[unlikely]]
            {
                last = generator();
            }
            else
            {
                last = std::move(next);
            }

            if (last.index() == &type::none)
            {
                generator = nullptr;
                return *this;
            }

            for (next = generator(); last.index() == next.index(); next = generator())
            {
                switch (last.index())
                {
                default:
                case &type::none: [[unlikely]]
                {
                    // should never arrive here, or maybe throw ?
                    last = std::monostate{};
                    next = std::monostate{};
                    generator = nullptr;
                    return *this;
                }
                case &type::retain:
                {
                    auto l = std::get_if<&type::retain>(&last);
                    auto r = std::get_if<&type::retain>(&next);
                    if (l->second != r->first)
                        return *this;

                    l->second = r->second;
                    break;
                }
                case &type::remove:
                {
                    auto l = std::get_if<&type::remove>(&last);
                    auto r = std::get_if<&type::remove>(&next);
                    if (l->second != r->first)
                        return *this;

                    l->second = r->second;
                    break;
                }
                case &type::insert:
                {
                    // not a good idea to expand std::basic_string_view
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
            // Note: it seems not easy to compare std::function
            // https://stackoverflow.com/q/3629835
            // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1667.pdf
            return (lhs.generator == nullptr && rhs.generator == nullptr)
                || (lhs.identify != nullptr && lhs.identify == rhs.identify);
        }

        friend auto constexpr operator!=(iterator const & lhs, iterator const & rhs) -> bool
        {
            return !(lhs == rhs);
        }

        generator_type  generator;
        value_type      next;
        value_type      last;
        void const *    identify;
    };

    template<::far::cep::matched_iter<C> I>
    struct iterator_pair : detail::iter_pair<iterator<I>>
    {
        using detail::iter_pair<iterator<I>>::iter_pair;
    };

public: // API

    template<::far::cep::matched<C> R>
    [[nodiscard]] auto constexpr generate(R & container) const
        -> std::function<change::variant<C, std::ranges::iterator_t<R>>()>
    {
        return generate(aux::begin(container), aux::end(container));
    }

    template<::far::cep::matched_iter<C> I>
    [[nodiscard]] auto constexpr generate(I first, I last) const
        -> std::function<change::variant<C, I> ()>
    {
        if /**/ (auto basic = std::get_if<basic_type>(&maybe))
            return generator<mode::basic, I>(*basic, std::move(first), std::move(last));

        else if (auto icase = std::get_if<icase_type>(&maybe))
            return generator<mode::icase, I>(*icase, std::move(first), std::move(last));

        else if (auto regex = std::get_if<regex_type>(&maybe))
            return generator<mode::regex, I>(*regex, std::move(first), std::move(last));

        else
            return []() -> change::variant<C, I> { return std::monostate{}; };
    }

    template<::far::cep::matched<C> R>
    [[nodiscard]] auto constexpr iterate(R & container) const
        -> iterator_pair<std::ranges::iterator_t<R>>
    {
        return iterate(aux::begin(container), aux::end(container));
    }

    template<::far::cep::matched_iter<C> I>
    [[nodiscard]] auto constexpr iterate(I first, I last) const
        -> iterator_pair<I>
    {
        using type = iterator<I>;
        auto value = iterator_pair<I>{};

        if /**/ (auto basic = std::get_if<basic_type>(&maybe))
            value.first = type(generator<mode::basic, I>(*basic, std::move(first), std::move(last)));

        else if (auto icase = std::get_if<icase_type>(&maybe))
            value.first = type(generator<mode::icase, I>(*icase, std::move(first), std::move(last)));

        else if (auto regex = std::get_if<regex_type>(&maybe))
            value.first = type(generator<mode::regex, I>(*regex, std::move(first), std::move(last)));

        return value;
    }

    [[nodiscard]] constexpr operator bool() const
    {
        return maybe.index() != 0;
    }

public:

    template<::far::cep::matcher<C> P, ::far::cep::matcher<C> R>
    faregex(P const & pattern, R const & replace, option options = option{})
        : maybe()
    {
        auto is_normal_mode = (options & option::normal_mode) != 0;
        auto do_ignore_case = (options & option::ignore_case) != 0;

        if (is_normal_mode)
        {
            if (do_ignore_case)
            {
                maybe.emplace<icase_type>(pattern, replace);
            }
            else
            {
                maybe.emplace<basic_type>(pattern, replace);
            }
        }
        else try
        {
            auto option = static_cast<std::regex_constants::syntax_option_type>
                ( (do_ignore_case ? std::regex_constants::icase : 0)
                | (std::regex_constants::ECMAScript) );
            maybe.emplace<regex_type>
                ( std::basic_regex <C>(aux::begin(pattern), aux::end(pattern), option)
                , std::basic_string<C>(aux::begin(replace), aux::end(replace)) );
        }
        catch ([[maybe_unused]] std::regex_error const & ex)
        {
            // TODO: build an error message
        }
    }

private:

    using empty_type = std::monostate;
    using basic_type = data<mode::basic> const; // Safety: const is necessary.
    using icase_type = data<mode::icase> const;
    using regex_type = data<mode::regex> const;

    std::variant<empty_type, basic_type, icase_type, regex_type> maybe;
};
