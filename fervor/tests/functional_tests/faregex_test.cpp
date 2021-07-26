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
        f("");
    }
    SECTION("std::wstring")
    {
        static_assert(far::cep::string_like<std::wstring, wchar_t>);
        auto w = std::wstring();
        auto f = far::faregex(w, w);
        f(w);
    }
    SECTION("std::list<wchar_t>")
    {
        auto l = std::list<wchar_t>{};
        auto f = far::faregex(l, l);
        f(l);
    }
}
