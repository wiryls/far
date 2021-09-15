#include <atomic>
#include <far/stat.hpp>

auto far::stat::subscriber::peek() const -> snapshot
{
    return snapshot
    {
        data->count.load(std::memory_order_relaxed),
        data->total.load(std::memory_order_relaxed),
        data->state.load(std::memory_order_relaxed),
    };
}

auto far::stat::subscriber::stop() -> void
{
    auto current = status::running;
    data->state.compare_exchange_strong
        ( current
        , status::halting
        , std::memory_order_release
        , std::memory_order_relaxed );
}

auto far::stat::subscriber::wait() const -> void
{
    data->state.wait(status::running, std::memory_order_acquire);
    data->state.wait(status::halting, std::memory_order_acquire);
}

far::stat::subscriber::subscriber(publisher && ex)
    : data(std::move(ex.data))
{}

auto far::stat::publisher::add(std::size_t i) const -> void
{
    data->count.fetch_add(i, std::memory_order_relaxed);
}

auto far::stat::publisher::set(std::size_t i) const -> void
{
    data->count.store(i, std::memory_order_relaxed);
}

auto far::stat::publisher::max(std::size_t i) const -> void
{
    data->total.store(i, std::memory_order_relaxed);
}

auto far::stat::publisher::drop() const -> bool
{
    return data->state.load(std::memory_order_relaxed) != status::running;
}

auto far::stat::publisher::done() const -> void
{
    data->state.store(status::stopped, std::memory_order_release);
    data->state.notify_all();
}
