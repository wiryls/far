#include <far/exec.hpp>

auto far::executor::operator()() -> void
{
    loop<true>();
}

far::executor::executor(std::size_t size)
    : running(size > 0)
    , holding()
    , tasks()
    , mutex()
    , condition()
    , executors()
{
    executors.reserve(size);
    while (size --> 0)
        executors.emplace_back(&executor::loop<false>, this);
}

far::executor::~executor()
{
    {
        std::scoped_lock _(mutex);
        running = false;
    }

    condition.notify_all();
    for (auto & executor : executors)
        executor.join();
}
