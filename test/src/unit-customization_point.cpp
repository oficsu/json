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
enum class parsed_via {
    invalid,
    member_f,
    static_member_f,
    free_adl_f,
    non_default_static_f,
    nob_default_adl_f
};

struct animal
{
    int m_age;
    std::string m_name;
    parsed_via m_previous_parse_method;

    animal() : m_age(), m_name(), m_previous_parse_method(parsed_via::invalid) {}
    animal(const int& a, const std::string& n, parsed_via p) : m_age(a), m_name(n), m_previous_parse_method(p) {}
};

template <typename BasicJsonType>
void valid_conversion_from_json(const BasicJsonType& j, animal& a, parsed_via pv)
{
    a.m_age = j["age"].template get<int>();
    a.m_name = j["name"].template get<std::string>();
    a.m_previous_parse_method = pv;
}

template <typename BasicJsonType>
void should_not_be_called(const BasicJsonType&, animal& a, parsed_via pv)
{
    a.m_age = 42;
    a.m_name = "it should never be called";
    a.m_previous_parse_method = pv;
}

static bool operator==(const animal& lhs, const animal& rhs)
{
    return std::tie(lhs.m_name, lhs.m_age, lhs.m_previous_parse_method) == std::tie(rhs.m_name, rhs.m_age, rhs.m_previous_parse_method);
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
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }
};

struct dog : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, dog& d)
    {
        valid_conversion_from_json(j, d, parsed_via::static_member_f);
    }
};

struct parrot : animal
{
    using animal::animal;
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, parrot& p)
{
    valid_conversion_from_json(j, p, parsed_via::free_adl_f);
}
}  // namespace udt

TEST_CASE("customization points for default constructible types" * doctest::test_suite("udt"))
{
    SECTION("conversion from json via member function")
    {
        json j = R"({"name":"tigger", "age":5})"_json;

        udt::cat animal;
        j.get_to(animal);
        CHECK(animal == udt::cat{5, "tigger", udt::parsed_via::member_f});


        auto def = j.get<udt::cat>();
        CHECK(def == udt::cat{5, "tigger", udt::parsed_via::member_f});
    }
    SECTION("conversion from json via static function")
    {
        json j = R"({"name":"bella", "age":9})"_json;

        udt::dog animal;
        j.get_to(animal);
        CHECK(animal == udt::dog{9, "bella", udt::parsed_via::static_member_f});

        auto def = j.get<udt::dog>();
        CHECK(def == udt::dog{9, "bella", udt::parsed_via::static_member_f});
    }
    SECTION("conversion from json via free function")
    {
        json j = R"({"name":"greenie", "age":3})"_json;

        udt::parrot animal;
        j.get_to(animal);
        CHECK(animal == udt::parrot{3, "greenie", udt::parsed_via::free_adl_f});

        auto def = j.get<udt::parrot>();
        CHECK(def == udt::parrot{3, "greenie", udt::parsed_via::free_adl_f});
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
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, rabbit& r)
    {
        valid_conversion_from_json(j, r, parsed_via::static_member_f);
    }
};

struct horse : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, horse& h)
    {
        valid_conversion_from_json(j, h, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, horse& h)
{
    should_not_be_called(j, h, parsed_via::free_adl_f);
}

struct hamster : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, hamster& h)
{
    should_not_be_called(j, h, parsed_via::free_adl_f);
}

struct ferret : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, ferret& f)
    {
        should_not_be_called(j, f, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, ferret& f)
{
    should_not_be_called(j, f, parsed_via::free_adl_f);
}
}  // namespace udt

TEST_CASE("priorities of user-defined customizations for default-constructible types" * doctest::test_suite("udt"))
{
    SECTION("have a member function and static method")
    {
        json j = R"({"name":"thumper", "age":1})"_json;

        auto animal = j.get<udt::rabbit>();
        CHECK(animal == udt::rabbit{1, "thumper", udt::parsed_via::member_f});

        auto def = j.get<udt::rabbit>();
        CHECK(def == udt::rabbit{1, "thumper", udt::parsed_via::member_f});
    }
    SECTION("have a static method and free function")
    {
        json j = R"({"name":"lilly", "age":23})"_json;

        auto animal = j.get<udt::horse>();
        CHECK(animal == udt::horse{23, "lilly", udt::parsed_via::static_member_f});

        auto def = j.get<udt::horse>();
        CHECK(def == udt::horse{23, "lilly", udt::parsed_via::static_member_f});
    }
    SECTION("have a member and free function")
    {
        json j = R"({"name":"ace", "age":2})"_json;

        auto animal = j.get<udt::hamster>();
        CHECK(animal == udt::hamster{2, "ace", udt::parsed_via::member_f});

        auto def = j.get<udt::hamster>();
        CHECK(def == udt::hamster{2, "ace", udt::parsed_via::member_f});
    }
    SECTION("have a member function, static method and free function")
    {
        json j = R"({"name":"kirra", "age":4})"_json;

        auto animal = j.get<udt::ferret>();
        CHECK(animal == udt::ferret{4, "kirra", udt::parsed_via::member_f});

        auto def = j.get<udt::ferret>();
        CHECK(def == udt::ferret{4, "kirra", udt::parsed_via::member_f});
    }
}  // namespace udt

