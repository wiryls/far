#pragma once

#include <concepts>
#include <type_traits>
#include <algorithm>
#include <ranges>
#include <thread>
#include <filesystem>
#include <regex>
#include <string>
#include <string_view>

namespace far { namespace task
{
    enum struct fettle
    {
        running = 0,
        halting = 1,
        stopped = 2,
    };

    struct flap
    {
        std::size_t count;
        std::size_t total; // maybe 0 if unknown
        fettle      state; // state of a fact
    };

    struct fact
    {
        class publisher;
        class subscriber;

        std::atomic<std::size_t> count{};
        std::atomic<std::size_t> total{};
        std::atomic<fettle>      state{ fettle::running };
    };

    class fact::subscriber
    {
    public:

        // Peek the current status.
        auto peek() const -> flap;

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

        std::shared_ptr<fact> data;
    };

    class fact::publisher
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
        std::shared_ptr<fact> data = std::make_shared<fact>();
    };
}}

namespace far { namespace task
{
    template<typename R>
    concept task_executor = std::invocable<R, std::is_function<void()>>;

    template
        < task_executor E
        , std::ranges::input_range I
        , std::invocable<std::filesystem::path> O
        >
    requires std::convertible_to<std::ranges::range_value_t<I>, std::filesystem::path>
          && std::same_as<std::invoke_result_t<O, std::filesystem::path>, bool>
    auto import
        ( E & executor
        , I const & input
        , bool recursive
        , O & output ) -> fact::subscriber
    {
        auto pub = fact::publisher();

        executor([&, pub=pub]
        {
            if (recursive)
            {
                using std::filesystem::recursive_directory_iterator;
                for (auto & item : input)
                    for (auto & entry : recursive_directory_iterator(item))
                        if (pub.drop()) [[unlikely]]
                            break;
                        else if (output(entry.path()))
                            pub.add(1);
            }
            else
            {
                using std::filesystem::path;
                auto cast = []<typename T>(T && x) { return static_cast<path>(std::forward<T>(x)); };
                for (auto item : input | std::views::transform(cast))
                    if (pub.drop()) [[unlikely]]
                        break;
                    else if (output(std::move(item)))
                        pub.add(1);
            }

            pub.done();
        });

        return pub;
    }

    namespace detail
    {

    }

    template
        < typename E // executor type
        , typename L // item list
        , typename P // pattern type
        , typename T // template type
        , typename G // member getter
        , typename R // retain-formatter
        , typename D // remove-formatter
        , typename I // insert-formatter
        , typename U // result updater
        , typename C = std::ranges::range_value_t<P> // char type
        >
    requires
           std::same_as<typename std::regex_traits<C>::char_type, C>
        && std::same_as<typename std:: char_traits<C>::char_type, C>
        && task_executor<E>
        && std::ranges::random_access_range<L>
        && std::same_as<C, std::ranges::range_value_t<std::ranges::range_value_t<L>>>
        && std::ranges::random_access_range<P>
        && std::same_as<C, std::ranges::range_value_t<P>>
        && std::ranges::random_access_range<T>
        && std::same_as<C, std::ranges::range_value_t<T>>
        && std::invocable<G, std::ranges::range_reference_t<L>>
        && std::invocable<U, std::ranges::range_reference_t<L>, std::basic_string<C>>
        && requires
            ( std::ranges::range_reference_t<L> item
            , G getter
            , U updater
            , std::iter_difference_t<std::ranges::range_value_t<L>> const n
            , std::basic_string<C> & accumulator )
        {
            {  getter(item)              } -> std::ranges::random_access_range;
            {  getter(item)[n]           } -> std::convertible_to<C>;
            { updater(item, accumulator) } -> std::same_as<void>;
        }
        && requires
            ( R retain
            , D remove
            , I insert
            , std::ranges::iterator_t<std::ranges::range_value_t<L>> iterator
            , std::basic_string_view<C> const & buffer
            , std::basic_string     <C> & accumulator )
        {
            { retain(iterator, iterator, accumulator) } -> std::same_as<void>;
            { remove(iterator, iterator, accumulator) } -> std::same_as<void>;
            { insert(buffer,             accumulator) } -> std::same_as<void>;
        }
    auto differ
        ( E & executor
        , L & items
        , P const & pattern
        , T const & replace
        , bool normal_mode
        , bool ignore_case
        , G source_get
        , R format_retain
        , D format_remove
        , I format_insert
        , U result_update ) -> fact::subscriber
    {
        if (normal_mode)
        {
            if (ignore_case)
            {
                ;
            }
            else
            {
                ;
            }
        }
        else // regexp_mode
        {
            ;
        }
    }

    //struct rename;
    //struct sort;
    //struct clear;
    //struct remove;
}}