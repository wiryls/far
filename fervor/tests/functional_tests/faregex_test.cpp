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
        auto g = f(w);
        while (true)
        {
            using C = std::ranges::range_value_t<decltype(w)>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype(w) const>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype(w) const>>;

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
        auto g = f(l);
        while (true)
        {
            using C = std::ranges::range_value_t<decltype(l)>;
            using I = far::insert<C>;
            using R = far::retain<C, std::ranges::iterator_t<decltype(l) const>>;
            using D = far::remove<C, std::ranges::iterator_t<decltype(l) const>>;

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
