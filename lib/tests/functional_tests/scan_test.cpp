#include <catch2/catch_test_macros.hpp>

#include <list>
#include <vector>
#include <sstream>
#include <far/scan.hpp>

using far::scan::operation, far::make_scanner, far::scan_mode;

TEST_CASE("helpers", "[scan]")
{
    SECTION("change::type operator&")
    {
        REQUIRE(&operation::retain == 0);
        auto i = operation::retain;
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
        for (auto o = g(); o.has_value(); o = g())
        {
            auto v = &o.value();
            if /**/ (std::get_if<&operation::insert>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::retain>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::remove>(v))
                FAIL("should never reach here");
        }
    }
    SECTION("std::wstring")
    {
        static_assert(::far::scan::bidirectional_sequence<std::wstring, wchar_t>);
        auto w = std::wstring();
        auto f = make_scanner<scan_mode::regex>(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (auto o = g(); o.has_value(); o = g())
        {
            auto v = &o.value();
            if /**/ (std::get_if<&operation::insert>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::retain>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::remove>(v))
                FAIL("should never reach here");
        }
    }
    SECTION("std::list<wchar_t>")
    {
        auto l = std::list<wchar_t>{};
        auto f = make_scanner<scan_mode::regex>(l, l);
        REQUIRE(f == true);
        auto g = f.generate(l);
        for (auto o = g(); o.has_value(); o = g())
        {
            auto v = &o.value();
            if /**/ (std::get_if<&operation::insert>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::retain>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::remove>(v))
                FAIL("should never reach here");
        }
    }
    SECTION("std::wstring const")
    {
        auto x = std::wstring();
        auto const & w = x;
        auto f = make_scanner<scan_mode::regex>(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (auto o = g(); o.has_value(); o = g())
        {
            auto v = &o.value();
            if /**/ (std::get_if<&operation::insert>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::retain>(v))
                FAIL("should never reach here");
            else if (std::get_if<&operation::remove>(v))
                FAIL("should never reach here");
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
            using namespace std::string_literals;
            auto g = f.generate(example);
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::remove>(&v.value());
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::insert>(&v.value());
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::retain>(&v.value());
                auto x = "(😅) = 💧 "s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::remove>(&v.value());
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::insert>(&v.value());
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(v.has_value());

                auto u = std::get_if<&operation::retain>(&v.value());
                auto x = "(😄)"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                REQUIRE(!v.has_value());
            }
        };

        std::apply([=](auto && ... f) { (tests(f), ...); }, cases);
    }
}

TEST_CASE("iterator", "[scan]")
{
    auto constexpr make_matcher = []
        <::far::scan::unit C>
        (std::vector<std::pair<operation, std::basic_string<C>>> && vec)
    {
        auto head = vec.begin();
        auto tail = vec.end();
        return [head=head, tail=tail, vec=std::move(vec)]
            <::far::scan::bidirectional_iterative<C> I>
            (::far::scan::change<C, I> const & var) mutable
        {
            if (head == tail)
                return false;

            using defer = std::shared_ptr<void>;
            defer _(nullptr, [&](...){ ++head; });

            if (static_cast<std::size_t>(head->first) != var.index())
                return false;

            return std::visit([&lhs = head->second](auto && rhs)
            {
                using that = std::remove_cvref_t<decltype(rhs)>;
                return std::equal
                    ( std::ranges::begin(lhs), std::ranges::end(lhs)
                    , std::ranges::begin(rhs), std::ranges::end(rhs) );
            }, var);
        };
    };

    auto constexpr make_cases = [](auto const & pattern, auto const & replace)
    {
        return std::make_tuple
            ( make_scanner<scan_mode::basic>(pattern, replace)
            , make_scanner<scan_mode::icase>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace)
            , make_scanner<scan_mode::regex>(pattern, replace, true) );
    };

    SECTION("empty")
    {
        auto empty = std::list<wchar_t>();
        auto cases = make_cases(empty, empty);
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
        auto cases = make_cases(dummy, dummy);
        auto tests = [&](auto const & f)
        {
            auto [head, tail] = f.iterate(dummy);
            REQUIRE(head != tail);

            auto retain = std::get_if<&operation::retain>(&*head);
            REQUIRE(retain != nullptr);
            REQUIRE(retain->begin() == dummy.begin());
            REQUIRE(retain->  end() == dummy.  end());
            REQUIRE(++head == tail);
        };

        std::apply([=](auto && ... f) { (tests(f), ...); }, cases);
    }
    SECTION("unicode with regex")
    {
        // Note: '?' makes ".+" non-greedy
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

        auto m = make_matcher(std::vector<std::pair<operation, std::string>>
        {
            { operation::retain, "为众人抱薪者，"},
            { operation::remove, "不可使其冻毙于风雪。" },
            { operation::retain, "为苍生治水者，" },
            { operation::remove, "不可使其沉溺于湖海。" },
            { operation::retain, "为当下奋斗者，" },
            { operation::remove, "不可使其淹没于尘埃。" },
            { operation::retain, "为未来奠基者，" },
            { operation::remove, "不可使其从宽而入窄。悲兮叹兮，若善者不" },
            { operation::retain, "得善终" },
            { operation::remove, "，恶者可更恶乎？" },
        });

        auto oss = std::ostringstream();
        for (auto & c : i)
        {
            REQUIRE(m(c));
            if /**/ (auto insert = std::get_if<&operation::insert>(&c))
                oss << std::string_view(*insert);
            else if (auto retain = std::get_if<&operation::retain>(&c))
                oss << std::string_view(*retain);
        }
        REQUIRE(output == oss.str());
    }
}
