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
    auto oss = std::ostream_iterator<char>(std::cout);
    auto out = std::string();
    auto x = std::string("0101010101111010");

    auto scan = far::scanner<far::scan_mode::icase, char>("0", "1");
    auto g = scan.generate(x);
    for (;;)
    {
        auto s = g();
        using op = far::scan::operation;
        /**/ if (auto retain = std::get_if<&op::retain>(&s))
        {
            std::format_to(oss, "retain {}\n", std::string_view(retain->begin(), retain->end()));
            out.append(retain->begin(), retain->end());
        }
        else if (auto remove = std::get_if<&op::remove>(&s))
        {
            std::format_to(oss, "remove {}\n", std::string_view(remove->begin(), remove->end()));
        }
        else if (auto insert = std::get_if<&op::insert>(&s))
        {
            std::format_to(oss, "insert {}\n", std::string_view(insert->begin(), insert->end()));
            out.append(insert->begin(), insert->end());
        }
        else if (std::get_if<&op::none>(&s))
            break;
    }

    std::cout << out << std::endl;

    for (auto & s : scan.iterate(x))
    {
        using op = far::scan::operation;
        /**/ if (auto retain = std::get_if<&op::retain>(&s))
        {
            std::format_to(oss, "retain {}\n", std::string_view(retain->begin(), retain->end()));
        }
        else if (auto remove = std::get_if<&op::remove>(&s))
        {
            std::format_to(oss, "remove {}\n", std::string_view(remove->begin(), remove->end()));
        }
        else if (auto insert = std::get_if<&op::insert>(&s))
        {
            std::format_to(oss, "insert {}\n", std::string_view(insert->begin(), insert->end()));
        }
    }

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
