#include <catch2/catch_test_macros.hpp>

#include <vector>
#include <string>
#include <ranges>

#include <far/exec.hpp>
#include <far/task.hpp>

TEST_CASE("examples for task differ", "[differ]")
{
    using far::scan::operation;
    auto exec = far::executor();

    SECTION("example")
    {
        using change = far::scan::change<char, std::string::iterator>;

        auto input = std::vector<std::string>
        {
            "000", "001", "010", "011", "100", "101", "110", "111"
        };

        auto output = [](std::string & item, std::string & buffer)
        {
            item = std::move(buffer);
        };

        auto buffer = []
        {
            return std::string();
        };

        auto collect = [](std::string & buffer, change const & modify)
        {
            if /**/ (auto retain = std::get_if<&operation::retain>(&modify); retain)
                buffer.append(retain->begin(), retain->end());
            else if (auto insert = std::get_if<&operation::insert>(&modify); insert)
                buffer.append(insert->begin(), insert->end());
        };

        auto rule = far::make_scanner<far::scan::mode::basic>("1", "0");
        auto task = far::differ(exec, input, output, buffer, collect, rule);

        task.wait();
        REQUIRE(task.stat() == far::stat::status::stopped);

        auto && [count, total] = task.peek();
        REQUIRE(count == input.size());
        REQUIRE(total == input.size());

        for (auto & item : input)
            REQUIRE(item == "000");
    }
}
