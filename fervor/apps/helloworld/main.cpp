#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <far/fever.hpp>
#include <far/fun.hpp>

namespace change
{
    enum struct retain;
    enum struct remove;
    enum struct insert;
}

auto f(change::retain) -> void
{
    std::cout << "a" << std::endl;
}

auto f(change::remove) -> void
{
    std::cout << "b" << std::endl;
}



auto main() -> int
{
    f(change::retain{});

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