namespace udt
{
template <typename ValueType, typename BasicJsonType>
ValueType valid_conversion_from_json(const BasicJsonType& j, parsed_via pv)
{
    ValueType p
    {
        j["age"].template get<int>(),
        j["name"].template get<std::string>(),
        pv
    };
    return p;
}

template <typename ValueType, typename BasicJsonType>
void should_not_be_called(const BasicJsonType&, parsed_via pv)
{
    ValueType p
    {
        42,
        "it should never be called",
        pv
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
        return valid_conversion_from_json<mice>(j, parsed_via::non_default_static_f);
    }
};

struct fish : animal
{
    using animal::animal;

    fish() = delete;
};

template <typename BasicJsonType>
fish from_json(const BasicJsonType& j, nlohmann::adl_tag<fish>)
{
    return valid_conversion_from_json<fish>(j, parsed_via::nob_default_adl_f);
}

struct turtle : animal
{
    using animal::animal;

    turtle() = delete;

    template <typename BasicJsonType>
    static turtle from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<turtle>(j, parsed_via::non_default_static_f);
    }
};

template <typename BasicJsonType>
turtle from_json(const BasicJsonType& j, nlohmann::adl_tag<turtle>)
{
    return should_not_be_called<turtle>(j, parsed_via::nob_default_adl_f);
}
}  // namespace udt

TEST_CASE("customization points for non-default constructible types" * doctest::test_suite("udt"))
{
    SECTION("conversion from json via member function")
    {
        json j = R"({"name":"basil", "age":1})"_json;

        auto animal = j.get<udt::mice>();
        CHECK(animal == udt::mice{1, "basil", udt::parsed_via::non_default_static_f});
    }
    SECTION("conversion from json via free function")
    {
        json j = R"({"name":"jack", "age":22})"_json;

        auto animal = j.get<udt::fish>();
        CHECK(animal == udt::fish{22, "jack", udt::parsed_via::nob_default_adl_f});
    }
    SECTION("priority between member and free function")
    {
        json j = R"({"name":"shelby", "age":50})"_json;

        auto animal = j.get<udt::turtle>();
        CHECK(animal == udt::turtle{50, "shelby", udt::parsed_via::non_default_static_f});
    }
}

namespace udt
{ // two correct
struct gerbil : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static gerbil from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<gerbil>(j, parsed_via::non_default_static_f);
    }

};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, gerbil& g)
{
    should_not_be_called(j, g, parsed_via::free_adl_f);
}

template <typename BasicJsonType>
gerbil from_json(const BasicJsonType& j, nlohmann::adl_tag<gerbil>)
{
    return should_not_be_called<gerbil>(j, parsed_via::nob_default_adl_f);
}

struct chipmunk : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static chipmunk from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, chipmunk& c)
    {
        return should_not_be_called(j, c, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
chipmunk from_json(const BasicJsonType& j, nlohmann::adl_tag<chipmunk>)
{
    return should_not_be_called<chipmunk>(j, parsed_via::nob_default_adl_f);
}

struct potbelly_pig : animal
{
    using animal::animal;
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, potbelly_pig& p)
{
    return valid_conversion_from_json<potbelly_pig>(j, p, parsed_via::free_adl_f);
}

template <typename BasicJsonType>
potbelly_pig from_json(const BasicJsonType& j, nlohmann::adl_tag<potbelly_pig>)
{
    return valid_conversion_from_json<potbelly_pig>(j, parsed_via::nob_default_adl_f);
}

struct chinchilla : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static chinchilla from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<chinchilla>(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, chinchilla& c)
    {
        return valid_conversion_from_json<chinchilla>(j, c, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, chinchilla& c)
{
    should_not_be_called(j, c, parsed_via::free_adl_f);
}

template <typename BasicJsonType>
chinchilla from_json(const BasicJsonType& j, nlohmann::adl_tag<chinchilla>)
{
    should_not_be_called<chinchilla>(j, parsed_via::nob_default_adl_f);
}

struct iguana : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json<iguana>(j, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static iguana from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<iguana>(j, parsed_via::non_default_static_f);
    }
};

struct spider : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static spider from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<spider>(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, spider& s)
    {
        should_not_be_called(j, s, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
void from_json(const BasicJsonType& j, spider& f)
{
    should_not_be_called(j, f, parsed_via::free_adl_f);
}

template <typename BasicJsonType>
spider from_json(const BasicJsonType& j, nlohmann::adl_tag<spider>)
{
    return should_not_be_called<spider>(j, parsed_via::nob_default_adl_f);
}

struct hedgehog : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static hedgehog from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<hedgehog>(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, hedgehog& h)
    {
        return valid_conversion_from_json<chinchilla>(j, h, parsed_via::static_member_f);
    }
};

template <typename BasicJsonType>
hedgehog from_json(const BasicJsonType& j, nlohmann::adl_tag<hedgehog>)
{
    return should_not_be_called<hedgehog>(j, parsed_via::nob_default_adl_f);
}

struct rodent : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j)
    {
        valid_conversion_from_json(j, *this, parsed_via::member_f);
    }

    template <typename BasicJsonType>
    static rodent from_json(const BasicJsonType& j)
    {
         return valid_conversion_from_json<rodent>(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, rodent& r)
    {
        should_not_be_called(j, r, parsed_via::static_member_f);
    }
};

struct guinea_pig : animal
{
    using animal::animal;

    template <typename BasicJsonType>
    static guinea_pig from_json(const BasicJsonType& j)
    {
        return valid_conversion_from_json<guinea_pig>(j, parsed_via::non_default_static_f);
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& j, guinea_pig& p)
    {
        return valid_conversion_from_json<guinea_pig>(j, p, parsed_via::static_member_f);
    }
};
}  // namespace udt
