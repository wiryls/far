#include <catch2/catch_test_macros.hpp>

#include <list>
#include <far/faregex.hpp>

TEST_CASE("constructor, trait, deduction guide", "[faregex]")
{
    // char type is limited by regex_traits
    // https://en.cppreference.com/w/cpp/regex/regex_traits
    SECTION("char const & [N]")
    {
        static_assert(far::cep::char_type<char>);
        auto f = far::faregex("", "");
        REQUIRE(f == true);
        auto g = f("");
        while (true)
        {
            auto s = g();
            if /**/ (std::get_if<far::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::none>(&s))
                break;
        }
    }
    SECTION("std::wstring")
    {
        static_assert(far::cep::matcher<std::wstring, wchar_t>);
        auto w = std::wstring();
        auto f = far::faregex(w, w);
        REQUIRE(f == true);
        auto g = f(w);
        while (true)
        {
            auto s = g();
            if /**/ (std::get_if<far::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::none>(&s))
                break;
        }
    }
    SECTION("std::list<wchar_t>")
    {
        auto l = std::list<wchar_t>{};
        auto f = far::faregex(l, l);
        REQUIRE(f == true);
        auto g = f(l);
        while (true)
        {
            auto s = g();
            if /**/ (std::get_if<far::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::none>(&s))
                break;
        }
    }
    SECTION("std::wstring const")
    {
        auto x = std::wstring();
        auto const & w = x;
        auto f = far::faregex(w, w);
        REQUIRE(f == true);
        auto g = f(w);
        while (true)
        {
            auto s = g();
            if /**/ (std::get_if<far::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<far::none>(&s))
                break;
        }
    }
}

TEST_CASE("error regexp", "[faregex]")
{
    auto f = far::faregex(R"(((()))", "");
    REQUIRE(f == false);
}

TEST_CASE("a sample lazy loop", "[faregex]")
{
    using namespace std::string_literals;

    SECTION("char const & [N]")
    {
        auto const & pattern = R"(log)";
        auto const & replace = R"(ln)";
        auto const & example = R"(log(😅) = 💧 log(😄))";

        for (auto const & f : {
            far::faregex(pattern, replace),
            far::faregex(pattern, replace, far::option::ignore_case),
            far::faregex(pattern, replace, far::option::normal_mode),
            far::faregex(pattern, replace, far::option(far::option::normal_mode | far::option::ignore_case)) })
        {
            auto g = f(example);

            using C = std::ranges::range_value_t<decltype("")>;
            {
                auto v = g();
                auto u = std::get_if<far::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::retain>(&v);
                auto x = "(😅) = 💧 "s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::retain>(&v);
                auto x = "(😄)"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<far::none>(&v);
                REQUIRE(u != nullptr);
            }
        }
    }
}
