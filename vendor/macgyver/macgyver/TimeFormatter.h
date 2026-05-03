// ======================================================================
/*!
 * \brief Format timestamps
 *
 * Known formats: iso, epoch, timestamp, sql, xml
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"
#include <string>

namespace Fmi
{
class TimeFormatter
{
 public:
  static TimeFormatter* create(const std::string& name);

  virtual ~TimeFormatter();

  virtual std::string format(const DateTime& t) const = 0;

  virtual std::string format(const Fmi::LocalDateTime& t) const = 0;
};
}  // namespace Fmi

// ======================================================================
