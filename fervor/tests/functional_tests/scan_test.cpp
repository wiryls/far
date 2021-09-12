#include <catch2/catch_test_macros.hpp>

#include <list>
#include <vector>
#include <sstream>
#include <far/scan.hpp>

using namespace std::string_literals;
namespace aux = far::scan::aux;
using op = far::scan::operation;
using far::make_scanner, far::scan_mode;

TEST_CASE("helpers", "[scan]")
{
    SECTION("change::type operator&")
    {
        REQUIRE(&op::none == 0);
        auto i = op::none;
        REQUIRE(&i != 0);
    }
}

TEST_CASE("constructor, trait, deduction guide", "[scan]")
{
    // char type is limited by regex_traits
    // https://en.cppreference.com/w/cpp/regex/regex_traits
    SECTION("char const & [N]")
    {
        static_assert(::far::scan::unit<char>);
        auto f = make_scanner<scan_mode::regex>("", "");
        REQUIRE(f == true);
        auto g = f.generate("");
        for (;;)
        {
            auto s = g();
            if /**/ (std::get_if<&op::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::none>(&s))
                break;
        }
    }
    SECTION("std::wstring")
    {
        static_assert(::far::scan::bidirectional_sequence<std::wstring, wchar_t>);
        auto w = std::wstring();
        auto f = make_scanner<scan_mode::regex>(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (;;)
        {
            auto s = g();
            if /**/ (std::get_if<&op::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::none>(&s))
                break;
        }
    }
    SECTION("std::list<wchar_t>")
    {
        auto l = std::list<wchar_t>{};
        auto f = make_scanner<scan_mode::regex>(l, l);
        REQUIRE(f == true);
        auto g = f.generate(l);
        for (;;)
        {
            auto s = g();
            if /**/ (std::get_if<&op::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::none>(&s))
                break;
        }
    }
    SECTION("std::wstring const")
    {
        auto x = std::wstring();
        auto const & w = x;
        auto f = make_scanner<scan_mode::regex>(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (;;)
        {
            auto s = g();
            if /**/ (std::get_if<&op::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<&op::none>(&s))
                break;
        }
    }
}

TEST_CASE("error regexp", "[scan]")
{
    auto f = make_scanner<scan_mode::regex>(R"(((()))", "");
    REQUIRE(f == false);
}

TEST_CASE("a sample lazy loop", "[scan]")
{
    using namespace std::string_literals;
    SECTION("char const & [N]")
    {
        auto const & pattern = R"(log)";
        auto const & replace = R"(ln)";
        auto const & example = R"(log(😅) = 💧 log(😄))";

        auto cases = std::make_tuple
            ( make_scanner<scan_mode::basic>(pattern, replace)
            , make_scanner<scan_mode::icase>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace, true) );

        auto tests = [&](auto const & f)
        {
            auto g = f.generate(example);
            {
                auto v = g();
                auto u = std::get_if<&op::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::retain>(&v);
                auto x = "(😅) = 💧 "s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::retain>(&v);
                auto x = "(😄)"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<&op::none>(&v);
                REQUIRE(u != nullptr);
            }
        };

        std::apply([=](auto && ... f) { (tests(f), ...); }, cases);
    }
}

TEST_CASE("iterator", "[scan]")
{
    auto constexpr make_matcher = []
        <::far::scan::unit C>
        (std::vector<std::pair<op, std::basic_string<C>>> && vec)
    {
        auto head = vec.begin();
        auto tail = vec.end();
        return [head=head, tail=tail, vec=std::move(vec)]
            <::far::scan::bidirectional_iterative<C> I>
            (::far::scan::change<C, I> const & var) mutable
        {
            using defer = std::shared_ptr<void>;

            if (head == tail && var.index() != &op::none)
                return false;

            defer _(nullptr, [&](...){ ++head; });
            if (static_cast<std::size_t>(head->first) != var.index())
                return false;

            auto & lhs = head->second;
            return std::visit([&](auto && rhs)
            {
                using that = std::remove_cvref_t<decltype(rhs)>;
                if constexpr (std::is_same_v<that, std::monostate>)
                    return false;
                else
                    return std::equal
                    ( std::ranges::begin(lhs), std::ranges::end(lhs)
                    , std::ranges::begin(rhs), std::ranges::end(rhs) );
            }, var);
        };
    };

    auto constexpr cases_maker = [](auto const & pattern, auto const & replace)
    {
        using value_type = std::ranges::range_value_t<decltype(pattern)>;
        return std::make_tuple
            ( make_scanner<scan_mode::basic>(pattern, replace)
            , make_scanner<scan_mode::icase>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace, true) );
    };

    SECTION("empty")
    {
        auto empty = std::list<wchar_t>();
        auto cases = cases_maker(empty, empty);
        auto tests = [&](auto const & f)
        {
            auto && [head, tail] = f.iterate(empty);
            REQUIRE(head == tail);
        };

        std::apply([=](auto && ... f) { (tests(f), ...); }, cases);
    }
    SECTION("all the same")
    {
        auto dummy = std::list<wchar_t>{'0'};
        auto cases = cases_maker(dummy, dummy);
        auto tests = [&](auto const & f)
        {
            auto [head, tail] = f.iterate(dummy);
            REQUIRE(head != tail);

            auto retain = std::get_if<&op::retain>(&*head);
            REQUIRE(retain != nullptr);
            REQUIRE(retain->begin() == dummy.begin());
            REQUIRE(retain->  end() == dummy.  end());
            REQUIRE(++head == tail);
        };

        std::apply([=](auto && ... f) { (tests(f), ...); }, cases);
    }
    SECTION("unicode with regex")
    {
        // '?' makes ".+" non-greedy
        // https://stackoverflow.com/a/2824314

        auto const & pattern = R"(不.+?。|，恶.+?？|悲.+?不)";
        auto const & replace = R"()";
        auto const & example =
            "为众人抱薪者，不可使其冻毙于风雪。"
            "为苍生治水者，不可使其沉溺于湖海。"
            "为当下奋斗者，不可使其淹没于尘埃。"
            "为未来奠基者，不可使其从宽而入窄。"
            "悲兮叹兮，若善者不得善终，恶者可更恶乎？";
        auto const & output =
            "为众人抱薪者，为苍生治水者，为当下奋斗者，为未来奠基者，得善终";

        auto f = make_scanner<scan_mode::regex>(pattern, replace);
        auto i = f.iterate(example);

        auto m = make_matcher(std::vector<std::pair<op, std::string>>
        {
            { op::retain, "为众人抱薪者，"},
            { op::remove, "不可使其冻毙于风雪。" },
            { op::retain, "为苍生治水者，" },
            { op::remove, "不可使其沉溺于湖海。" },
            { op::retain, "为当下奋斗者，" },
            { op::remove, "不可使其淹没于尘埃。" },
            { op::retain, "为未来奠基者，" },
            { op::remove, "不可使其从宽而入窄。悲兮叹兮，若善者不" },
            { op::retain, "得善终" },
            { op::remove, "，恶者可更恶乎？" },
        });

        auto oss = std::ostringstream();
        for (auto & c : i)
        {
            REQUIRE(m(c));
            if /**/ (auto insert = std::get_if<&op::insert>(&c))
                oss << std::string_view(*insert);
            else if (auto retain = std::get_if<&op::retain>(&c))
                oss << std::string_view(*retain);
        }
        REQUIRE(output == oss.str());
    }
}
