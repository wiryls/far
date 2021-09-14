#pragma once

#include <concepts>
#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace far
{
    class executor;
}

class far::executor
{
public:
    using task = std::function<void()>;

    template<std::convertible_to<::far::executor::task> T>
    auto operator()(T && todo) -> bool;

public:
    explicit executor(std::size_t size = std::thread::hardware_concurrency());
            ~executor();

private:
    auto push(task && todo) -> bool;
    auto loop() -> void;

private:
    bool                     running;
    std::deque<task>         tasks;
    std::mutex               mutex;
    std::condition_variable  condition;
    std::vector<std::thread> consumers;
};

template<std::convertible_to<::far::executor::task> T>
auto far::executor::operator()(T && todo) -> bool
{
    return push(std::move(static_cast<task>(std::forward<T>(todo))));
}
