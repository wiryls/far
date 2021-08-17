#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <sstream>
#include <regex>
#include <far/faregex.hpp>
#include "tool.hpp"

TEST_CASE("regex", "[faregex]")
{
    using namespace std::string_literals;
    using far::change::type;

    auto const & chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    auto input = std::string(512, '\0');
    fill_random(input.begin(), input.end(), far::aux::begin(chars), far::aux::end(chars));

    auto const & pattern = R"([a-z])";
    auto const & replace = R"(_)";

    BENCHMARK(R"(warm: "[a-z]" to "_")")
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

    BENCHMARK(R"(far::faregex generator)")
    {
        auto o = std::ostringstream();
        auto f = far::faregex(pattern, replace);
        auto g = f.generate(input);
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (auto ins = std::get_if<&type::insert>(&s))
                o << *ins;
            else if (auto ret = std::get_if<&type::retain>(&s))
                o << std::string_view(ret->begin(), ret->end());
            else if (std::get_if<&type::remove>(&s))
                continue;
            else if (std::get_if<&type::none>(&s))
                break;
        }
        return o.str();
    };

    BENCHMARK(R"(far::faregex iterator)")
    {
        auto o = std::ostringstream();
        auto f = far::faregex(pattern, replace);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto ins = std::get_if<&type::insert>(&c))
                o << *ins;
            else if (auto ret = std::get_if<&type::retain>(&c))
                o << std::string_view(ret->begin(), ret->end());
        }
        return o.str();
    };
}

TEST_CASE("plain text", "[faregex]")
{
    using namespace std::string_literals;
    using far::change::type;

    auto const & chars = "01";
    auto input = std::string(1024, '\0');
    fill_random(input.begin(), input.end(), far::aux::begin(chars), far::aux::end(chars));

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

    BENCHMARK(R"(far::faregex generator)")
    {
        auto o = std::ostringstream();
        auto f = far::faregex(pattern, replace, far::normal_mode);
        auto g = f.generate(input);
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (auto ins = std::get_if<&type::insert>(&s))
                o << *ins;
            else if (auto ret = std::get_if<&type::retain>(&s))
                o << std::string_view(ret->begin(), ret->end());
            else if (std::get_if<&type::remove>(&s))
                continue;
            else if (std::get_if<&type::none>(&s))
                break;
        }
        return o.str();
    };


    BENCHMARK(R"(far::faregex iterator)")
    {
        auto o = std::ostringstream();
        auto f = far::faregex(pattern, replace, far::normal_mode);
        for (auto & c : f.iterate(input))
        {
            if /**/ (auto ins = std::get_if<&type::insert>(&c))
                o << *ins;
            else if (auto ret = std::get_if<&type::retain>(&c))
                o << std::string_view(ret->begin(), ret->end());
        }
        return o.str();
    };
}
