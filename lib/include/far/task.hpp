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
        auto job = stat::sensor();
        auto fun = std::function<void()>();

        if (recursive)
        {
            fun = [&, job]
            {
                using std::filesystem::recursive_directory_iterator;
                for (auto & item : input)
                    for (auto & entry : recursive_directory_iterator(item))
                        if (job.expired()) [[unlikely]]
                            break;
                        else if (output(entry.path()))
                            job.add(1);
            };
        }
        else
        {
            fun = [&, job]
            {
                auto constexpr cast = []<typename T>(T && x)
                {
                    using std::filesystem::path;
                    return static_cast<path>(std::forward<T>(x));
                };
                for (auto item : input | std::views::transform(cast))
                    if (job.expired()) [[unlikely]]
                        break;
                    else if (output(std::move(item)))
                        job.add(1);
            };
        }

        executor(std::move(fun));
        return job;
    }
}}}

namespace far { namespace task { namespace differ
{
    //// differ

    template<typename I>
    using item = std::ranges::range_value_t<I>;

    template<typename R, typename C>
    concept input
        = std::ranges::random_access_range<R>
       && scan::bidirectional_sequence<item<R>, C>
        ;

    template<scan::unit C>
    using buffer = std::basic_string<C>;

    template<typename F /* func */, typename C /* char */, typename I /* input */>
    concept output = requires
        ( F output
        , item  <I> & item
        , buffer<C> & buffer )
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

    template
        < executor E
        , scan::mode M
        , scan::unit C
        , input  <C> I
        , collect<C, I> F
        , output <C, I> O
        >
    auto differ
        ( E & executor
        , scanner<M, C> const & scan
        , O output
        , F collect
        , I & input
        ) -> stat::control
    {
        auto job = stat::sensor(input.size());

        // recursive lambda, see:
        // https://stackoverflow.com/a/40873505
        auto fun = [&executor, &scan, output, collect]
            <std::forward_iterator I>
            // use std::views::drop will cause infinite recursion during type deduction,
            // so i use a pair of iterators as a fallback.
            (I head, I tail, stat::sensor const & job, auto & fun) -> void
        {
            if (job.expired())
                return;

            auto constexpr static batch = std::iter_difference_t<I>(1);
            auto step = std::min(batch, std::ranges::distance(head, tail));
            auto next = std::ranges::next(head, step);

            if (next != tail)
                executor([=] { fun(next, tail, job, fun); });

            for (auto & value : std::ranges::subrange(head, next))
            {
                auto buff = buffer<C>();
                for (auto & change : scan.iterate(value))
                    collect(buff, change);
                output(value, buff);
                job.add(1);
            }
        };
        auto run = [=, head = std::ranges::begin(input), tail = std::ranges::end(input)]
        {
            fun(head, tail, job, fun);
        };

        executor(run);
        return job;
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
