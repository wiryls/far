#include <far/exec.hpp>

far::executor::executor(std::size_t size)
    : running(size > 0)
    , tasks()
    , mutex()
    , condition()
    , consumers()
{
    consumers.reserve(size);
    while (size --> 0)
        consumers.emplace_back(&executor::loop, this);
}

far::executor::~executor()
{
    {
        std::scoped_lock _(mutex);
        running = false;
    }

    condition.notify_all();
    for (auto & executor : consumers)
        executor.join();
}

auto far::executor::push(task && todo) -> bool
{
    auto o = true;
    {
        std::scoped_lock _(mutex);
        if ((o = running, o))
            tasks.push_back(std::move(todo));
    }

    if (o)
        condition.notify_one();

    return o;
}

auto far::executor::loop() -> void
{
    auto keep = true;
    while (keep)
    {
        auto todo = task();
        {
            auto lock = std::unique_lock(mutex);
            condition.wait(lock, [&] { return !tasks.empty() || !running; });

            if (tasks.empty())
            {
                keep = running;
            }
            else
            {
                // maybe replace it with a lock-free queue some day.
                todo = std::move(tasks.front());
                tasks.pop_front();
            }
        }

        if (todo)
            todo();
    }
}
