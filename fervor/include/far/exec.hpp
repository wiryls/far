#pragma once

#include <concepts>
#include <functional>
#include <memory>
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
    auto operator()() -> void;

public:
    explicit executor(std::size_t size = std::thread::hardware_concurrency());
            ~executor();

private:
    template<bool R /* eleasable */>
    auto loop() -> void;

private:
    bool                     running;
    std::size_t              holding;
    std::deque<task>         tasks;
    std::mutex               mutex;
    std::condition_variable  condition;
    std::vector<std::thread> executors;
};

template<std::convertible_to<::far::executor::task> T>
auto far::executor::operator()(T && todo) -> bool
{
    auto o = true;
    {
        std::scoped_lock _(mutex);
        if ((o = running, o))
        {
            ++holding;
            tasks.push_back(std::forward<T>(todo));
        }
    }

    if (o)
        condition.notify_one();

    return o;
}

template<bool R /* eleasable */>
auto far::executor::loop() -> void
{
    auto loop = true;
    while (loop)
    {
        auto todo = task();
        {
            auto lock = std::unique_lock(mutex);
            condition.wait(lock, [&]
            {
                if constexpr (R)
                    return holding == 0 || !running;
                else
                    return !tasks.empty() || !running;
            });

            while (!tasks.empty() && todo == nullptr)
            {
                todo = std::move(tasks.front());
                tasks.pop_front();
            }
        }

        if (todo != nullptr)
        {
            todo();

            mutex.lock();
            if (--holding == 0)
                condition.notify_all();
        }
        else
        {
            mutex.lock();
        }
        {
            if constexpr (R)
                loop = !tasks.empty() || holding != 0;
            else
                loop = !tasks.empty() || running;
            mutex.unlock();
        }
    }
}
