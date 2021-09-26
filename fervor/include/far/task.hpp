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

    template<typename C, typename R>
    concept input
        = std::ranges::random_access_range<R>
       && scan::bidirectional_sequence<std::ranges::range_value_t<R>, C>
        ;

    template<typename I, typename B, typename O>
    concept output = requires(O output, B buffer, std::ranges::range_value_t<I> item)
    {
        { output(item, buffer) } -> std::same_as<void>;
    };

    template<scan::unit C>
    using buffer = std::basic_string<C>;

    template
        < executor E
        , scan::mode M
        , scan::unit C
        , input<C> I
        , output<I, buffer<C>> O
        >
    auto differ
        ( E & executor
        , O output
        , I & input
        , scanner<M, C> const & rule
        ) -> stat::control
    {
        auto rec = stat::sensor();
        auto fun = [&executor, &output, &input, &rule]
            <std::ranges::random_access_range V>(auto && fun, stat::sensor rec, V vec)
        {
            auto constexpr static batch = 100;
            if (auto rest = std::views::drop{ vec, batch }; !rest.empty())
                executor(/* TODO: fun(rec, std::move(rest)) */ );

            auto todo = std::views::take{ vec, batch };
            for (auto & item : todo)
            {
                using scan::operation;
                auto buff = buffer<C>();
                for (auto & c : rule.iterate(item))
                {
                    // TODO:
                }
            }
        };

        executor([]
        {
            ;
        });

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
}
