#include <atomic>
#include <far/fun.hpp>

auto far::fun::fact::subscriber::peek() const -> flap
{
    return flap
    {
        data->count.load(std::memory_order_relaxed),
        data->total.load(std::memory_order_relaxed),
        data->state.load(std::memory_order_relaxed),
    };
}

auto far::fun::fact::subscriber::clam() -> void
{
    auto current = fettle::running;
    data->state.compare_exchange_strong
        ( current
        , fettle::halting
        , std::memory_order_release
        , std::memory_order_relaxed );
}

auto far::fun::fact::subscriber::wait() const -> void
{
    data->state.wait(fettle::running, std::memory_order_acquire);
    data->state.wait(fettle::halting, std::memory_order_acquire);
}

far::fun::fact::subscriber::subscriber(publisher && ex)
    : data(std::move(ex.data))
{}

auto far::fun::fact::publisher::add(std::size_t i) const -> void
{
    data->count.fetch_add(i, std::memory_order_relaxed);
}

auto far::fun::fact::publisher::set(std::size_t i) const -> void
{
    data->count.store(i, std::memory_order_relaxed);
}

auto far::fun::fact::publisher::max(std::size_t i) const -> void
{
    data->total.store(i, std::memory_order_relaxed);
}

auto far::fun::fact::publisher::drop() const -> bool
{
    return data->state.load(std::memory_order_relaxed) != fettle::running;
}

auto far::fun::fact::publisher::done() const -> void
{
    data->state.store(fettle::stopped, std::memory_order_release);
    data->state.notify_all();
}
