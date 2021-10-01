#pragma once

#include <concepts>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <ranges>
#include <thread>
#include <filesystem>
#include <regex>
#include <string>
#include <string_view>

#include <far/stat.hpp>
#include <far/scan.hpp>

namespace far { namespace task
{
    //// shared

    template<typename E>
    concept executor = requires(E execute, void (*fp)(), std::function<void()> fn)
    {
        { execute(fp) };
        { execute(fn) };
    };

    namespace detail
    {
        template<typename F>
        struct passing
        {
            // pass by reference
            using type = std::add_lvalue_reference_t<F>;
        };

        template<std::copyable F>
        requires (sizeof(F) <= sizeof(void *))
        struct passing<F>
        {
            // pass by value
            using type = F;
        };
    }

    template<typename F>
    using passing = detail::passing<F>::type;
}}

namespace far { namespace task { namespace import
{
    //// import

    template<typename I>
    concept input
        = std::ranges::input_range<I>
       && std::convertible_to<std::ranges::range_value_t<I>, std::filesystem::path>
        ;

    template<typename O>
    concept output = requires (O output, std::filesystem::path path)
    {
        { output(path) } -> std::same_as<bool>;
    };

    template
        < executor E
        , input    I
        , output   O >
    auto import
        ( E & executor
        , O & output
        , I const & input
        , bool recursive) -> stat::control
    {
        auto rec = stat::sensor();
        auto fun = std::function<void()>();

        if (recursive)
        {
            fun = [&, rec = rec]
            {
                using std::filesystem::recursive_directory_iterator;
                for (auto & item : input)
                    for (auto & entry : recursive_directory_iterator(item))
                        if (rec.expired()) [[unlikely]]
                            break;
                        else if (output(entry.path()))
                            rec.add(1);
            };
        }
        else
        {
            fun = [&, rec = rec]
            {
                auto constexpr cast = []<typename T>(T && x)
                {
                    using std::filesystem::path;
                    return static_cast<path>(std::forward<T>(x));
                };

                for (auto item : input | std::views::transform(cast))
                    if (rec.expired()) [[unlikely]]
                        break;
                    else if (output(std::move(item)))
                        rec.add(1);
            };
        }

        executor(std::move(fun));
        return rec;
    }
}}}

namespace far { namespace task { namespace differ
{
    //// differ

    template<typename I>
    using item = std::ranges::range_value_t<I>;

    template<typename C, typename R>
    concept input
        = std::ranges::random_access_range<R>
       && scan::bidirectional_sequence<item<R>, C>
        ;

    template<scan::unit C>
    using buffer = std::basic_string<C>;

    template<typename F /* func */, typename C /* char */, typename I /* input item */>
    concept output = requires
        ( F output
        , buffer<C> const & buffer
        , item<I> & item )
    {
        { output(item, buffer) } -> std::same_as<void>;
    };

    template<typename F /* func */, typename C /* char */, typename I /* input */>
    concept collect = requires
        ( F collect
        , buffer<C> & buffer
        , scan::change<C, std::ranges::iterator_t<item<I>>> const & item )
    {
        { collect(buffer, item) } -> std::same_as<void>;
    };

    //template
    //    < executor E
    //    , scan::mode M
    //    , scan::unit C
    //    //, input  <C> I
    //    //, collect<C, I> F
    //    //, output <C, I> O
    //    , typename I
    //    , typename F
    //    , typename O
    //    > struct differ
    //{
    //    auto operator()() const -> void
    //    {
    //        auto constexpr static batch = 1;
    //        if (auto rest = std::views::drop(vec, batch); !rest.empty())
    //        {
    //            //auto todo = [rest, fun] { fun(rest, fun); };
    //            //executor(todo);
    //            //fun(rest, fun);
    //        }

    //        auto list = std::views::take(vec, batch);
    //        for (auto & value : list)
    //        {
    //            using scan::operation;
    //            auto buff = buffer<C>();
    //            for (auto& change : rule.iterate(value))
    //                collect(buff, change);
    //            output(value, buff);
    //        }
    //    }

    //    differ(E & executor, scanner<M, C> const & rule, O output, F collect, I & input)
    //        : executor(executor)
    //        , rule(rule)
    //        , output(output)
    //        , collect(collect)
    //        , input(input)
    //    {}

    //    E &                   executor;
    //    scanner<M, C> const & rule;
    //    passing<O>            output;
    //    passing<F>            collect;
    //    I &                   input;
    //};

    template
        < executor E
        , scan::mode M
        , scan::unit C
        //, input  <C> I
        //, collect<C, I> F
        //, output <C, I> O
        , typename I
        , typename F
        , typename O
        >
    auto differ
        ( E & executor
        , scanner<M, C> const & rule
        , O output
        , F collect
        , I & input
        ) -> stat::control
    {
        auto rec = stat::sensor();
        auto run = [&]
        {
            rec.max(input.size());

            // use std::views::drop will cause infinite recursion.
            // so i use iterators as a fallback.
            auto fun = [&executor, &rule, &output, &collect]<std::forward_iterator I>
                (I head, I tail, stat::sensor const & r, auto & f) -> void
            {
                if (r.expired())
                    return;

                auto constexpr static batch = std::iter_difference_t<I>(1);
                auto step = std::min(batch, std::ranges::distance(head, tail));
                auto next = std::ranges::next(head, step);

                if (next != tail)
                    executor([head = next, tail, r, &f] { f(head, tail, r, f); });

                for (auto & value : std::ranges::subrange(head, next))
                {
                    using scan::operation;
                    auto buff = buffer<C>();
                    for (auto & change : rule.iterate(value))
                    {
                        collect(buff, change);
                        r.add(1);
                    }

                    output(value, buff);
                }
            };
            using std::ranges::begin, std::ranges::end;
            return [&, rec] { fun(begin(input), end(input), rec, fun); };
        };

        executor(run());
        return rec;
    }

    //struct rename;
    //struct sort;
    //struct clear;
    //struct remove;
}}}

namespace far
{
    // export to task

    using task::import::import;
    using task::differ::differ;
}
