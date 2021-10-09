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
    auto exe = far::executor();
    auto vec = std::vector<std::string>{ R"(C:\Users\)" };
    auto src = vec | std::views::transform([](auto & item) -> decltype(auto)
    {
        return std::make_pair<std::string &, std::filesystem::path>(item, item);
    });

    struct out
    {
        auto error(std::string & item) const -> void
        {
            std::cout << std::format("failed to read {}\n", item);
        }
        auto collect(std::filesystem::path const & path) const -> bool
        {
            std::cout << path << "\n";
            return true;
        }
    };
    auto dst = out();

    auto f = far::import(exe, src, dst, true);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    f.stop();

    f.wait();
    auto && [count, total] = f.peek();
    std::cout << count << "/" << total << std::endl;

    return 0;
}
