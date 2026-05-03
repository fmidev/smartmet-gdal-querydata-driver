// ======================================================================
/*!
 * \brief Parse timestamps
 *
 * Known formats: iso, epoch, timestamp, sql, xml
 *
 * \deprecated
 *
 * This header is deprecated. Please use DateTimeParser instead, which
 * is faster when used in loops. This header will be removed once FMI
 * has ported all code to use DateTimeParser instead.
 *
 * Note that DateTimeParser reveals only a minimal set of methods.
 * Some private methods hidden behind an implementation pointer
 * may have to be made public once old code is ported.
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"
#include <string>

namespace Fmi
{
namespace TimeParser
{
// Generic parsers

Fmi::LocalDateTime parse(const std::string& str,
                                         const std::string& format,
                                         Fmi::TimeZonePtr tz);

Fmi::LocalDateTime parse(const std::string& str,
                                         Fmi::TimeZonePtr tz);

DateTime parse(const std::string& str, const std::string& format);

DateTime parse(const std::string& str);

// Individual format parsers

std::string looks(const std::string& str);
bool looks_utc(const std::string& str);

DateTime try_parse_iso(const std::string& str, bool* isutc);
DateTime try_parse_offset(const std::string& str);

DateTime parse_iso(const std::string& str);
DateTime parse_epoch(const std::string& str);
DateTime parse_sql(const std::string& str);
DateTime parse_fmi(const std::string& str);
DateTime parse_offset(const std::string& str);

// Specialized parsers

DateTime parse_http(const std::string& str);

// Local date time creator which handles DST changes nicely. Used internally
// and provided as a convenice function for external parsers

using date_time::make_time;

TimeDuration try_parse_duration(const std::string& str);
TimeDuration try_parse_iso_duration(const std::string& str);

TimeDuration parse_duration(const std::string& str);
TimeDuration parse_iso_duration(const std::string& str);

}  // namespace TimeParser
}  // namespace Fmi

// ======================================================================
