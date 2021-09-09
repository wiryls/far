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
    auto i = far::scan::immutable_field<far::scan::mode::icase, char>("", "");
    auto m = far::scan::  mutable_field<far::scan::mode::icase, char, char const *>();
    auto f = std::function<void (char const *, char const *)>();
    auto g = std::function<void (std::string::const_iterator, std::string::const_iterator)>();

    far::scan::next(i, m, f, f, g);

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
