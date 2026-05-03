#pragma once

#include <iterator>
#include <limits>
#include <type_traits>

namespace Fmi
{

template <typename T, typename = void>
struct is_iterable : std::false_type {};

// this gets used only when we can call std::begin() and std::end() on that type
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())),
                                  decltype(std::end(std::declval<T&>())) >
                  > : std::true_type {};

//=============================================================================================

// Helper struct to check if T is a numeric type (integer or floating point)
template <typename T>
struct is_numeric : std::integral_constant<bool,
    std::is_arithmetic<T>::value && !std::is_same<T, bool>::value> {};



template <typename Source>
constexpr typename std::enable_if<std::is_integral_v<Source> && !std::is_same_v<Source, bool>, Source>::type
lower_limit()
{
    return std::numeric_limits<Source>::min();
}

template <typename Source>
constexpr typename std::enable_if<std::is_floating_point_v<Source>, Source>::type
lower_limit()
{
    return - std::numeric_limits<Source>::max();
}

template <typename Source>
constexpr Source upper_limit()
{
    return std::numeric_limits<Source>::max();
}

template <typename... Args>
constexpr bool are_all_parameters_copyable()
{
    return (std::is_copy_constructible<Args>::value && ...);
}

template <typename... Args>
constexpr bool are_all_parameters_assignable()
{
    return (std::is_copy_assignable<Args>::value && ...);
}

}  // namespace Fmi
