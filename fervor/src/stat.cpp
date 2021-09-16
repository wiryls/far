#include <atomic>
#include <far/stat.hpp>

//// progress

struct far::stat::progress
{
private: // padding

    /* cache line size */
    auto inline constexpr static pad = std::size_t(64);

    /* helper to sum all */
    template<std::size_t ... N>
    auto inline constexpr static sum = (N + ...);

    /* dummy type */
    template<std::size_t ... N>
    requires (pad > (N + ...))
    using padding = std::uint8_t[pad - sum<N ...>];

public: // members
    std::atomic<status>      state{ status::pending };
    std::atomic<std::size_t> refer{};
    padding<sizeof state, sizeof refer> __0;

    std::atomic<std::size_t> count{};
    std::atomic<std::size_t> total{};
    padding<sizeof count, sizeof total> __1;

public: // status
    auto inline status() const noexcept -> status;
    auto inline expire() const noexcept -> bool;
    auto inline holdon() const noexcept -> void;
    auto inline cancel()       noexcept -> void;

public: // refer && status
    auto inline increase() noexcept -> void;
    auto inline decrease() noexcept -> void;

public: // count && total
    auto inline now() const noexcept -> current;
    auto inline add(std::size_t delta) noexcept -> void;
    auto inline set(std::size_t delta) noexcept -> void;
    auto inline max(std::size_t delta) noexcept -> void;
};

auto far::stat::progress::status() const noexcept -> far::stat::status
{
    return state.load(std::memory_order_relaxed);
}

auto far::stat::progress::expire() const noexcept -> bool
{
    return state.load(std::memory_order_relaxed) != status::running;
}

auto far::stat::progress::holdon() const noexcept -> void
{
    state.wait(status::pending, std::memory_order_acquire);
    state.wait(status::running, std::memory_order_acquire);
    state.wait(status::halting, std::memory_order_acquire);
}

auto far::stat::progress::cancel() noexcept -> void
{
    auto current = status::running;
    state.compare_exchange_strong
        ( current
        , status::halting
        , std::memory_order_release
        , std::memory_order_relaxed );
}

auto far::stat::progress::increase() noexcept -> void
{
    // +: std::memory_order_relaxed
    // -: std::memory_order_acq_rel
    // https://stackoverflow.com/a/41425204
    if (refer.fetch_add(1, std::memory_order_relaxed) == 0)
    {
        auto current = status::pending;
        state.compare_exchange_strong
            ( current
            , status::running
            , std::memory_order_release
            , std::memory_order_relaxed );
    }
}

auto far::stat::progress::decrease() noexcept -> void
{
    // similar to sp: https://stackoverflow.com/a/891027
    if (refer.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        state.store(status::stopped, std::memory_order_release);
        state.notify_all();
    }
}

auto far::stat::progress::now() const noexcept -> current
{
    return current
    {
        count.load(std::memory_order_relaxed),
        total.load(std::memory_order_relaxed),
    };
}

auto far::stat::progress::add(std::size_t delta) noexcept -> void
{
    count.fetch_add(delta, std::memory_order_relaxed);
}

auto far::stat::progress::set(std::size_t delta) noexcept -> void
{
    count.store(delta, std::memory_order_relaxed);
}

auto far::stat::progress::max(std::size_t delta) noexcept -> void
{
    total.store(delta, std::memory_order_relaxed);
}

//// control

auto far::stat::control::peek() const -> current
{
    return data->now();
}

auto far::stat::control::stat() const -> status
{
    return data->status();
}

auto far::stat::control::stop() -> void
{
    data->cancel();
}

auto far::stat::control::wait() const -> void
{
    data->holdon();
}

far::stat::control::control(sensor && ex)
    : data()
{
    if (ex.data)
    {
        ex.data->decrease();
        data = std::move(ex.data);
        ex.data = nullptr;
    }
}

//// sensor

auto far::stat::sensor::add(std::size_t i) const -> void
{
    data->add(i);
}

auto far::stat::sensor::set(std::size_t i) const -> void
{
    data->set(i);
}

auto far::stat::sensor::max(std::size_t i) const -> void
{
    data->max(i);
}

auto far::stat::sensor::expired() const -> bool
{
    return data->expire();
}

far::stat::sensor::~sensor()
{
    if (data)
        data->decrease();
}

far::stat::sensor::sensor()
    : data(std::make_shared<progress>())
{
    data->increase();
}

far::stat::sensor::sensor(sensor && rhs) noexcept
    : data(std::move(rhs.data))
{
    rhs.data = nullptr;
}

far::stat::sensor::sensor(sensor const & rhs)
    : data(rhs.data)
{
    if (data)
        data->increase();
}

far::stat::sensor & far::stat::sensor::operator=(sensor && rhs) noexcept
{
    if (data)
        data->decrease();
    data = std::move(rhs.data);
    rhs.data = nullptr;
    return *this;
}

far::stat::sensor & far::stat::sensor::operator=(sensor const & rhs)
{
    if (data)
        data->decrease();
    data = rhs.data;
    if (data)
        data->increase();
    return *this;
}
