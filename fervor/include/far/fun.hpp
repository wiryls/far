#pragma once

#include <ranges>
#include <filesystem>

namespace far { namespace task
{
    struct import;

    struct differ;

    struct rename;

    struct sort;

    struct clear;

    struct remove;
}}

struct far::task::import
{
    using path_type = std::filesystem::path;

    template<std::ranges::input_range I, std::ranges::output_range O, typename R>
    requires std::same_as<std::ranges::range_value_t<I>, path_type>
    auto operator()(R & runner, I const & input, O & output)
    {
        ;
    }
};
