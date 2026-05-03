#pragma once

/*!
 * \brief Fast replacements for math commands when compiler fast math options cannot be enabled
 */

namespace Fmi
{
// https://stackoverflow.com/questions/824118/why-is-floor-so-slow/30308919#30308919

inline long floor(double x)
{
  return static_cast<long>(x) - (x < static_cast<long>(x));
}

inline long ceil(double x)
{
  return static_cast<long>(x) + (x > static_cast<long>(x));
}

}  // namespace Fmi
