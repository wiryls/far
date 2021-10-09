#include <catch2/catch_test_macros.hpp>

#include <vector>
#include <string>
#include <ranges>

#include <far/exec.hpp>
#include <far/task.hpp>

TEST_CASE("examples for task import", "[import]")
{
    using far::scan::operation;
    auto exec = far::executor();

    SECTION("example")
    {
        auto vec = std::vector<std::string>{ R"()" };
        auto src = vec | std::views::transform([](auto & item) -> decltype(auto)
        {
            return std::make_pair<std::string &, std::filesystem::path>(item, item);
        });

        struct out
        {
            auto error(std::string & item [[maybe_unused]]) const -> void
            {
                // do nothing.
            }
            auto collect(std::filesystem::path const & path [[maybe_unused]]) const -> bool
            {
                // do nothing.
                return true;
            }
        };
        auto dst = out();

        auto task = far::import(exec, src, dst, true);
        task.wait();
        REQUIRE(task.stat() == far::stat::status::stopped);

        auto && [count, total] = task.peek();
        REQUIRE(count == 0);
        REQUIRE(total == 0);
    }
}
