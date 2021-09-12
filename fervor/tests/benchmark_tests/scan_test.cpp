#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <sstream>
#include <regex>
#include <far/scan.hpp>
#include "tool.hpp"

using namespace std::string_literals;
namespace aux = far::scan::aux;
using op = far::scan::operation;
using far::make_scanner, far::scan_mode;

TEST_CASE(R"(regex "[a-z]" to "_")", "[scan]")
{
    auto input = std::string(512, '\0');
    fill_random(input,
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789");

    auto const & pattern = R"([a-z])";
    auto const & replace = R"(_)";

    BENCHMARK("warm")
    {
        return input.size();
    };

    BENCHMARK(R"(std::regex)")
    {
        auto const r = std::regex(pattern);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);
        std::regex_replace(i, head, tail, r, replace);
        return o.str();
    };

    BENCHMARK(R"(far::scan::generator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::regex>(pattern, replace);
        auto g = f.generate(input);
        for (;;)
        {
            auto s = g();
            if /**/ (auto insert = std::get_if<&op::insert>(&s))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&op::retain>(&s))
                o << std::string_view(*retain);
            else if (std::get_if<&op::remove>(&s))
                continue;
            else if (std::get_if<&op::none>(&s))
                break;
        }
        return o.str();
    };

    BENCHMARK(R"(far::scan::iterator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::regex>(pattern, replace);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto insert = std::get_if<&op::insert>(&c))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&op::retain>(&c))
                o << std::string_view(*retain);
        }
        return o.str();
    };

    BENCHMARK(R"(far::scan::apply)")
    {
        auto const rule = far::scan::rule<far::scan::mode::regex, char>
            (pattern, replace, false);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);

        far::scan::apply(
            rule,
            [&](auto left, auto right) { o << std::string_view(left, right); },
            [ ](auto, auto) {},
            [&](auto left, auto right) { o << std::string_view(left, right); },
            head,
            tail);

        return o.str();
    };
}

TEST_CASE(R"(plain text "0011" to "1100")", "[scan]")
{
    auto input = std::string(1024, '\0');
    fill_random(input, "01");

    auto const & pattern = "0011";
    auto const & replace = "1100";

    BENCHMARK(R"(warm)")
    {
        return input.size();
    };

    BENCHMARK(R"(std::string.find)")
    {
        auto head = std::size_t{};
        auto tail = input.size();
        auto o = std::ostringstream();

        auto s = input.begin();
        while (head < tail)
        {
            auto i = input.find(pattern, head);
            if ( i == std::string::npos )
                break;

            o << std::string_view(std::next(s, head), std::next(s, i)) << replace;
            head = i + 1;
        }

        if (head != tail)
            o << std::string_view(std::next(s, head), std::next(s, tail));

        return o.str();
    };

    BENCHMARK(R"(far::scan::generator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::basic>(pattern, replace);
        auto g = f.generate(input);
        for (;;)
        {
            auto s = g();
            if /**/ (auto insert = std::get_if<&op::insert>(&s))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&op::retain>(&s))
                o << std::string_view(*retain);
            else if (std::get_if<&op::remove>(&s))
                continue;
            else if (std::get_if<&op::none>(&s))
                break;
        }
        return o.str();
    };


    BENCHMARK(R"(far::scan::iterator)")
    {
        auto o = std::ostringstream();
        auto f = make_scanner<scan_mode::basic>(pattern, replace);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto insert = std::get_if<&op::insert>(&c))
                o << std::string_view(*insert);
            else if (auto retain = std::get_if<&op::retain>(&c))
                o << std::string_view(*retain);
        }
        return o.str();
    };

    BENCHMARK(R"(far::scan::apply)")
    {
        auto const rule = far::scan::rule<far::scan::mode::basic, char>(pattern, replace);

        auto head = input.begin();
        auto tail = input.end();

        auto o = std::ostringstream();
        auto i = std::ostreambuf_iterator(o);

        far::scan::apply(
            rule,
            [&](auto left, auto right) { o << std::string_view(left, right); },
            [ ](auto, auto) {},
            [&](auto left, auto right) { o << std::string_view(left, right); },
            head,
            tail);

        return o.str();
    };
}
