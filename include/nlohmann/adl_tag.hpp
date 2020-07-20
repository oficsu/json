#pragma once

namespace nlohmann
{
/*!
@brief tag for ADL

Nested template parameter is used for TAD
([template argument deduction](https://en.cppreference.com/w/cpp/language/template_argument_deduction))
when user can't provide function which accepts needed typename as parameter
Such function argument can be used for ADL
([argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl))
by nested template parameter for serialization.

@todo I don't know where is the right place for tag, thus i choosed separate file
*/
template<typename>
struct adl_tag {};
} // namespace nlohmann
