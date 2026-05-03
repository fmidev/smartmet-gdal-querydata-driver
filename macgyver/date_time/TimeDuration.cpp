#include "TimeDuration.h"
#include "Internal.h"
#include "ParserDefinitions.h"
#include "../Exception.h"
#include "../StringConversion.h"
#include <boost/regex.hpp>

namespace internal = Fmi::date_time::internal;

Fmi::date_time::TimeDuration::TimeDuration(const detail::duration_t& duration)
    : Fmi::date_time::Base(Fmi::date_time::Base::NORMAL)
    , m_duration(duration)
{
}

Fmi::date_time::TimeDuration::TimeDuration(int hours, int minutes, int seconds, int microseconds)
    : Fmi::date_time::Base(Fmi::date_time::Base::NORMAL)
    , m_duration(
          detail::hours_t(hours) +
          detail::minutes_t(minutes) +
          detail::seconds_t(seconds) +
          detail::microsec_t(microseconds))
{
}

int64_t Fmi::date_time::TimeDuration::hours() const
{
  assert_special();
  return std::chrono::duration_cast<detail::hours_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::minutes() const
{
  assert_special();
  int64_t minutes = std::chrono::duration_cast<detail::minutes_t>(m_duration).count();
  int64_t hours = minutes / 60;
  return (int)(minutes - hours * 60);
}

int64_t Fmi::date_time::TimeDuration::seconds() const
{
  assert_special();
  int64_t seconds = std::chrono::duration_cast<detail::seconds_t>(m_duration).count();
  int64_t minutes = seconds / 60;
  return (int)(seconds - minutes * 60);
}

int64_t Fmi::date_time::TimeDuration::fractional_seconds() const
{
  int64_t mks = total_microseconds();
  int64_t seconds = mks / 1000000;
  return (int)(mks - seconds * 1000000);
}

int64_t Fmi::date_time::TimeDuration::total_days() const
{
  assert_special();
  return std::chrono::duration_cast<detail::days_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::total_hours() const
{
  assert_special();
  return std::chrono::duration_cast<detail::hours_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::total_minutes() const
{
  assert_special();
  return std::chrono::duration_cast<detail::minutes_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::total_seconds() const
{
  assert_special();
  return std::chrono::duration_cast<detail::seconds_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::total_milliseconds() const
{
  assert_special();
  return std::chrono::duration_cast<detail::millisec_t>(m_duration).count();
}

int64_t Fmi::date_time::TimeDuration::total_microseconds() const
{
  assert_special();
  return std::chrono::duration_cast<detail::microsec_t>(m_duration).count();
}

std::string Fmi::date_time::TimeDuration::to_simple_string() const
{
  const std::string str = format_time("%H:%M:%S", *this);
  const std::size_t pos = str.find_last_of(".,");
  if (pos != std::string::npos && str.substr(pos + 1) == "000000")
    return str.substr(0, pos);

  return str;
}

std::string Fmi::date_time::TimeDuration::to_iso_string() const
{
  const std::string str = format_time("%H%M%S", *this);
  const std::size_t pos = str.find_last_of(".,");
  if (pos != std::string::npos && str.substr(pos + 1) == "000000")
    return str.substr(0, pos);
  return str;
}

std::string Fmi::date_time::TimeDuration::to_iso_extended_string() const
{
  const std::string str = format_time("%H:%M:%S", *this);
  const std::size_t pos = str.find_last_of(".,");
  if (pos != std::string::npos && str.substr(pos + 1) == "000000")
    return str.substr(0, pos);
  return str;
}

void Fmi::date_time::TimeDuration::assert_special() const
{
  if (is_special())
    throw Fmi::Exception(BCP, "Cannot perform operation on special time duration");
}

Fmi::date_time::TimeDuration Fmi::date_time::Days(int days)
{
  return Fmi::date_time::TimeDuration(detail::days_t(days));
}

Fmi::date_time::TimeDuration Fmi::date_time::Hours(int hours)
{
  return Fmi::date_time::TimeDuration(detail::hours_t(hours));
}

Fmi::date_time::TimeDuration Fmi::date_time::Minutes(int minutes)
{
  return Fmi::date_time::TimeDuration(detail::minutes_t(minutes));
}

Fmi::date_time::TimeDuration Fmi::date_time::Seconds(int seconds)
{
  return Fmi::date_time::TimeDuration(detail::seconds_t(seconds));
}

Fmi::date_time::TimeDuration Fmi::date_time::Milliseconds(int milliseconds)
{
  return Fmi::date_time::TimeDuration(detail::millisec_t(milliseconds));
}

Fmi::date_time::TimeDuration Fmi::date_time::Microseconds(int microseconds)
{
  return Fmi::date_time::TimeDuration(detail::microsec_t(microseconds));
}


bool Fmi::date_time::TimeDuration::operator==(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator==(other);

  return m_duration == other.m_duration;
}

bool Fmi::date_time::TimeDuration::operator!=(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator!=(other);

  return m_duration != other.m_duration;
}

bool Fmi::date_time::TimeDuration::operator<(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator<(other);

  return m_duration < other.m_duration;
}

bool Fmi::date_time::TimeDuration::operator<=(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator<=(other);

  return m_duration <= other.m_duration;
}

bool Fmi::date_time::TimeDuration::operator>(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator>(other);

  return m_duration > other.m_duration;
}

bool Fmi::date_time::TimeDuration::operator>=(const TimeDuration& other) const
{
  if (is_special() || other.is_special())
    return Base::operator>=(other);

  return m_duration >= other.m_duration;
}

Fmi::date_time::TimeDuration& Fmi::date_time::TimeDuration::operator+=(const TimeDuration& other)
{
  if (is_special() && other.is_special())
  {
    *this = TimeDuration(NOT_A_DATE_TIME);
  }
  else
  {
    m_duration += other.m_duration;
  }
  return *this;
}

Fmi::date_time::TimeDuration& Fmi::date_time::TimeDuration::operator-=(const TimeDuration& other)
{
  if (is_special() && other.is_special())
  {
    *this = TimeDuration(NOT_A_DATE_TIME);
  }
  else
  {
    m_duration -= other.m_duration;
  }
  return *this;
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator+(const TimeDuration& other) const
{
  if (is_special() && other.is_special())
    return TimeDuration(NOT_A_DATE_TIME);

  return TimeDuration(m_duration + other.m_duration);
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator-(const TimeDuration& other) const
{
  if (is_special() && other.is_special())
    return TimeDuration(NOT_A_DATE_TIME);

  return TimeDuration(m_duration - other.m_duration);
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator*(int64_t factor) const
{
  if (is_special())
    return *this;

  return TimeDuration(m_duration * factor);
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator/(int64_t factor) const
{
  if (is_special())
    return *this;

  return TimeDuration(m_duration / factor);
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator++(int)
{
  if (is_special())
    return *this;

  TimeDuration tmp(*this);
  m_duration ++;
  return tmp;
}

Fmi::date_time::TimeDuration& Fmi::date_time::TimeDuration::operator++()
{
  if (!is_special())
    m_duration ++;
  return *this;
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::operator--(int)
{
  if (is_special())
    return *this;

  TimeDuration tmp(*this);
  m_duration --;
  return tmp;
}

Fmi::date_time::TimeDuration& Fmi::date_time::TimeDuration::operator--()
{
  if (!is_special())
    m_duration --;
  return *this;
}

Fmi::date_time::TimeDuration
Fmi::date_time::TimeDuration::from_stream(std::istream& is, bool assume_eoi)
{
  detail::duration_t td1;
  try
  {
    const internal::StreamExceptionState save(is, std::ios::failbit | std::ios::badbit);
    is >> date::parse("%H:%M", td1);
    if (!is.eof() && is.peek() == ':')
    {
      detail::duration_t td2;
      is >> date::parse(":%S", td2);
      td1 += td2;
      // Skip any remaining digits of second part if present
      while (!is.eof() && std::isdigit(is.peek()))
        is.get();
    }
  }
  catch (...)
  {
    auto err = Fmi::Exception::Trace(BCP, "Failed to parse time duration from string");
    throw err;
  }

  Fmi::date_time::internal::check_parse_status(is, assume_eoi, "time duration");

  return td1;
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::from_tm(const std::tm& tm)
{
  return Fmi::date_time::TimeDuration(tm.tm_hour, tm.tm_min, tm.tm_sec, 0);
}

Fmi::date_time::TimeDuration
Fmi::date_time::TimeDuration::from_iso_string(
    const std::string& str,
    bool supports_negative,
    unsigned max_hours)
{
  using namespace boost::spirit::qi;
  using iterator = std::string::const_iterator;

  const std::string input = Fmi::trim_copy(str);
  iterator begin = input.begin();
  iterator end = input.end();

  Fmi::date_time::parser::duration_members_t members;
  Fmi::date_time::parser::DurationParser<iterator, char> parser(0, supports_negative, max_hours);
  bool r = parse(begin, end, parser >> eoi, members);
  if (!r)
  {
    auto err = Fmi::Exception::Trace(BCP, "Failed to parse time duration from string '" + str + "'");
    err.addParameter("Error position", "'" + std::string(begin, end) + "'");
    throw err;
  }

  int hours = members.get_hours();
  int minutes = members.get_minutes();
  int seconds = members.get_seconds();
  int microseconds = members.get_mks();
  return Fmi::date_time::TimeDuration(hours, minutes, seconds, microseconds);
}

Fmi::date_time::TimeDuration Fmi::date_time::TimeDuration::from_iso_extended_string(
    const std::string& str,
    bool supports_negative,
    unsigned max_hours)
{
  using namespace boost::spirit::qi;
  using iterator = std::string::const_iterator;

  const std::string input = Fmi::trim_copy(str);
  iterator begin = input.begin();
  iterator end = input.end();

  Fmi::date_time::parser::duration_members_t members;
  Fmi::date_time::parser::DurationParser<iterator, char> parser(':', supports_negative, max_hours);
  bool r = parse(begin, end, parser >> eoi, members);
  if (!r)
  {
    auto err = Fmi::Exception::Trace(BCP, "Failed to parse time duration from string '" + str + "'");
    err.addParameter("Error position", "'" + std::string(begin, end) + "'");
    throw err;
  }

  int hours = members.get_hours();
  int minutes = members.get_minutes();
  int seconds = members.get_seconds();
  int microseconds = members.get_mks();
  return Fmi::date_time::TimeDuration(hours, minutes, seconds, microseconds);
}

Fmi::date_time::TimeDuration
Fmi::date_time::TimeDuration::from_string(const std::string& str, bool supports_negative, unsigned max_hours)
{
  using namespace boost::spirit::qi;
  using iterator = std::string::const_iterator;

  const std::string input = Fmi::trim_copy(str);
  iterator begin = input.begin();
  iterator end = input.end();

  Fmi::date_time::parser::duration_members_t members;
  Fmi::date_time::parser::GenericDurationParser<iterator> parser(supports_negative, max_hours);
  bool r = parse(begin, end, parser >> eoi, members);
  if (!r)
  {
    auto err = Fmi::Exception::Trace(BCP, "Failed to parse time duration from string '" + str + "'");
    err.addParameter("Error position", "'" + std::string(begin, end) + "'");
    throw err;
  }

  int hours = members.get_hours();
  int minutes = members.get_minutes();
  int seconds = members.get_seconds();
  int microseconds = members.get_mks();
  return Fmi::date_time::TimeDuration(hours, minutes, seconds, microseconds);
}

struct std::tm Fmi::date_time::to_tm(const date_time::TimeDuration& t)
{
  if (t.is_special())
    throw Fmi::Exception(BCP, "Cannot convert special time to tm");

  struct std::tm result;
  result.tm_year = 0;
  result.tm_mon = 0;
  result.tm_mday = 0;
  result.tm_hour = t.hours();
  result.tm_min = t.minutes();
  result.tm_sec = t.seconds();
  result.tm_isdst = -1;
  return result;
}

Fmi::date_time::TimeDuration Fmi::date_time::duration_from_string(const std::string& str)
{
  return Fmi::date_time::TimeDuration::from_string(str);
}
