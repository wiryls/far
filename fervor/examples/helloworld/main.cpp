#include <functional>
#include <far/faregex.hpp>

auto main() -> int
{
    auto s = std::string("123");
    auto g = far::faregex<char>::generator
        < far::faregex<char>::mode::normal
        , std::string::iterator
        > (nullptr, std::begin(s), std::end(s));

    return 0;
}
