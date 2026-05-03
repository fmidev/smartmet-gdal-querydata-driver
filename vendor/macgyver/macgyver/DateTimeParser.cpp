
// ======================================================================
/*!
 * \brief Parse timestamps
 */
// ======================================================================

#include "DateTimeParser.h"
#include "Exception.h"

#include "TimeParserDefinitions.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <cctype>
#include <stdexcept>

using namespace Fmi::TimeParser;

namespace
{
Fmi::DateTime bad_date;
Fmi::TimeDuration bad_duration;

boost::regex iso8601_weeks{"^P(\\d+)W$"};
boost::regex iso8601_short{"^P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?$"};
#if 0
boost::regex iso8601_long{
    "^P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?T([[:d:]]+H)?([[:d:]]+M)?([[:d:]]+S|[[:d:]]+\\.[[:d:]]+"
    "S)?$"};
#endif

boost::regex iso8601_long{
    "^P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?(T([[:d:]]+H)?([[:d:]]+M)?([[:d:]]+S|[[:d:]]+\\.[[:d:]]+"
    "S)?)?$"};

Fmi::DateTime buildFromSQL(const TimeStamp& target)
{
  try
  {
    unsigned int hour = 0, minute = 0, second = 0;
    if (target.hour)
      hour = *target.hour;
    if (target.minute)
      minute = *target.minute;
    if (target.second)
      second = *target.second;

    Fmi::DateTime ret;

    // Translate the exception to runtime_error
    try
    {
      ret = Fmi::DateTime(Fmi::Date(target.year, target.month, target.day),
                                     Fmi::TimeDuration(hour, minute, second));
    }
    catch (const std::exception& err)
    {
      throw Fmi::Exception(BCP, err.what());
    }

    return ret;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime buildFromISO(const TimeStamp& target)
{
  try
  {
    unsigned int hour = 0, minute = 0, second = 0;

    if (target.hour)
      hour = *target.hour;
    if (target.minute)
      minute = *target.minute;
    if (target.second)
      second = *target.second;

    Fmi::DateTime res;

    try
    {
      res = Fmi::DateTime(Fmi::Date(target.year, target.month, target.day),
                                     Fmi::TimeDuration(hour, minute, second));
    }
    catch (const std::exception& err)
    {
      throw Fmi::Exception(BCP, err.what());
    }

    // Do timezone
    // Sign is parsed separately to avoid mixing unsigned and
    // signed values in hour and minute definitions. The sign
    // must be parsed exactly once, not separately for hour and minute
    if (target.tz.sign == '+')
    {
      res -= Fmi::Hours(target.tz.hours);
      res -= Fmi::Minutes(target.tz.minutes);
    }
    else
    {
      res += Fmi::Hours(target.tz.hours);
      res += Fmi::Minutes(target.tz.minutes);
    }

    return res;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime buildFromEpoch(const UnixTime& target)
{
  try
  {
    return Fmi::date_time::from_time_t(target);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

Fmi::DateTime buildFromOffset(Fmi::TimeDuration offset)
{
  try
  {
    // Apply to current time rounded to closest minute

    Fmi::DateTime now = Fmi::SecondClock::universal_time();
    Fmi::TimeDuration tnow = now.time_of_day();
    int secs = tnow.seconds();

    if (secs >= 30)
      offset += Fmi::Seconds(60 - secs);  // round up
    else
      offset -= Fmi::Seconds(secs);  // round down

    // Construct the shifted time

    return Fmi::DateTime(now.date(), tnow + offset);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

std::uint16_t get_short_month(const std::string& str)
{
  try
  {
    if (str == "Jan")
      return 1;
    if (str == "Feb")
      return 2;
    if (str == "Mar")
      return 3;
    if (str == "Apr")
      return 4;
    if (str == "May")
      return 5;
    if (str == "Jun")
      return 6;
    if (str == "Jul")
      return 7;
    if (str == "Aug")
      return 8;
    if (str == "Sep")
      return 9;
    if (str == "Oct")
      return 10;
    if (str == "Nov")
      return 11;
    if (str == "Dec")
      return 12;

    throw Fmi::Exception(BCP, "Invalid month name '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool is_short_month(const std::string& str)
{
  try
  {
    return (get_short_month(str) > 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool is_short_weekday(const std::string& str)
{
  try
  {
    return (str == "Sun" || str == "Mon" || str == "Tue" || str == "Wed" || str == "Thu" ||
            str == "Fri" || str == "Sat");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

bool is_long_weekday(const std::string& str)
{
  try
  {
    return (str == "Sunday" || str == "Monday" || str == "Tuesday" || str == "Wednesday" ||
            str == "Thursday" || str == "Friday" || str == "Saturday");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse an unsigned integer from a C-string
 */
// ----------------------------------------------------------------------

bool parse_ushort(const char** str, unsigned int length, std::uint16_t* value)
{
  try
  {
    const char* ptr = *str;

    std::uint16_t tmp = 0;
    for (unsigned int i = 0; i < length; i++)
    {
      if (!isdigit(*ptr))
        return false;
      tmp *= 10;
      tmp += static_cast<unsigned int>(*ptr - '0');
      ++ptr;
    }
    *value = tmp;
    *str = ptr;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief ISO 8601 dates use "-" as a date separator and ":" as a time separator
 */
// ----------------------------------------------------------------------

bool skip_separator(const char** str, char separator, bool extended_format)
{
  try
  {
    if (!extended_format)
      return true;
    if (**str != separator)
      return false;
    ++*str;
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return true if string looks like a nonnegative integer
 */
// ----------------------------------------------------------------------

bool looks_integer(const std::string& str)
{
  try
  {
    return boost::algorithm::all(str, boost::algorithm::is_digit());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if string looks like an SQL timestamp
 *
 * Format: YYYY-MM-DD HH:MI:SS
 */
// ----------------------------------------------------------------------

bool looks_sql(const std::string& t)
{
  try
  {
    return (t.size() == 19 && t[4] == '-' && t[7] == '-' && t[10] == ' ' && t[13] == ':' &&
            t[16] == ':' && looks_integer(t.substr(0, 4)) && looks_integer(t.substr(5, 2)) &&
            looks_integer(t.substr(8, 2)) && looks_integer(t.substr(11, 2)) &&
            looks_integer(t.substr(14, 2)) && looks_integer(t.substr(17, 2)));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if string looks like a epoch time
 */
// ----------------------------------------------------------------------

bool looks_epoch(const std::string& t)
{
  try
  {
    return looks_integer(t);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if string looks like a time offset
 */
// ----------------------------------------------------------------------

bool looks_offset(const std::string& str)
{
  try
  {
    if (str.empty())
      return false;

    return (str == "0" || (str.size() == 2 && str[0] == '0') ||  // 0m, 0h etc
            str[0] == '+' || str[0] == '-');
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse an ISO8601 time duration
 *
 * - https://en.wikipedia.org/wiki/ISO_8601#Durations
 * - https://stackoverflow.com/questions/23886140/parse-iso-8601-durations
 */
//----------------------------------------------------------------------

Fmi::TimeDuration try_parse_iso_duration(const std::string& str)
{
  try
  {
    boost::smatch match;

    if (boost::regex_search(str, match, iso8601_weeks))
    {
      int n = std::stoi(match[1]);
      return Fmi::Hours(7 * 24 * n);
    }

    if (!boost::regex_search(str, match, iso8601_long))
      return Fmi::TimeDuration();

    // years, months, days, tmp , hours, minutes, seconds
    std::vector<int> vec{0, 0, 0, -1, 0, 0, 0};

    for (size_t i = 1; i < match.size(); ++i)
    {
      if (match[i].matched && i != 4)
      {
        std::string tmp = match[i];
        tmp.pop_back();
        vec[i - 1] = std::stoi(tmp);
      }
    }

    if (vec[1] < 0 || vec[1] > 12)
      return Fmi::TimeDuration();
    if (vec[4] < 0 || vec[4] > 24)
      return Fmi::TimeDuration();

    // Year length 365 and month length 30 are arbitrary choices here

    return Fmi::Hours(365 * 24 * vec[0] + 30 * 24 * vec[1] + 24 * vec[2]) +
           Fmi::TimeDuration(vec[4], vec[5], vec[6], 0);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Try to parse iso format
 *
 * \param str The string to parse
 * \param isutc Flag to indicate if the time is in UTC
 * \return The parsed time or an invalid time
 *
 * We ignore
 *
 *  - fractional times
 *  - week format
 *  - day of year format
 */
// ----------------------------------------------------------------------

Fmi::DateTime try_parse_iso(const std::string& str, bool* isutc)
{
  try
  {
    std::uint16_t year = 0;
    std::uint16_t month = 1;
    std::uint16_t day = 1;
    std::uint16_t hour = 0;
    std::uint16_t minute = 0;
    std::uint16_t second = 0;
    std::uint16_t houroffset = 0;
    std::uint16_t minuteoffset = 0;
    bool positiveoffset = false;

    const char* ptr = str.c_str();

    // By default the time is in local time
    *isutc = false;

    // Year

    if (!parse_ushort(&ptr, 4, &year))
      return Fmi::DateTime();

    // Quick sanity check to prevent further useless parsing
    // - boost library version 1.34 or greater support dates
    //   at least in the range 1400-Jan-01 to 9999-Dec-31
    // - Dates prior to 1582 using the Julian Calendar
    if (year < 1582 || year > 5000)
      return Fmi::DateTime();

    // Establish whether we have basic or extended format

    bool extended_format = (*ptr == '-');

    // Month

    if (!skip_separator(&ptr, '-', extended_format))
      return Fmi::DateTime();  // should never happen though
    if (!parse_ushort(&ptr, 2, &month))
      return Fmi::DateTime();  // YYYY is not allowed
    if (month == 0 || month > 12)
      return Fmi::DateTime();

    if (*ptr == '\0')
    {
      if (!extended_format)
        return Fmi::DateTime();  // YYYYMM is not allowed
      goto build_iso;     // YYYY-MM is allowed
    }

    // Day

    if (!skip_separator(&ptr, '-', extended_format))
      return Fmi::DateTime();
    if (!parse_ushort(&ptr, 2, &day))
      return Fmi::DateTime();
    if (day == 0 || day > 31)
      return Fmi::DateTime();
    if (*ptr == '\0')
      goto build_iso;  // YYYY-MM-DD is allowed

    // We permit omitting 'T' to enable old YYYYMMDDHHMI timestamp format

    if (*ptr == 'T')
      ++ptr;
    if (!parse_ushort(&ptr, 2, &hour))
      return Fmi::DateTime();
    if (hour > 23)
      return Fmi::DateTime();
    if (*ptr == '\0')
      goto build_iso;  // YYYY-MM-DDTHH is allowed

    if (*ptr == 'Z' || *ptr == '+' || *ptr == '-')
      goto zone_began;

    if (!skip_separator(&ptr, ':', extended_format))
      return Fmi::DateTime();
    if (!parse_ushort(&ptr, 2, &minute))
      return Fmi::DateTime();
    if (minute > 59)
      return Fmi::DateTime();
    if (*ptr == '\0')
      goto build_iso;  // YYYY-MM-DDTHH:MI is allowed

    if (*ptr == 'Z' || *ptr == '+' || *ptr == '-')
      goto zone_began;

    if (!skip_separator(&ptr, ':', extended_format))
      return Fmi::DateTime();
    if (!parse_ushort(&ptr, 2, &second))
      return Fmi::DateTime();
    if (second > 59)
      return Fmi::DateTime();
    if (*ptr == '\0')
      goto build_iso;  // YYYY-MM-DDTHH:MI:SS is allowed

    if (*ptr != 'Z' && *ptr != '+' && *ptr != '-')
      return Fmi::DateTime();

  zone_began:

    *isutc = true;
    if (*ptr == 'Z')
    {
      ++ptr;
      if (*ptr != '\0')
        return Fmi::DateTime();
      goto build_iso;
    }

    positiveoffset = (*ptr == '+');
    ptr++;

    if (!parse_ushort(&ptr, 2, &houroffset))
      return Fmi::DateTime();
    if (houroffset >= 14)
      return Fmi::DateTime();  // some offsets are > 12

    if (*ptr == '\0')
      goto build_iso;

    if (!skip_separator(&ptr, ':', extended_format))
      return Fmi::DateTime();
    if (!parse_ushort(&ptr, 2, &minuteoffset))
      return Fmi::DateTime();
    if (*ptr != '\0')
      return Fmi::DateTime();

  build_iso:

    Fmi::DateTime t(Fmi::Date(year, month, day),
                               Fmi::TimeDuration(hour, minute, second));

    // Adjust if necessary

    if (houroffset != 0 || minuteoffset != 0)
    {
      if (positiveoffset)
        t -= (Fmi::Hours(houroffset) + Fmi::Minutes(minuteoffset));
      else
        t += (Fmi::Hours(houroffset) + Fmi::Minutes(minuteoffset));
    }

    return t;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if string looks like a iso timestamp
 */
// ----------------------------------------------------------------------

bool looks_iso(const std::string& str)
{
  try
  {
    bool utc;
    Fmi::DateTime t = try_parse_iso(str, &utc);
    return !t.is_not_a_date_time();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

namespace Fmi
{
// ----------------------------------------------------------------------
/*!
 * \brief DateTime parser implementation details
 */
// ----------------------------------------------------------------------

struct DateTimeParser::Impl
{
  using const_iterator = std::string::const_iterator;
  TimeParser::ISOParser<const_iterator> itsISOParser{};
  TimeParser::FMIParser<const_iterator> itsFMIParser{};
  TimeParser::SQLParser<const_iterator> itsSQLParser{};
  TimeParser::OffsetParser<const_iterator> itsOffsetParser{};
  TimeParser::EpochParser itsEpochParser{};

  DateTime parse(const std::string& str) const;
  DateTime parse_iso(const std::string& str) const;
  DateTime parse_epoch(const std::string& str) const;
  DateTime parse_sql(const std::string& str) const;
  DateTime parse_fmi(const std::string& str) const;
  DateTime parse_offset(const std::string& str) const;

  std::string looks(const std::string& str) const;
  bool looks_utc(const std::string& str) const;

  DateTime try_parse_offset(const std::string& str) const;

  TimeDuration try_parse_duration(const std::string& str) const;

  DateTime match_and_parse(const std::string& str, ParserId& matchedParser) const;
};

// ----------------------------------------------------------------------
/*!
 * \brief Parse ISO time format
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse_iso(const std::string& str) const
{
  try
  {
    using iterator = std::string::const_iterator;
    TimeParser::TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, itsISOParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return buildFromISO(target);

    throw Fmi::Exception(BCP, "Invalid ISO-time: '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse FMI time format
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse_fmi(const std::string& str) const
{
  try
  {
    using iterator = std::string::const_iterator;
    TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, itsFMIParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return buildFromISO(target);

    throw Fmi::Exception(BCP, "Invalid ISO-time: '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse sql format
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse_sql(const std::string& str) const
{
  try
  {
    using iterator = std::string::const_iterator;
    TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, itsSQLParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return buildFromSQL(target);

    throw Fmi::Exception(BCP, "Invalid SQL-time: '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse epoch time
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse_epoch(const std::string& str) const
{
  try
  {
    using iterator = std::string::const_iterator;

    UnixTime target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, itsEpochParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return buildFromEpoch(target);

    throw Fmi::Exception(BCP, "Invalid epoch time: '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Try to parse a time offset
 *
 * \param str The string to parse
 * \return The parsed time or an invalid time
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::try_parse_offset(const std::string& str) const
{
  try
  {
    if (str.empty())
      return Fmi::DateTime::NOT_A_DATE_TIME;

    auto offset = try_parse_duration(str);
    if (offset.is_not_a_date_time())
      return Fmi::DateTime::NOT_A_DATE_TIME;

    return buildFromOffset(offset);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Guess the input format
 */
// ----------------------------------------------------------------------

std::string DateTimeParser::Impl::looks(const std::string& str) const
{
  try
  {
    if (looks_offset(str))
      return "offset";
    if (looks_iso(str))
      return "iso";
    if (looks_sql(str))
      return "sql";
    if (looks_epoch(str))
      return "epoch";

    throw Fmi::Exception(BCP, "Unrecognizable time format in string '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Does the time format look like it is in UTC
 */
// ----------------------------------------------------------------------

bool DateTimeParser::Impl::looks_utc(const std::string& str) const
{
  try
  {
    if (looks_sql(str))
      return false;
    if (looks_offset(str))  // offsets are always relative to the time now
      return true;

    bool utc;
    DateTime t = try_parse_iso(str, &utc);
    if (!t.is_not_a_date_time())
      return utc;

    return looks_epoch(str);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a time offset
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse_offset(const std::string& str) const
{
  try
  {
    if (str.empty())
      throw Fmi::Exception(BCP, "Trying to parse an empty string as a time offset");

    auto offset = try_parse_duration(str);
    if (offset.is_not_a_date_time())
      throw Fmi::Exception(BCP, "Failed to parse '" + str + "' as a duration");

    return buildFromOffset(offset);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a time duration
 *
 * Allowed formats include ISO8601 and for historical reasons simple offsets
 *
 *     ISO8601  (P<n>...)
 *     0		(zero offset)
 *     0m,0h... (zero offset with units)
 *     -+NNNN	(offset in minutes)
 *     +-NNNNm	(offset in minutes)
 *     +-NNNNh	(offset in hours)
 *     +-NNNNd	(offset in days)
 *     +-NNNNw	(offset in weeks)
 *     +-NNNNy	(offset in years)
 */
//----------------------------------------------------------------------

TimeDuration DateTimeParser::Impl::try_parse_duration(
    const std::string& str) const
{
  try
  {
    if (str.empty())
      return Fmi::TimeDuration();

    if (str[0] == 'P')
      return try_parse_iso_duration(str);

    // Old time duration format

    using iterator = std::string::const_iterator;

    TimeOffset target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, itsOffsetParser, target);

    if (!success)
      return Fmi::TimeDuration();

    int offset_value;

    // Handle the sign
    if (target.sign == '-')
      offset_value = static_cast<int>(-target.value);
    else
      offset_value = static_cast<int>(target.value);

    // Default unit is minutes
    if (!target.unit)
      return Fmi::Minutes(offset_value);

    char theUnit = *target.unit;

    if (theUnit == 's' || theUnit == 'S')
      return Fmi::Seconds(offset_value);

    if (theUnit == 'm' || theUnit == 'M')
      return Fmi::Minutes(offset_value);

    if (theUnit == 'h' || theUnit == 'H')
      return Fmi::Hours(offset_value);

    if (theUnit == 'd' || theUnit == 'D')
      return Fmi::Hours(offset_value * 24);

    if (theUnit == 'w' || theUnit == 'W')
      return Fmi::Hours(offset_value * 24 * 7);

    if (theUnit == 'y' || theUnit == 'Y')
      return Fmi::Hours(offset_value * 24 * 365);

    return Fmi::TimeDuration();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a time string and return the matched parser
 */
// ----------------------------------------------------------------------
DateTime DateTimeParser::Impl::match_and_parse(const std::string& str,
                                                               ParserId& matchedParser) const
{
  try
  {
    using iterator = std::string::const_iterator;

    {
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, itsISOParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          DateTime ret;
          ret = buildFromISO(target);
          matchedParser = ISO;
          return ret;
        }
        catch (...)
        {
          // Simply pass to the next parser
        }
      }
    }

    {
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, itsFMIParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          DateTime ret;
          ret = buildFromISO(target);  // Similar building as with ISO parser
          matchedParser = FMI;
          return ret;
        }
        catch (...)
        {
          // Simply pass to the next parser
        }
      }
    }

    {
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, itsSQLParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          DateTime ret;
          ret = buildFromSQL(target);
          matchedParser = SQL;
          return ret;
        }
        catch (...)
        {
          // Simply pass to the next parser
        }
      }
    }

    {
      auto ret = try_parse_offset(str);
      if (!ret.is_not_a_date_time())
      {
        matchedParser = OFFSET;
        return ret;
      }
    }

    {
      UnixTime target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, itsEpochParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          DateTime ret;
          ret = buildFromEpoch(target);
          matchedParser = EPOCH;
          return ret;
        }
        catch (...)
        {
          // Simply pass to the next parser
        }
      }
    }

    // Control is here, no match
    throw Fmi::Exception(BCP, "Unknown time string '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Guess time format and parse the time
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::Impl::parse(const std::string& str) const
{
  try
  {
    ParserId unused;
    return match_and_parse(str, unused);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor definition for implementation hiding
 */
// ----------------------------------------------------------------------

DateTimeParser::~DateTimeParser() = default;

// ----------------------------------------------------------------------
/*!
 * \brief Constructor initializes Boost Spirit parsers once for reuse speed
 */
// ----------------------------------------------------------------------

DateTimeParser::DateTimeParser() : impl(new DateTimeParser::Impl) {}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a date time in given time zone in the given format
 */
// ----------------------------------------------------------------------

Fmi::LocalDateTime DateTimeParser::parse(const std::string& str,
                                                         const std::string& format,
                                                         Fmi::TimeZonePtr tz) const
{
  try
  {
    DateTime t = parse(str, format);

    // epoch is always in UTC
    if (format == "epoch")
      return Fmi::LocalDateTime(t, tz);

    // timestamps are local
    return make_time(t.date(), t.time_of_day(), tz);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a local date time in the given time zone
 */
// ----------------------------------------------------------------------

Fmi::LocalDateTime DateTimeParser::parse(const std::string& str,
                                                         Fmi::TimeZonePtr tz) const
{
  try
  {
    ParserId matched;

    DateTime t = impl->match_and_parse(str, matched);

    // epoch is always in UTC
    if (matched == EPOCH)
      return Fmi::LocalDateTime(t, tz);

    // timestamps are local
    return make_time(t.date(), t.time_of_day(), tz);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a time in the given format
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::parse(const std::string& str,
                                               const std::string& format) const
{
  try
  {
    if (format == "iso" || format == "xml" || format == "timestamp")
      return impl->parse_iso(str);
    if (format == "sql")
      return impl->parse_sql(str);
    if (format == "epoch")
      return impl->parse_epoch(str);
    if (format == "offset")
      return impl->parse_offset(str);
    if (format == "fmi")
      return impl->parse_fmi(str);

    throw Fmi::Exception(BCP, "Unknown time format '" + format + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Guess time format and parse the time
 */
// ----------------------------------------------------------------------ö

DateTime DateTimeParser::parse(const std::string& str) const
{
  try
  {
    ParserId unused;
    return impl->match_and_parse(str, unused);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a http date
 *
 * From the standard:
 *
 * HTTP applications have historically allowed three different
 * formats for the representation of date/time stamps:
 *
 *   Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
 *   Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
 *   Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
 *
 * The first format is preferred as an Internet standard and represents
 * a fixed-length subset of that defined by RFC 1123 [8] (an update to
 * RFC 822 [9]). The second format is in common use, but is based on the
 * obsolete RFC 850 [12] date format and lacks a four-digit year.
 * HTTP/1.1 clients and servers that parse the date value MUST accept
 * all three formats (for compatibility with HTTP/1.0), though they MUST
 * only generate the RFC 1123 format for representing HTTP-date values
 * in header fields. See section 19.3 for further information.
 *
 * Note that we do not implement full RFC parsers, since in HTTP
 * the dates are always in GMT time.
 */
// ----------------------------------------------------------------------

DateTime DateTimeParser::parse_http(const std::string& str) const
{
  try
  {
    if (str.empty())
      throw Fmi::Exception(BCP, "Empty string is not a HTTP date");

    std::string s = boost::algorithm::replace_all_copy(str, "  ", " ");

    try
    {
      std::vector<std::string> parts;
      boost::algorithm::split(parts, s, boost::algorithm::is_any_of(" "));

      std::uint16_t dd;
      std::uint16_t yy;
      std::uint16_t mm;
      std::uint16_t hh;
      std::uint16_t mi;
      std::uint16_t ss;
      std::string hms;

      switch (parts.size())
      {
        case 6:  // RFC822: Sun, 06 Nov 1994 08:49:37 GMT
        {
          if (!is_short_weekday(parts[0].substr(0, 3)) || parts[0].substr(3, 1) != "," ||
              !is_short_month(parts[2]) || parts[5] != "GMT")
          {
            throw Fmi::Exception(BCP, "");
          }
          dd = std::stoul(parts[1]);
          yy = std::stoul(parts[3]);
          mm = get_short_month(parts[2]);
          hms = parts[4];
          break;
        }
        case 4:  // RFC 850: Sunday, 06-Nov-94 08:49:37 GMT
        {
          if (!is_long_weekday(parts[0].substr(0, parts[0].size() - 1)) ||
              parts[0].substr(parts[0].size() - 1, 1) != "," ||
              !is_short_month(parts[1].substr(3, 3)) || parts[3] != "GMT")
          {
            throw Fmi::Exception(BCP, "");
          }
          dd = std::stoul(parts[1].substr(0, 2));
          yy = std::stoul(parts[1].substr(7, 2));
          yy += (yy < 50 ? 2000 : 1900);
          mm = get_short_month(parts[1].substr(3, 3));
          hms = parts[2];
          break;
        }
        case 5:  // asctime: Sun Nov  6 08:49:37 1994
        {
          if (!is_short_weekday(parts[0]) || !is_short_month(parts[1]))
          {
            throw Fmi::Exception(BCP, "");
          }
          dd = std::stoul(parts[2]);
          yy = std::stoul(parts[4]);
          mm = get_short_month(parts[1]);
          hms = parts[3];
          break;
        }
        default:
          throw Fmi::Exception(BCP, "Invalid HTTP date: " + str);
      }

      hh = std::stoul(hms.substr(0, 2));
      mi = std::stoul(hms.substr(3, 2));
      ss = std::stoul(hms.substr(6, 2));

      DateTime t(Fmi::Date(yy, mm, dd),
                                 Fmi::Hours(hh) + Fmi::Minutes(mi) +
                                     Fmi::Seconds(ss));

      if (t.is_not_a_date_time())
        throw Fmi::Exception(BCP, "");

      return t;
    }
    catch (...)
    {
      throw Fmi::Exception(BCP, "Not a HTTP-date: " + str);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse a time duration
 *
 * Allowed formats include ISO8601 and for historical reasons simple offsets
 *
 *     ISO8601  (P<n>...)
 *     0		(zero offset)
 *     0m,0h... (zero offset with units)
 *     -+NNNN	(offset in minutes)
 *     +-NNNNm	(offset in minutes)
 *     +-NNNNh	(offset in hours)
 *     +-NNNNd	(offset in days)
 *     +-NNNNw	(offset in weeks)
 *     +-NNNNy	(offset in years)
 */
//----------------------------------------------------------------------

TimeDuration DateTimeParser::parse_duration(const std::string& str) const
{
  try
  {
    if (str.empty())
      throw Fmi::Exception(BCP, "Trying to parse an empty string as a time duration");

    auto dura = impl->try_parse_duration(str);
    if (dura.is_not_a_date_time())
      throw Fmi::Exception(BCP, "Failed to parse '" + str + "' as a duration");

    return dura;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse an ISO8601 time duration
 *
 * - https://en.wikipedia.org/wiki/ISO_8601#Durations
 * - https://stackoverflow.com/questions/23886140/parse-iso-8601-durations
 */
//----------------------------------------------------------------------

TimeDuration DateTimeParser::parse_iso_duration(const std::string& str) const
{
  try
  {
    auto dura = try_parse_iso_duration(str);

    if (!dura.is_not_a_date_time())
      return dura;

    throw Fmi::Exception(BCP, "Unable to parse ISO8601 time duration from '" + str + "'");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi

// ======================================================================
