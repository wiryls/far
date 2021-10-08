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

    template<typename I>
    using element = std::ranges::range_value_t<I>;
}}

namespace far { namespace task { namespace import
{
    template<typename I>
    concept input
        = std::ranges::input_range<I>
       && std::same_as<element<I>, std::filesystem::path>
        ;

    template<typename O>
    concept output = requires (O output, std::filesystem::path const & path)
    {
        { output(path) } -> std::same_as<bool>;
    };

    template
        < executor E
        , input    I
        , output   O >
    auto import
        ( E & executor 
        , I const & input
        , O output
        , bool recursive
        ) -> stat::control
    {
        auto job = stat::sensor();
        auto fun = std::function<void()>();

        if (recursive)
        {
            fun = [&, job, output = std::move(output)]
            {
                using std::filesystem::recursive_directory_iterator;
                using std::filesystem::directory_options;
                auto option = directory_options::skip_permission_denied;
                for (auto & item : input)
                    for (auto & entry : recursive_directory_iterator(item, option))
                        if (job.expired()) [[unlikely]]
                            break;
                        else if (output(entry.path()))
                            job.add(1);
            };
        }
        else
        {
            fun = [&, job, output = std::move(output)]
            {
                for (auto item : input)
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
    template<typename R, typename C>
    concept input
        = std::ranges::random_access_range<R>
       && scan::bidirectional_sequence<element<R>, C>
        ;

    template<typename F>
    concept buffer = std::invocable<F>;

    template<typename F /* func */, typename I /* input */, typename B /* buffer */>
    concept output = requires
        ( F output
        , element<I> & item
        , std::add_lvalue_reference_t<std::invoke_result_t<B>> buffer)
    {
        typename std::invoke_result_t<B>;
        { output(item, buffer) } -> std::same_as<void>;
    };

    template<typename F, typename B, typename C, typename I>
    concept collect = requires
        ( F collect
        , std::add_lvalue_reference_t<std::invoke_result_t<B>> buffer
        , scan::change<C, std::ranges::iterator_t<element<I>>> const & item )
    {
        typename std::invoke_result_t<B>;
        { collect(buffer, item) } -> std::same_as<void>;
    };

    template
        < executor E
        , scan::mode M
        , scan::unit C
        , buffer     B
        , input  <C> I
        , collect<B, C, I> F
        , output <I, B> O >
    auto differ
        ( E & executor
        , I & input
        , O output  /* write result with buffer */
        , B buffer  /* buffer builder */
        , F collect /* merge changes into buffer */
        , scanner<M, C> const & rule
        ) -> stat::control
    {
        auto job = stat::sensor(input.size());

        // recursive lambda, see:
        // https://stackoverflow.com/a/40873505
        auto fun = [&executor, &rule, buffer, collect, output]<std::forward_iterator I>
            // use std::views::drop will cause infinite recursion during type deduction,
            // so i use a pair of iterators as a fallback.
            (I head, I tail, stat::sensor const & job, auto & fun) -> void
        {
            if (job.expired())
                return;

            auto constexpr static batch = std::iter_difference_t<I>(100);
            auto step = std::min(batch, std::ranges::distance(head, tail));
            auto next = std::ranges::next(head, step);

            if (next != tail)
                executor([=] { fun(next, tail, job, fun); });

            for (auto & item : std::ranges::subrange(head, next))
            {
                auto buff = buffer();
                for (auto & change : rule.iterate(item))
                    collect(buff, change);
                output(item, buff);
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
}}}

namespace far { namespace task { namespace rename
{
    template<typename R>
    concept input = std::ranges::random_access_range<R> && requires(element<R> pair)
    {
        { std::get<0>(pair) } -> std::same_as<std::filesystem::path>;
        { std::get<1>(pair) } -> std::same_as<std::filesystem::path>;
    };

    template<typename F>
    concept error = requires (F callback, int code)
    {
        { callback(code) } -> std::same_as<void>;
    };

    template<executor E, input I, error E>
    auto rename(E & executor, I const & input, E callback) -> stat::control
    {
        auto job = stat::sensor();

        auto fun = [callback]<std::forward_iterator I>
            (I head, I tail, stat::sensor const & job, auto & fun) -> void
        {
            if (job.expired())
                return;

            auto constexpr static batch = std::iter_difference_t<I>(100);
            auto step = std::min(batch, std::ranges::distance(head, tail));
            auto next = std::ranges::next(head, step);

            if (next != tail)
                executor([=] { fun(next, tail, job, fun); });

            for (auto & item : std::ranges::subrange(head, next))
            {
                auto & src = std::get<0>(item);
                auto & dst = std::get<1>(item);
                auto code = std::rename(src.c_str(), dst.c_str());
                if (code == 0)
                    job.add(1);
                else
                    callback(code);
            }
        };
        auto run = [=, head = std::ranges::begin(input), tail = std::ranges::end(input)]
        {
            fun(head, tail, job, fun);
        };

        executor(run);
        return job;
    }
}}}

namespace far
{
    // export to task

    using task::import::import;
    using task::differ::differ;
    using task::rename::rename;

    // struct sort;
    // struct clear;
    // struct remove;
}
