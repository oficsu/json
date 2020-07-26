#pragma once

#include <utility>

#include <nlohmann/adl_tag.hpp>
#include <nlohmann/detail/conversions/from_json.hpp>
#include <nlohmann/detail/conversions/to_json.hpp>

namespace nlohmann
{

template<typename ValueType, typename>
struct adl_serializer
{
    /*!
    @brief convert a JSON value to any value type

    This function is usually called by the `get()` function of the
    @ref basic_json class (either explicit or via conversion operators).
    */
    template<typename... Args>
    static auto from_json(Args&& ... args) noexcept(
    noexcept(::nlohmann::from_json.call(adl_tag<ValueType> {}, detail::max_priority_t{}, std::forward<Args>(args)...)))
    -> decltype(::nlohmann::from_json.call(adl_tag<ValueType> {}, detail::max_priority_t{}, std::forward<Args>(args)...))
    {
        return ::nlohmann::from_json.call(adl_tag<ValueType> {}, detail::max_priority_t{}, std::forward<Args>(args)...);
    }

    /*!
    @brief convert any value type to a JSON value

    This function is usually called by the constructors of the @ref basic_json
    class.

    @param[in,out] j  JSON value to write to
    @param[in] val    value to read from
    */
    template<typename BasicJsonType, typename ValueTypeCV>
    static auto to_json(BasicJsonType& j, ValueTypeCV&& val) noexcept(
        noexcept(::nlohmann::to_json(j, std::forward<ValueTypeCV>(val))))
    -> decltype(::nlohmann::to_json(j, std::forward<ValueTypeCV>(val)), void())
    {
        static_assert(std::is_same<detail::uncvref_t<ValueTypeCV>, detail::uncvref_t<ValueType>>::value,
                      "ValueType from specialization and passed ValueTypeCV should be same");
        ::nlohmann::to_json(j, std::forward<ValueTypeCV>(val));
    }
};

}  // namespace nlohmann
