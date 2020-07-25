/*
    __ _____ _____ _____
 __|  |   __|     |   | |  JSON for Modern C++ (test suite)
|  |  |__   |  |  | | | |  version ?.?.?
|_____|_____|_____|_|___|  https://github.com/nlohmann/json

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2013-2020 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "doctest_compatibility.h"

#include <nlohmann/json.hpp>
using nlohmann::json;


namespace udt
{
struct animal
{
    int m_age;
    std::string m_name;

    animal() : m_age(), m_name() {}
    animal(const int& a, const std::string& n) : m_age(a), m_name(n) {}
};

template <typename BasicJsonType>
void valid_conversion_from_json(const BasicJsonType& j, animal& a)
{
    a.m_age = j["age"].template get<int>();
    a.m_name = j["name"].template get<std::string>();
}

template <typename BasicJsonType>
void should_not_be_called(const BasicJsonType&, animal& a)
{
    a.m_age = 42;
    a.m_name = "it should never be called";
}

static bool operator==(const animal& lhs, const animal& rhs)
{
    return std::tie(lhs.m_name, lhs.m_age) == std::tie(rhs.m_name, rhs.m_age);
}
}  // namespace udt

namespace udt
{
struct cat : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this);
    }
};

struct dog : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, dog& d)
    {
        valid_conversion_from_json(j, d);
    }
};

struct parrot : animal
{
    using animal::animal;
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, parrot& p)
{
    valid_conversion_from_json(j, p);
}
}  // namespace udt

TEST_CASE("customization points for default constructible types" * doctest::test_suite("udt"))
{
    SECTION("conversion from json via member function")
    {
        udt::cat tigger;
        json json = R"({"name":"tigger", "age":5})"_json;

        json.get_to(tigger);

        CHECK(tigger == udt::cat{{5}, {"tigger"}});
    }
    SECTION("conversion from json via static function")
    {
        udt::dog bella;
        json json = R"({"name":"bella", "age":9})"_json;

        json.get_to(bella);

        CHECK(bella == udt::dog{{9}, {"bella"}});
    }
    SECTION("conversion from json via free function")
    {
        udt::parrot greenie;
        json json = R"({"name":"greenie", "age":3})"_json;

        json.get_to(greenie);

        CHECK(greenie == udt::parrot{{3}, {"greenie"}});
    }
}

namespace udt
{
struct rabbit : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, rabbit& r)
    {
        should_not_be_called(j, r);
    }
};

struct horse : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, horse& h)
    {
        valid_conversion_from_json(j, h);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, horse& h)
{
    should_not_be_called(j, h);
}

struct hamster : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, hamster& h)
{
    should_not_be_called(j, h);
}

struct ferret : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, ferret& f)
    {
        should_not_be_called(j, f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, ferret& f)
{
    should_not_be_called(j, f);
}
}  // namespace udt

TEST_CASE("priorities of user-defined customizations for default-constructible types" * doctest::test_suite("udt"))
{
    SECTION("have a member function and static method")
    {
        json json = R"({"name":"thumper", "age":1})"_json;

        auto thumper = json.get<udt::rabbit>();

        CHECK(thumper == udt::rabbit{1, "thumper"});
    }
    SECTION("have a static method and free function")
    {
        json json = R"({"name":"lilly", "age":23})"_json;

        auto lilly = json.get<udt::horse>();

        CHECK(lilly == udt::horse{23, "lilly"});
    }
    SECTION("have a member and free function")
    {
        json json = R"({"name":"ace", "age":2})"_json;

        auto ace = json.get<udt::hamster>();

        CHECK(ace == udt::hamster{2, "ace"});
    }
    SECTION("have a member function, static method and free function")
    {
        json json = R"({"name":"kirra", "age":4})"_json;

        auto kirra = json.get<udt::ferret>();

        CHECK(kirra == udt::ferret{4, "kirra"});
    }
}  // namespace udt

namespace udt
{
template <typename ValueType, typename BasicJsonType>
ValueType valid_conversion_from_json(const BasicJsonType& j)
{
    ValueType p
    {
        j["age"].template get<int>(),
        j["name"].template get<std::string>(),
    };
    return p;
}

template <typename ValueType, typename BasicJsonType>
void should_not_be_called(const BasicJsonType&)
{
    ValueType p
    {
        42,
        "it should never be called",
    };
    return p;
}

struct mice : animal
{
    using animal::animal;

    mice() = delete;

    template <typename BasicJsonType>
    static mice from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<mice>(j);
    }
};

struct fish : animal
{
    using animal::animal;

    fish() = delete;
};

template <typename BasicJsonType>
static fish from_json(const BasicJsonType& j, nlohmann::adl_tag<fish>)
{
    return valid_conversion_from_json<fish>(j);
}

struct turtle : animal
{
    using animal::animal;

    turtle() = delete;

    template <typename BasicJsonType>
    static turtle from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<turtle>(j);
    }
};

template <typename BasicJsonType>
static turtle from_json(const BasicJsonType& j, nlohmann::adl_tag<turtle>)
{
    return should_not_be_called<turtle>(j);
}
}  // namespace udt

TEST_CASE("customization points for non-default constructible types" * doctest::test_suite("udt"))
{
    SECTION("conversion from json via member function")
    {
        json json = R"({"name":"basil", "age":1})"_json;

        auto basil = json.get<udt::mice>();

        CHECK(basil == udt::mice{1, "basil"});
    }
    SECTION("conversion from json via free function")
    {
        json json = R"({"name":"jack", "age":22})"_json;

        static_assert (nlohmann::detail::from_json_fn::has_tagged_adl_accesible_from_json<udt::fish, nlohmann::json>::value, "");
        static_assert (!nlohmann::detail::from_json_fn::has_non_default_static_from_json<udt::fish, nlohmann::json>::value, "");
        auto jack = json.get<udt::fish>();

        CHECK(jack == udt::fish{22, "jack"});
    }
    SECTION("priority between member and free function")
    {
        json json = R"({"name":"shelby", "age":50})"_json;

        auto shelby = json.get<udt::turtle>();

        CHECK(shelby == udt::turtle{50, "shelby"});
    }
}

namespace udt
{
struct gerbil : animal
{
    using animal::animal;
};

struct chipmunk : animal
{
    using animal::animal;
};

struct potbelly_pig : animal
{
    using animal::animal;
};
struct chinchilla : animal
{
    using animal::animal;
};
struct iguana : animal
{
    using animal::animal;
};

struct spider : animal
{
    using animal::animal;
};

struct hedgehog : animal
{
    using animal::animal;
};
struct rodent : animal
{
    using animal::animal;
};
struct guinea_pig : animal
{
    using animal::animal;
};
}  // namespace udt
