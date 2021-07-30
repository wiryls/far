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
            using C = std::ranges::range_value_t<decltype("")>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype("")>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype("")>>;

            auto s = g();
            if /**/ (std::get_if<I>(&s))
                FAIL("should never reach here");
            else if (std::get_if<R>(&s))
                FAIL("should never reach here");
            else if (std::get_if<D>(&s))
                FAIL("should never reach here");
            else if (std::get_if<std::monostate>(&s))
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
            using C = std::ranges::range_value_t<decltype(w)>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype(w)>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype(w)>>;

            auto s = g();
            if /**/ (std::get_if<I>(&s))
                FAIL("should never reach here");
            else if (std::get_if<R>(&s))
                FAIL("should never reach here");
            else if (std::get_if<D>(&s))
                FAIL("should never reach here");
            else if (std::get_if<std::monostate>(&s))
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
            using C = std::ranges::range_value_t<decltype(l)>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype(l)>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype(l)>>;

            auto s = g();
            if /**/ (std::get_if<I>(&s))
                FAIL("should never reach here");
            else if (std::get_if<R>(&s))
                FAIL("should never reach here");
            else if (std::get_if<D>(&s))
                FAIL("should never reach here");
            else if (std::get_if<std::monostate>(&s))
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
            using C = std::ranges::range_value_t<decltype(w)>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype(w)>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype(w)>>;

            auto s = g();
            if /**/ (std::get_if<I>(&s))
                FAIL("should never reach here");
            else if (std::get_if<R>(&s))
                FAIL("should never reach here");
            else if (std::get_if<D>(&s))
                FAIL("should never reach here");
            else if (std::get_if<std::monostate>(&s))
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
        auto f = far::faregex(R"(log)", "ln");
        auto g = f("log(😅) = 💧 log(😄)");

        using C = std::ranges::range_value_t<decltype("")>;
        using I = far::insert<C>;
        using R = far::retain<C, std::ranges::iterator_t<decltype("")>>;
        using D = far::remove<C, std::ranges::iterator_t<decltype("")>>;

        {
            auto v = g();
            auto u = std::get_if<D>(&v);
            auto x = "log"s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->first, u->second));
        }
        {
            auto v = g();
            auto u = std::get_if<I>(&v);
            auto x = "ln"s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
        }
        {
            auto v = g();
            auto u = std::get_if<R>(&v);
            auto x = "(😅) = 💧 "s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->first, u->second));
        }
        {
            auto v = g();
            auto u = std::get_if<D>(&v);
            auto x = "log"s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->first, u->second));
        }
        {
            auto v = g();
            auto u = std::get_if<I>(&v);
            auto x = "ln"s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
        }
        {
            auto v = g();
            auto u = std::get_if<R>(&v);
            auto x = "(😄)"s;
            REQUIRE(u != nullptr);
            REQUIRE(std::equal(x.begin(), x.end(), u->first, u->second));
        }
    }
}
