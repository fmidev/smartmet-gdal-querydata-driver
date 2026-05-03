// ======================================================================
/*!
 * \brief Parse timestamps
 *
 * This replaces the TimeParser namespace, which is not efficient when
 * parsing large amounts of timestamps such as when parsing database
 * output.
 *
 * Known time formats: iso, epoch, timestamp, sql, xml
 * Known duration formats: iso, FMI custom
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"
#include <string>

namespace Fmi
{
class DateTimeParser
{
 public:
  ~DateTimeParser();
  DateTimeParser();
  DateTimeParser(const DateTimeParser& other);
  DateTimeParser(DateTimeParser&& other);
  DateTimeParser& operator=(const DateTimeParser& other);
  DateTimeParser& operator=(DateTimeParser&& other);

  Fmi::LocalDateTime parse(const std::string& str,
                                           const std::string& format,
                                           Fmi::TimeZonePtr tz) const;
  Fmi::LocalDateTime parse(const std::string& str,
                                           Fmi::TimeZonePtr tz) const;

  DateTime parse(const std::string& str, const std::string& format) const;
  DateTime parse(const std::string& str) const;

  DateTime parse_http(const std::string& str) const;

  TimeDuration parse_duration(const std::string& str) const;
  TimeDuration parse_iso_duration(const std::string& str) const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl;

};  // class DateTimeParser
}  // namespace Fmi

// ======================================================================
