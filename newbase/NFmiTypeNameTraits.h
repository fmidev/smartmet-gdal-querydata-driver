// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiTypeNameTraits
 */
// ======================================================================

#pragma once

#include <string>

#define SPECIALIZE(X, Y)             \
  template <>                        \
  inline const std::string Name<X>() \
  {                                  \
    return Y;                        \
  }

namespace NFmiTypeNameTraits
{
template <typename T>
inline const std::string Name()
{
  return "(unknown)";
}

SPECIALIZE(unsigned char, "unsigned char")
SPECIALIZE(unsigned short, "unsigned short")
SPECIALIZE(unsigned int, "unsigned int")
SPECIALIZE(unsigned long, "unsigned long")

SPECIALIZE(char, "char")
SPECIALIZE(short, "short")
SPECIALIZE(int, "int")
SPECIALIZE(long, "long")

SPECIALIZE(bool, "bool")
SPECIALIZE(float, "float")
SPECIALIZE(double, "double")
}  // namespace NFmiTypeNameTraits

// ======================================================================
