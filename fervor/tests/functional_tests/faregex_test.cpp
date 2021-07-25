#include <catch2/catch_test_macros.hpp>
#include <far/faregex.hpp>

TEST_CASE("constructable", "[faregex]")
{
    // char type is limited by regex_traits
    // https://en.cppreference.com/w/cpp/regex/regex_traits
    SECTION("char")
    {
        auto f = far::faregex("", "");
        f("");
    }
    SECTION("wchar_t")
    {
        auto w = std::wstring();
        auto f = far::faregex(w, w);
        f(w);
    }
}
