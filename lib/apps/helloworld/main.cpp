#include <functional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <format>

#include <far/exec.hpp>
#include <far/task.hpp>

auto main() -> int
{
    auto p = std::filesystem::path();

    auto exec = far::executor();

    auto i = std::vector<const char *>{R"(C:\Users\)" };
    auto o = []<typename T>(T && x)
    {
        std::cout << x << std::endl;
        return true;
    };

    auto f = far::import(exec, i, o, true);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    f.stop();

    f.wait();
    auto && [count, total] = f.peek();
    std::cout << count << "/" << total << std::endl;

    return 0;
}
