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
        std::size_t total; // maybe 0 if unknown.
    };

    class sensor
    {
    public:

        // add the counter.
        auto add(std::size_t i) const -> void;

        // set the counter.
        auto set(std::size_t i) const -> void;

        // set the total.
        auto max(std::size_t i) const -> void;

        // is it cancelled.
        auto expired() const -> bool;

    public:
       ~sensor();
        sensor(std::size_t max = 0);
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

        // peek current progress.
        auto peek() const -> current;

        // status of execution;
        auto stat() const -> status;

        // try to stop.
        //
        // - may be ignored.
        // - non-block operation.
        auto stop() -> void;

        // wait until tasks done or stopped.
        auto wait() const -> void;

    public:
        control(sensor && pub);

    private:
        std::shared_ptr<progress> data;
    };
}}
