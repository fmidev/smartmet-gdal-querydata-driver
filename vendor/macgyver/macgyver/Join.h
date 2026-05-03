#pragma once

#include "StringConversion.h"
#include <string>
#include <type_traits>

namespace Fmi
{
namespace detail
{
template <typename T>
struct field_converter
{
  const std::string& operator()(const std::string& value) const { return value; }
  const char* operator()(const char* value) const { return value; }

  template <typename U, typename = std::enable_if_t<!std::is_same_v<std::decay_t<U>, std::string>>>
  std::string operator()(const U& value) const
  {
    return Fmi::to_string(value);
  }
};
}  // namespace detail

template <typename Container>
std::string join(const Container& c, const std::string& delimiter = ",")
{
  std::string result;
  bool first = true;
  for (const auto& value : c)
  {
    if (!first)
      result += delimiter;
    result += detail::field_converter<std::decay_t<decltype(value)>>{}(value);
    first = false;
  }
  return result;
}

template <typename Container,
          typename Converter,
          typename = std::enable_if_t<!std::is_convertible_v<Converter, std::string>>>
std::string join(const Container& c, Converter converter, const std::string& delimiter = ",")
{
  std::string result;
  bool first = true;
  for (const auto& value : c)
  {
    if (!first)
      result += delimiter;
    // may return int or whatever
    auto fieldValue = converter(value);
    // force conversion to string before append
    result += detail::field_converter<std::decay_t<decltype(fieldValue)>>{}(fieldValue);
    first = false;
  }
  return result;
}

}  // namespace Fmi
