#pragma once

#include <concepts>
#include <functional>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace far
{
    struct fever;
    struct fettle;
}

struct far::fever
{
public:
    using task = std::function<void()>;

    template<std::convertible_to<task> T>
    auto operator()(T && todo) -> void;

public:
    explicit fever(std::size_t size = std::thread::hardware_concurrency());
            ~fever();

private:
    auto loop() -> void;

private:
    bool                     running;
    std::deque<task>         tasks;
    std::mutex               mutex;
    std::condition_variable  condition;
    std::vector<std::thread> executors;
};

template<std::convertible_to<far::fever::task> T>
auto far::fever::operator()(T && todo) -> void
{
    auto o = true;
    {
        std::scoped_lock _(mutex);
        if ((o = running, o))
            tasks.push_back(std::forward<T>(todo));
    }

    if (o)
        condition.notify_one();

    return o;
}

struct far::fettle
{
public:
    struct progress
    {
        std::size_t done;
        std::size_t total;
    };

    // the current progress.
    auto peek() -> progress;

    // Try to clam down.
    // - Blocked until canceled.
    // - Also may be ignored.
    auto clam() -> void;

private:
    enum struct status
    {
        running = 0,
        halting = 1,
        stopped = 2,
    };

    std::atomic<std::size_t> done;
    std::atomic<std::size_t> total;
    std::atomic<status>      stage;
};
