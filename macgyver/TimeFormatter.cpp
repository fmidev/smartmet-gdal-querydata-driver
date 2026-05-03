// ======================================================================
/*!
 * \brief Format boost time objects
 */
// ======================================================================

#include "TimeFormatter.h"
#include "Exception.h"
#include "StringConversion.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <array>
#include <stdexcept>

namespace Fmi
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor defined here to prevent existence of a weak vtable
 */
// ----------------------------------------------------------------------

TimeFormatter::~TimeFormatter() = default;
// ----------------------------------------------------------------------
/*!
 * \brief ISO-formatter (see boost manuals)
 *
 * Convert to form YYYYMMDDTHHMMSS,fffffffff where T is the date-time separator
 */
// ----------------------------------------------------------------------

struct IsoFormatter : public TimeFormatter
{
  IsoFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief SQL-formatter
 *
 * Convert to form YYYY-MM-DD HH:MM:SS
 */
// ----------------------------------------------------------------------

struct SqlFormatter : public TimeFormatter
{
  SqlFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief XML-formatter
 *
 * Convert to form YYYY-MM-DDTHH:MM:SS
 */
// ----------------------------------------------------------------------

struct XmlFormatter : public TimeFormatter
{
  XmlFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief Epoch-formatter
 *
 * Convert to form ssssss, time in seconds since epoch 1970-01-01 00:00:00
 */
// ----------------------------------------------------------------------

struct EpochFormatter : public TimeFormatter
{
  EpochFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief Timestamp-formatter
 *
 * Convert to form YYYYMMDDHHMM
 */
// ----------------------------------------------------------------------

struct TimeStampFormatter : public TimeFormatter
{
  TimeStampFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief HTTP-date formatter
 *
 * Convert to form Sun, 06 Nov 1994 08:49:37 GMT
 */
// ----------------------------------------------------------------------

struct HttpFormatter : public TimeFormatter
{
  HttpFormatter() = default;
  std::string format(const DateTime& t) const override;
  std::string format(const Fmi::LocalDateTime& t) const override;
};

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime
 */
// ----------------------------------------------------------------------

std::string IsoFormatter::format(const DateTime& t) const
{
  try
  {
    return Fmi::to_iso_string(t);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string IsoFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    return Fmi::to_iso_string(t.local_time());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime
 */
// ----------------------------------------------------------------------

std::string SqlFormatter::format(const DateTime& t) const
{
  try
  {
    std::string tmp = Fmi::to_iso_extended_string(t);
    tmp[10] = ' ';
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string SqlFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    std::string tmp = Fmi::to_iso_extended_string(t.local_time());
    tmp[10] = ' ';
    return tmp;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime
 */
// ----------------------------------------------------------------------

std::string XmlFormatter::format(const DateTime& t) const
{
  try
  {
    return Fmi::to_iso_extended_string(t);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string XmlFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    return Fmi::to_iso_extended_string(t.local_time());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime
 */
// ----------------------------------------------------------------------

std::string EpochFormatter::format(const DateTime& t) const
{
  try
  {
    time_t epo = (t - Fmi::Date::epoch).total_seconds();
    return Fmi::to_string(epo);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string EpochFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    const auto tmp = t.utc_time();
    time_t epo = (tmp - Fmi::Date::epoch).total_seconds();
    return Fmi::to_string(epo);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime: YYYYMMDDHHMI
 */
// ----------------------------------------------------------------------

std::string TimeStampFormatter::format(const DateTime& t) const
{
  try
  {
    return Fmi::to_timestamp_string(t);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string TimeStampFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    return Fmi::to_timestamp_string(t.local_time());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a ptime into style Sun, 06 Nov 1994 08:49:37 GMT
 */
// ----------------------------------------------------------------------

std::string HttpFormatter::format(const DateTime& t) const
{
  try
  {
    return Fmi::to_http_string(t);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a local date time
 */
// ----------------------------------------------------------------------

std::string HttpFormatter::format(const Fmi::LocalDateTime& t) const
{
  try
  {
    return format(t.utc_time());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a time formatter
 */
// ----------------------------------------------------------------------

TimeFormatter* TimeFormatter::create(const std::string& name)
{
  try
  {
    if (name == "iso")
      return new IsoFormatter();
    if (name == "sql")
      return new SqlFormatter();
    if (name == "xml")
      return new XmlFormatter();
    if (name == "epoch")
      return new EpochFormatter();
    if (name == "timestamp")
      return new TimeStampFormatter();
    if (name == "http")
      return new HttpFormatter();

    throw Fmi::Exception(BCP, "Unknown time format '" + name + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi

// ======================================================================
