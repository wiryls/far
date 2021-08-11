#include <far/fever.hpp>

far::fever::fever(std::size_t size)
    : running(size > 0)
    , tasks()
    , mutex()
    , condition()
    , executors()
{
    executors.reserve(size);
    while (size-- > 0)
        executors.emplace_back(&fever::loop, this);
}

far::fever::~fever()
{
    {
        std::scoped_lock _(mutex);
        running = false;
    }

    condition.notify_all();
    for (auto& executor : executors)
        executor.join();
}

auto far::fever::loop() -> void
{
    auto loop = true;
    auto grab = false;
    while (loop || grab)
    {
        auto todo = task();
        {
            auto lock = std::unique_lock(mutex);
            condition.wait(lock, [&] { return !tasks.empty() || !running; });

            loop = running;
            grab = !tasks.empty();

            if (grab)
            {
                todo = std::move(tasks.front());
                tasks.pop_front();
            }
        }

        if (grab)
            todo();
    }
}

auto far::fettle::peek() -> progress
{
    return progress
    {
        done .load(std::memory_order_relaxed),
        total.load(std::memory_order_relaxed),
    };
}

auto far::fettle::clam() -> void
{
    // TODO:
}
