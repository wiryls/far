#include <catch2/catch_test_macros.hpp>

#include <list>
#include <vector>
#include <far/faregex.hpp>

TEST_CASE("constructor, trait, deduction guide", "[faregex]")
{
    // char type is limited by regex_traits
    // https://en.cppreference.com/w/cpp/regex/regex_traits
    SECTION("char const & [N]")
    {
        static_assert(far::cep::char_type<char>);
        auto f = far::faregex("", "");
        REQUIRE(f == true);
        auto g = f.generate("");
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (std::get_if<*type::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::none>(&s))
                break;
        }
    }
    SECTION("std::wstring")
    {
        static_assert(far::cep::matcher<std::wstring, wchar_t>);
        auto w = std::wstring();
        auto f = far::faregex(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (std::get_if<*type::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::none>(&s))
                break;
        }
    }
    SECTION("std::list<wchar_t>")
    {
        auto l = std::list<wchar_t>{};
        auto f = far::faregex(l, l);
        REQUIRE(f == true);
        auto g = f.generate(l);
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (std::get_if<*type::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::none>(&s))
                break;
        }
    }
    SECTION("std::wstring const")
    {
        auto x = std::wstring();
        auto const & w = x;
        auto f = far::faregex(w, w);
        REQUIRE(f == true);
        auto g = f.generate(w);
        for (;;)
        {
            auto s = g();
            using far::change::type;
            if /**/ (std::get_if<*type::insert>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::retain>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::remove>(&s))
                FAIL("should never reach here");
            else if (std::get_if<*type::none>(&s))
                break;
        }
    }
}

TEST_CASE("error regexp", "[faregex]")
{
    auto f = far::faregex(R"(((()))", "");
    REQUIRE(f == false);
}

TEST_CASE("a sample lazy loop", "[faregex]")
{
    using namespace std::string_literals;
    SECTION("char const & [N]")
    {
        auto const & pattern = R"(log)";
        auto const & replace = R"(ln)";
        auto const & example = R"(log(😅) = 💧 log(😄))";

        for (auto const & f : {
            far::faregex(pattern, replace),
            far::faregex(pattern, replace, far::option::ignore_case),
            far::faregex(pattern, replace, far::option::normal_mode),
            far::faregex(pattern, replace, far::option(far::option::normal_mode | far::option::ignore_case)) })
        {
            auto g = f.generate(example);

            using far::change::type;
            using C = std::ranges::range_value_t<decltype("")>;
            {
                auto v = g();
                auto u = std::get_if<*type::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::retain>(&v);
                auto x = "(😅) = 💧 "s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::remove>(&v);
                auto x = "log"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::insert>(&v);
                auto x = "ln"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::retain>(&v);
                auto x = "(😄)"s;
                REQUIRE(u != nullptr);
                REQUIRE(std::equal(x.begin(), x.end(), u->begin(), u->end()));
            }
            {
                auto v = g();
                auto u = std::get_if<*type::none>(&v);
                REQUIRE(u != nullptr);
            }
        }
    }
}

#include <iostream>

TEST_CASE("iterator", "[faregex]")
{
    auto make_matcher = []
        <far::cep::char_type C>
        (std::vector<std::pair<far::change::type, std::basic_string<C>>> && vec)
    {
        auto head = vec.begin();
        auto tail = vec.end();
        return [head=head, tail=tail, vec=std::move(vec)]
            <far::cep::matched_iter<C> I>
            (far::change::variant<C, I> const & var) mutable
        {
            using defer = std::shared_ptr<void>;
            using far::change::type;
            defer _(nullptr, [&](...){ ++head; });

            if (head == tail && var.index() != *type::none)
                return false;

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

    using namespace std::string_literals;
    SECTION("empty")
    {
        auto l = std::list<wchar_t>();
        for (auto const & f : {
            far::faregex(l, l),
            far::faregex(l, l, far::option::ignore_case),
            far::faregex(l, l, far::option::normal_mode),
            far::faregex(l, l, far::option(far::option::normal_mode | far::option::ignore_case)) })
        {
            auto [head, tail] = f.iterate(l);
            REQUIRE(head == tail);
        }
    }
    SECTION("all the same")
    {
        auto l = std::list<wchar_t>{'0'};
        for (auto const& f : {
            far::faregex(l, l),
            far::faregex(l, l, far::option::ignore_case),
            far::faregex(l, l, far::option::normal_mode),
            far::faregex(l, l, far::option(far::option::normal_mode | far::option::ignore_case)) })
        {
            auto [head, tail] = f.iterate(l);
            REQUIRE(head != tail);

            auto retain = std::get_if<*far::change::type::retain>(&*head);
            REQUIRE(retain != nullptr);
            REQUIRE(retain->begin() == l.begin());
            REQUIRE(retain->end() == l.end());
            REQUIRE(++head == tail);
        }
    }
    SECTION("unicode with regex")
    {
        auto const & pattern = R"(不.+?。|，恶.+?？|悲.+?不)";
        auto const & replace = R"()";
        auto const & example =
            "为众人抱薪者，不可使其冻毙于风雪。"
            "为苍生治水者，不可使其沉溺于湖海。"
            "为当下奋斗者，不可使其淹没于尘埃。"
            "为未来奠基者，不可使其从宽而入窄。"
            "悲兮叹兮，若善者不得善终，恶者可更恶乎？";

        auto f = far::faregex(pattern, replace);
        auto i = f.iterate(example);

        using far::change::type;
        auto m = make_matcher(std::vector<std::pair<type, std::string>>
        {
            { type::retain, "为众人抱薪者，"},
            { type::remove, "不可使其冻毙于风雪。" },
            { type::retain, "为苍生治水者，" },
            { type::remove, "不可使其沉溺于湖海。" },
            { type::retain, "为当下奋斗者，" },
            { type::remove, "不可使其淹没于尘埃。" },
            { type::retain, "为未来奠基者，" },
            { type::remove, "不可使其从宽而入窄。悲兮叹兮，若善者不" },
            { type::retain, "得善终" },
            { type::remove, "，恶者可更恶乎？" },
        });

        for (auto & c : i)
            REQUIRE(m(c));
    }
}
