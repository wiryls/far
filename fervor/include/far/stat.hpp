#pragma once

#include <memory>

namespace far { namespace stat
{
    struct progress;

    enum struct status
    {
        pending = 0,
        running = 1,
        halting = 2,
        stopped = 3,
    };

    struct current
    {
        std::size_t count;
        std::size_t total; // maybe 0 if unknown
    };

    class sensor
    {
    public:

        // Add the counter
        auto add(std::size_t i) const -> void;

        // Set the counter
        auto set(std::size_t i) const -> void;

        // Set the total.
        auto max(std::size_t i) const -> void;

        // Whether it should be cancelled.
        auto expired() const -> bool;

    public:
       ~sensor();
        sensor();
        sensor            (sensor const & rhs);
        sensor            (sensor      && rhs) noexcept;
        sensor & operator=(sensor const & rhs);
        sensor & operator=(sensor      && rhs) noexcept;

    private:
        friend class control;
        std::shared_ptr<progress> data;
    };

    class control
    {
    public:

        // Peek the current progress.
        auto peek() const -> current;

        // status of execution;
        auto stat() const -> status;

        // Try to stop.
        //
        // - May be ignored.
        // - Non-block operation.
        auto stop() -> void;

        // Wait until tasks done or stopped.
        auto wait() const -> void;

    public:
        control(sensor && pub);

    private:
        std::shared_ptr<progress> data;
    };
}}
