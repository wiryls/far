#include <far/exec.hpp>

auto far::fever::operator()() -> void
{
    loop<true>();
}

far::fever::fever(std::size_t size)
    : running(size > 0)
    , holding()
    , tasks()
    , mutex()
    , condition()
    , executors()
{
    executors.reserve(size);
    while (size --> 0)
        executors.emplace_back(&fever::loop<false>, this);
}

far::fever::~fever()
{
    {
        std::scoped_lock _(mutex);
        running = false;
    }

    condition.notify_all();
    for (auto & executor : executors)
        executor.join();
}
