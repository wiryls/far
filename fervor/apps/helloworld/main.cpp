#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <format>

#include <far/exec.hpp>
#include <far/task.hpp>
#include <far/scan.hpp>

auto main() -> int
{
    auto i = 0;

    struct
    {
        auto operator()()
        {
            x = 1;
        }

        int & x { i };
    } y;

    //auto i = std::vector<const char*>{R"(C:\Users\)" };
    //auto o = []<typename T>(T && x)
    //{
    //    std::cout << x << std::endl;
    //    return true;
    //};

    //auto r = far::executor();
    //auto f = far::import(r, o, i, true);

    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //f.stop();

    //f.wait();
    //auto && [count, _2] = f.peek();
    //std::cout << count << std::endl;

    return 0;
}
