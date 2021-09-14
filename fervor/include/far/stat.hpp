#pragma once

#include <atomic>
#include <memory>

namespace far { namespace stat
{
    enum struct status
    {
        running = 0,
        halting = 1,
        stopped = 2,
    };

    struct snapshot
    {
        std::size_t count;
        std::size_t total; // maybe 0 if unknown
        status      state; // state of a fact
    };

    struct progress
    {
        std::atomic<std::size_t> count{};
        std::atomic<std::size_t> total{};
        std::atomic<status>      state{ status::running };
    };

    class publisher
    {
    public:

        // Add the counter
        auto add(std::size_t i) const -> void;

        // Set the counter
        auto set(std::size_t i) const -> void;

        // Set the total.
        auto max(std::size_t i) const -> void;

        // Whether it receives a halting message.
        auto drop() const -> bool;

        // All tasks have been all done. No more waiting!
        auto done() const -> void;

        // If not drop then perform add
        //auto drop_or_add() const -> bool;

    private:

        friend class subscriber;
        std::shared_ptr<progress> data = std::make_shared<progress>();
    };

    class subscriber
    {
    public:

        // Peek the current status.
        auto peek() const -> snapshot;

        // Try to clam down.
        //
        // - May be ignored.
        // - Non-block operation.
        auto clam() -> void;

        // Wait until all done.
        auto wait() const -> void;

    public:
        subscriber(publisher && ex);

    private:

        std::shared_ptr<progress> data;
    };
}}
