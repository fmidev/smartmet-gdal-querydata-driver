// ======================================================================
/*!
 * \brief Parse timestamps
 */
// ======================================================================

#include "TimeParser.h"
#include "Exception.h"
#include "TimeParserDefinitions.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include <cctype>
#include <stdexcept>

static Fmi::DateTime bad_time;
static Fmi::TimeDuration bad_duration;

namespace
{
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

void check_hms(unsigned hour, unsigned minute, unsigned second)
{
  if (hour > 23)
    throw Fmi::Exception(BCP, "Invalid hour in time");
  if (minute > 59)
    throw Fmi::Exception(BCP, "Invalid minute in time");
  if (second > 59)
    throw Fmi::Exception(BCP, "Invalid second in time");
}

Fmi::DateTime buildFromSQL(const Fmi::TimeParser::TimeStamp& target)
{
  try
  {
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    if (target.hour)
      hour = *target.hour;
    if (target.minute)
      minute = *target.minute;
    if (target.second)
      second = *target.second;

    Fmi::DateTime ret;

    // Fmi::Date constructor checks the validity of the date - we do not need
    // to do it here
    check_hms(hour, minute, second);

    // Translate the exception to runtime_error
    try
    {
      ret = Fmi::DateTime(Fmi::Date(target.year, target.month, target.day),
                                     Fmi::Hours(hour) +
                                         Fmi::Minutes(minute) +
                                         Fmi::Seconds(second));
    }
    catch (std::exception& err)
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

Fmi::DateTime buildFromISO(const Fmi::TimeParser::TimeStamp& target)
{
  try
  {
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;

    if (target.hour)
      hour = *target.hour;
    if (target.minute)
      minute = *target.minute;
    if (target.second)
      second = *target.second;

    Fmi::DateTime res;

    // Fmi::Date constructor checks the validity of the date - we do not need
    // to do it here
    check_hms(hour, minute, second);

    try
    {
      res = Fmi::DateTime(Fmi::Date(target.year, target.month, target.day),
                                     Fmi::Hours(hour) +
                                         Fmi::Minutes(minute) +
                                         Fmi::Seconds(second));
    }
    catch (std::exception& err)
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

Fmi::DateTime buildFromEpoch(const Fmi::TimeParser::UnixTime& target)
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

}  // namespace

namespace Fmi
{
namespace TimeParser
{
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

bool parse_uint16(const char** str, unsigned int length, std::uint16_t* value)
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
    using iterator = std::string::const_iterator;
    SQLParser<iterator> theParser;
    return qi::parse(t.begin(), t.end(), theParser);
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
 * \brief Guess the input format
 */
// ----------------------------------------------------------------------

std::string looks(const std::string& str)
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

bool looks_utc(const std::string& str)
{
  try
  {
    if (looks_sql(str))
      return false;

    if (looks_offset(str))  // offsets are always relative to the time now
      return true;

    bool utc;
    Fmi::DateTime t = try_parse_iso(str, &utc);
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
 * \brief Parse epoch time
 */
// ----------------------------------------------------------------------

Fmi::DateTime parse_epoch(const std::string& str)
{
  try
  {
    using iterator = std::string::const_iterator;

    EpochParser theParser;
    UnixTime target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser >> qi::eoi, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return ::buildFromEpoch(target);

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

Fmi::DateTime try_parse_offset(const std::string& str)
{
  try
  {
    if (str.empty())
      return bad_time;

    auto offset = try_parse_duration(str);
    if (offset.is_not_a_date_time())
      return bad_time;

    return ::buildFromOffset(offset);
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
    using iterator = std::string::const_iterator;
    ISOParser<iterator> theParser;
    TimeStamp target;
    Fmi::DateTime t = bad_time;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser, target);

    if (success)
    {
      try
      {
        t = buildFromISO(target);
        *isutc = target.tz.present;
      }
      catch (...)
      {
        // We do not want to get exception thrown on invalid input
      }
    }
    else
    {
      *isutc = true;
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
 * \brief Parse ISO time format
 */
// ----------------------------------------------------------------------

Fmi::DateTime parse_iso(const std::string& str)
{
  try
  {
    using iterator = std::string::const_iterator;
    ISOParser<iterator> theParser;
    TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return ::buildFromISO(target);

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

Fmi::DateTime parse_fmi(const std::string& str)
{
  try
  {
    using iterator = std::string::const_iterator;
    FMIParser<iterator> theParser;
    TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return ::buildFromISO(target);

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

Fmi::DateTime parse_sql(const std::string& str)
{
  try
  {
    using iterator = std::string::const_iterator;
    SQLParser<iterator> theParser;
    TimeStamp target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser, target);

    if (success)  // parse succesful, parsers check that entire input was consumed
      return ::buildFromSQL(target);

    throw Fmi::Exception(BCP, "Invalid SQL-time: '" + str + "'");
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

Fmi::DateTime parse_offset(const std::string& str)
{
  try
  {
    if (str.empty())
      throw Fmi::Exception(BCP, "Trying to parse an empty string as a time offset");

    TimeDuration offset = parse_duration(str);

    return ::buildFromOffset(offset);
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

TimeDuration parse_duration(const std::string& str)
{
  try
  {
    if (str.empty())
      throw Fmi::Exception(BCP, "Trying to parse an empty string as a time duration");

    auto dura = try_parse_duration(str);
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

TimeDuration try_parse_duration(const std::string& str)
{
  try
  {
    if (str.empty())
      return bad_duration;

    if (str[0] == 'P')
      return try_parse_iso_duration(str);

    // Old time duration format

    using iterator = std::string::const_iterator;

    OffsetParser<iterator> theParser;
    TimeOffset target;

    iterator start = str.begin();
    iterator finish = str.end();

    bool success = qi::parse(start, finish, theParser, target);

    if (!success)
      return bad_duration;

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

    return bad_duration;
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

TimeDuration parse_iso_duration(const std::string& str)
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

// ----------------------------------------------------------------------
/*!
 * \brief Parse an ISO8601 time duration
 *
 * - https://en.wikipedia.org/wiki/ISO_8601#Durations
 * - https://stackoverflow.com/questions/23886140/parse-iso-8601-durations
 */
//----------------------------------------------------------------------

TimeDuration try_parse_iso_duration(const std::string& str)
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
      return bad_duration;

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
      return bad_duration;
    if (vec[4] < 0 || vec[4] > 24)
      return bad_duration;

    // Year length 365 and month length 30 are arbitrary choices here

    return Fmi::Hours(365 * 24 * vec[0] + 30 * 24 * vec[1] + 24 * vec[2]) +
           TimeDuration(vec[4], vec[5], vec[6], 0);
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
Fmi::DateTime match_and_parse(const std::string& str, ParserId& matchedParser)
{
  try
  {
    using iterator = std::string::const_iterator;

    {
      FMIParser<iterator> theParser;
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, theParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          Fmi::DateTime ret;
          ret = ::buildFromISO(target);  // Similar building as with ISO parser
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
      ISOParser<iterator> theParser;
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, theParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          Fmi::DateTime ret;
          ret = ::buildFromISO(target);
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
      SQLParser<iterator> theParser;
      TimeStamp target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, theParser, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          Fmi::DateTime ret;
          ret = ::buildFromSQL(target);
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
      EpochParser theParser;
      UnixTime target;

      iterator start = str.begin();
      iterator finish = str.end();
      bool success = qi::parse(start, finish, theParser >> qi::eoi, target);
      if (success)  // parse succesful, parsers check that entire input was consumed
      {
        try
        {
          Fmi::DateTime ret;
          ret = ::buildFromEpoch(target);
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
 * \brief Parse a time in the given format
 */
// ----------------------------------------------------------------------

Fmi::DateTime parse(const std::string& str, const std::string& format)
{
  try
  {
    if (format == "iso" || format == "xml" || format == "timestamp")
      return parse_iso(str);
    if (format == "sql")
      return parse_sql(str);
    if (format == "epoch")
      return parse_epoch(str);
    if (format == "offset")
      return parse_offset(str);
    if (format == "fmi")
      return parse_fmi(str);

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
// ----------------------------------------------------------------------�

Fmi::DateTime parse(const std::string& str)
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
 * \brief Parse a time in the given format
 */
// ----------------------------------------------------------------------

Fmi::LocalDateTime parse(const std::string& str,
                                         const std::string& format,
                                         Fmi::TimeZonePtr tz)
{
  try
  {
    Fmi::DateTime t = parse(str, format);

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
 * \brief Parse a time
 */
// ----------------------------------------------------------------------

Fmi::LocalDateTime parse(const std::string& str,
                                         Fmi::TimeZonePtr tz)
{
  try
  {
    ParserId matched;

    Fmi::DateTime t = match_and_parse(str, matched);

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

Fmi::DateTime parse_http(const std::string& str)
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

      std::uint16_t dd, yy, mm;
      std::uint16_t hh, mi, ss;
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

      Fmi::DateTime t(Fmi::Date(yy, mm, dd),
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

}  // namespace TimeParser
}  // namespace Fmi

// ======================================================================
