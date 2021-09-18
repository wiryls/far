#pragma once

#include <concepts>
#include <type_traits>
#include <algorithm>
#include <ranges>
#include <thread>
#include <filesystem>
#include <regex>
#include <string>
#include <string_view>

#include <far/stat.hpp>

namespace far { namespace task
{
    //// shared

    template<typename E>
    concept executor = requires(E execute)
    {
        { execute((void (*)())nullptr) };
        { execute([] {}) };
        { execute([=] () mutable {}) };
        { execute([&] () mutable {}) };
    };
}}

namespace far { namespace task
{
    //// import

    template<typename I>
    concept import_input
        = std::ranges::input_range<I>
       && std::convertible_to<std::ranges::range_value_t<I>, std::filesystem::path>
        ;

    template<typename O>
    concept import_output = requires (O output, std::filesystem::path path)
    {
        { output(path) } -> std::same_as<bool>;
    };

    template
        < executor E
        , import_input I
        , import_output O>
    auto import
        ( E & executor
        , O & output
        , I const & input
        , bool recursive) -> stat::control
    {
        auto rec = stat::sensor();

        if (recursive)
        {
            executor([&, rec = rec]
            {
                using std::filesystem::recursive_directory_iterator;
                for (auto & item : input)
                    for (auto & entry : recursive_directory_iterator(item))
                        if (rec.expired()) [[unlikely]]
                            break;
                        else if (output(entry.path()))
                            rec.add(1);
            });
        }
        else
        {
            executor([&, rec = rec]
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
            });
        }

        return rec;
    }
}}

namespace far { namespace task
{
    //// differ



    template
        < executor E
        , typename L // item list
        , typename P // pattern type
        , typename T // template type
        , typename G // member getter
        , typename R // retain-formatter
        , typename D // remove-formatter
        , typename I // insert-formatter
        , typename U // result updater
        , typename C = std::ranges::range_value_t<P> // char type
        >
    requires
           std::same_as<typename std::regex_traits<C>::char_type, C>
        && std::same_as<typename std:: char_traits<C>::char_type, C>
        && std::ranges::random_access_range<L>
        && std::same_as<C, std::ranges::range_value_t<std::ranges::range_value_t<L>>>
        && std::ranges::random_access_range<P>
        && std::same_as<C, std::ranges::range_value_t<P>>
        && std::ranges::random_access_range<T>
        && std::same_as<C, std::ranges::range_value_t<T>>
        && std::invocable<G, std::ranges::range_reference_t<L>>
        && std::invocable<U, std::ranges::range_reference_t<L>, std::basic_string<C>>
        && requires
            ( std::ranges::range_reference_t<L> item
            , G getter
            , U updater
            , std::iter_difference_t<std::ranges::range_value_t<L>> const n
            , std::basic_string<C> & accumulator )
        {
            {  getter(item)              } -> std::ranges::random_access_range;
            {  getter(item)[n]           } -> std::convertible_to<C>;
            { updater(item, accumulator) } -> std::same_as<void>;
        }
        && requires
            ( R retain
            , D remove
            , I insert
            , std::ranges::iterator_t<std::ranges::range_value_t<L>> iterator
            , std::basic_string_view<C> const & buffer
            , std::basic_string     <C> & accumulator )
        {
            { retain(iterator, iterator, accumulator) } -> std::same_as<void>;
            { remove(iterator, iterator, accumulator) } -> std::same_as<void>;
            { insert(buffer,             accumulator) } -> std::same_as<void>;
        }
    auto differ
        ( E & executor
        , L & items
        , P const & pattern
        , T const & replace
        , bool normal_mode
        , bool ignore_case
        , G source_get
        , R format_retain
        , D format_remove
        , I format_insert
        , U result_update ) -> stat::control
    {
        if (normal_mode)
        {
            if (ignore_case)
            {
                ;
            }
            else
            {
                ;
            }
        }
        else // regexp_mode
        {
            ;
        }
    }

    //struct rename;
    //struct sort;
    //struct clear;
    //struct remove;
}}
