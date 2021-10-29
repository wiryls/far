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

    using status = stat::control;
}}

namespace far { namespace task { namespace import
{
    template<typename R>
    concept input = std::ranges::input_range<R> && requires(element<R> item)
    {
        // source item
        { std::get<0>(item) };
        // file path
        { std::get<1>(item) } -> std::convertible_to<std::filesystem::path const &>;
    };

    template<typename C, typename I>
    concept callback = input<I> && requires(C on, element<I> item)
    {
        // report failure
        { on.error (std::get<0>(item)) };
        // output a valid path and increase counter if return true
        { on.output(std::get<1>(item)) } -> std::same_as<bool>;
    };

    template<executor E, input I, callback<I> C>
    auto import(E & executor, I && input, C callback, bool recursive) -> status
    {
        auto job = stat::sensor();
        auto fun = [&, job, input = std::forward<I>(input), on = std::move(callback)]
        {
            using std::filesystem::recursive_directory_iterator;
            using std::filesystem::directory_options;
            auto constexpr option = directory_options::skip_permission_denied;

            for (auto [item, path] : input)
            {
                if /**/ (job.expired()) [[unlikely]]
                {
                    break;
                }
                else if (!std::filesystem::exists(path)) [[unlikely]]
                {
                    on.error(item);
                }
                else if (recursive)
                {
                    for (auto & entry : recursive_directory_iterator(path, option))
                        if (job.expired()) [[unlikely]]
                            break;
                        else if (on.output(entry.path()))
                            job.add(1);
                }
                else if (on.output(path))
                {
                    job.add(1);
                }
            }
        };

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

    template<typename R, typename C>
    using change = scan::change<C, std::ranges::iterator_t<element<R>>>;

    template<typename C>
    using buffer = std::remove_cvref_t<decltype(std::declval<C>().buffer())>;

    template<typename B, typename R, typename C>
    concept callback = input<R, C> && requires
        ( B on
        , buffer<B> buffer
        , element<R> item
        , change<R, C> const & patch )
    {
        // build a new buffer
        { on.buffer() };
        // append changes to buffer
        { on.append(buffer, patch) } -> std::same_as<void>;
        // output buffer
        { on.output(item, buffer) } -> std::same_as<bool>;
    };

    template
        < executor E
        , scan::mode M
        , scan::char_type U
        , input<U> I
        , callback<I, U> C >
    auto differ
        ( E & executor
        , I & input
        , C callback
        , scanner<M, U> const & rule
        ) -> status
    {
        auto job = stat::sensor(input.size());

        // recursive lambda, see:
        // https://stackoverflow.com/a/40873505
        auto fun = [&executor, &rule, on = std::move(callback)]<std::forward_iterator I>
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
                auto buff = on.buffer();
                for (auto & change : rule.iterate(item))
                    on.append(buff, change);
                on.output(item, buff);
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

    template<executor E, input I, error C>
    auto rename(E & executor, I const & input, C callback) -> stat::control
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
