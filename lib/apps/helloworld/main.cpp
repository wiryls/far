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
    auto exec = far::executor();

    auto input = std::vector<std::string>
    {
        "a00", "0b0", "0bc", "ibb", "", "none", "0000000"
    };

    auto output = [](std::string & item, std::string & buffer)
    {
        item = std::move(buffer);
    };

    auto collect = [](std::string & buffer, far::scan::change<char, std::string::iterator> const & change)
    {
        using far::scan::operation;
        if /**/ (auto retain = std::get_if<&operation::retain>(&change); retain)
            buffer.append(retain->begin(), retain->end());
        else if (auto insert = std::get_if<&operation::insert>(&change); insert)
            buffer.append(insert->begin(), insert->end());
    };

    auto scan = far::make_scanner<far::scan_mode::regex>("b", "1");
    auto f = far::differ(exec, scan, output, collect, input);

    //auto i = std::vector<const char*>{R"(C:\Users\)" };
    //auto o = []<typename T>(T && x)
    //{
    //    std::cout << x << std::endl;
    //    return true;
    //};

    //auto f = far::import(r, o, i, true);

    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //f.stop();

    f.wait();
    auto && [count, total] = f.peek();
    std::cout << count << "/" << total << std::endl;

    for (auto const & out : input)
        std::cout << out << std::endl;

    return 0;
}
