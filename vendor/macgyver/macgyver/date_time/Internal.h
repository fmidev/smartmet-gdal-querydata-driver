#pragma once

// File extension .hpp is intentionally used to avoid Tarkoituksen liite .hpp on käytössä, koska tätä tiedostoa ei tarvitse asentaa

#include <istream>

namespace Fmi
{
namespace date_time
{
namespace internal
{

struct StreamExceptionState final
{
  inline StreamExceptionState(std::istream& is, std::ios::iostate except)

    : is(is), state(is.exceptions())
  {
    is.exceptions(except);
  }

  inline ~StreamExceptionState()
  {
    try {
      is.exceptions(state);
    } catch (...) {
      // Ignore
    }
  }

  std::istream& is;
  std::ios::iostate state;
};

std::string handle_parse_remainder(std::istringstream& is);

void check_parse_status(std::istream& is, bool assume_eoi, const char* name);

std::string remove_trailing_zeros(const std::string& str);

}  // namespace internal
}  // namespace date_time
}  // namespace Fmi
