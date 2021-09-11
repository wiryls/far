#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <far/fever.hpp>
#include <far/fun.hpp>
#include <far/scanner.hpp>

auto main() -> int
{
    auto oss = std::ostream_iterator<char>(std::cout);
    auto out = std::string();
    auto retain = [&](auto first, auto last)
    {
        std::format_to(oss, "retain {}\n", std::string_view(first, last));
        out.append(first, last);
    };
    auto remove = [&](auto first, auto last)
    {
        std::format_to(oss, "remove {}\n", std::string_view(first, last));
    };
    auto insert = [&](std::string::const_iterator first, std::string::const_iterator last)
    {
        std::format_to(oss, "insert {}\n", std::string_view(first, last));
        out.append(first, last);
    };

    auto i = far::scan::core<far::scan::mode::icase, char>("0", "1");
    auto s = std::string("0101010101111010");
    auto m = far::scan::generator(i, retain, remove, insert, std::begin(s), std::end(s));

    while (m());
    std::cout << out << std::endl;

    //auto m = machine();
    //m.import(cancel, status, finish_callback);

    //auto i = std::vector<const char*>{R"(C:\Users\)" };
    //auto o = []<typename T>(T && x)
    //{
    //    std::cout << x << std::endl;
    //    return true;
    //};

    //auto r = far::fever();
    //auto f = far::fun::import(r, i, true, o);

    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //f.clam();

    //f.wait();
    //auto && [count, _2, _3] = f.peek();
    //std::cout << count << std::endl;
    return 0;
}
