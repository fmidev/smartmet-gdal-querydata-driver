#include "DateTime.h"
#include "Internal.h"
#include "ParserDefinitions.h"
#include "../Exception.h"
#include "../StringConversion.h"

namespace internal = Fmi::date_time::internal;

#define DEBUG_PARSER_DEFINITIONS 0

namespace
{
    std::string maybe_discard_seconds_part(std::string&& src)
    {
        const std::size_t pos = src.find_last_not_of("0");
        if (pos == std::string::npos) {
            return src;
        } else if (src[pos] == '.') {
            return std::string(src.begin(), src.begin() + pos);
        } else {
            return src;
        }
    }
}

#if DEBUG_PARSER_DEFINITIONS

#define DEBUG(x)
namespace
{
    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::date_members_t& dm)
    {
        os << "(DATE " << dm.year << "-" << dm.month << "-" << dm.mday << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::seconds_members_t& sm)
    {
        os << "(SECONDS " << sm.seconds << "." << sm.frac_sec << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::duration_members_t& dm)
    {
        os << "(DURATION " << dm.hours << ":" << dm.minutes;
        if (dm.seconds) os  << ":" << *dm.seconds;
        os << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::time_zone_offset_members_t& tz)
    {
        os << "(TZ ";
        if (tz.sign == 'Z')
        {
            os << " UTC";
        }
        else
        {
            os << " UTC" << tz.sign << tz.hours << ":" << tz.minutes;
        }
        os << ")";
        return os;
    }

    std::ostream& operator << (std::ostream& os, const Fmi::date_time::parser::date_time_members_t& dt)
    {
        os << "(DATE_TIME " << dt.date;
        if (dt.time) os << " " << *dt.time;
        if (dt.tz_offset) os << " " << *dt.tz_offset;
        os << ")";
        return os;
    }
}

#else // DEBUG_PARSER_DEFINITIONS

#define DEBUG(x)

#endif // !DEBUG_PARSER_DEFINITIONS

const Fmi::date_time::DateTime Fmi::date_time::DateTime::epoch(Fmi::date_time::Date(1970, 1, 1), Fmi::date_time::Seconds(0));

// FIXME: do we need narrower area?
const Fmi::date_time::DateTime Fmi::date_time::DateTime::min = detail::time_point_t::min();
const Fmi::date_time::DateTime Fmi::date_time::DateTime::max = detail::time_point_t::max();

Fmi::date_time::DateTime::DateTime() = default;

Fmi::date_time::DateTime::DateTime(const Type& type)
    : Base(type)
{
}

Fmi::date_time::DateTime::DateTime(const DateTime& other) = default;

Fmi::date_time::DateTime::DateTime(const Date& date)
    : Base(DateTime::NORMAL)
{
    if (date.is_special())
        throw Fmi::Exception(BCP, "Cannot convert special date to DateTime");
    m_time_point = date.get_impl();
}

Fmi::date_time::DateTime::DateTime(const Date& date, const TimeDuration& time)
    : Base(DateTime::NORMAL)
{
    if (date.is_special() || time.is_special())
        throw Fmi::Exception(BCP, "Cannot convert special date or time duration to DateTime");
    m_time_point = date.get_impl() + time.get_impl();
}

Fmi::date_time::DateTime::DateTime(const detail::time_point_t& time_point)
    : Base(DateTime::NORMAL)
    , m_time_point(time_point)
{
}

Fmi::date_time::DateTime::~DateTime() = default;

Fmi::date_time::DateTime& Fmi::date_time::DateTime::operator=(const DateTime& other) = default;


bool Fmi::date_time::DateTime::operator==(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator==(other);

    return m_time_point == other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}


bool Fmi::date_time::DateTime::operator!=(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator!=(other);

    return m_time_point != other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}


bool Fmi::date_time::DateTime::operator<(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator<(other);

    return m_time_point < other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}


bool Fmi::date_time::DateTime::operator<=(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator<=(other);

    return m_time_point <= other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}


bool Fmi::date_time::DateTime::operator>(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator>(other);

    return m_time_point > other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}


bool Fmi::date_time::DateTime::operator>=(const DateTime& other) const
try
{
    if (is_special() || other.is_special())
        return Base::operator>=(other);

    return m_time_point >= other.m_time_point;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}

Fmi::date_time::DateTime& Fmi::date_time::DateTime::operator+=(const TimeDuration& duration)
try
{
    const Type new_type = add_impl(type(), duration.type());

    if (new_type != NORMAL)
    {
        set_type(new_type);
    }
    else
    {
        m_time_point += duration.get_impl();
    }
    return *this;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("duration", duration.to_simple_string());
    throw err;
}

Fmi::date_time::DateTime& Fmi::date_time::DateTime::operator-=(const TimeDuration& duration)
try
{
    const Type new_type = sub_impl(type(), duration.type());

    if (new_type != NORMAL)
    {
        set_type(new_type);
    }
    else
    {
        m_time_point -= duration.get_impl();
    }
    return *this;
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("duration", duration.to_simple_string());
    throw err;
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::operator+(const TimeDuration& duration) const
try
{
    const Type new_type = add_impl(type(), duration.type());

    if (new_type != NORMAL)
    {
        return DateTime(new_type);
    }

    return DateTime(m_time_point + duration.get_impl());
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("duration", duration.to_simple_string());
    throw err;
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::operator-(const TimeDuration& duration) const
try
{
    const Type new_type = sub_impl(type(), duration.type());

    if (new_type != NORMAL)
    {
        return DateTime(new_type);
    }

    return DateTime(m_time_point - duration.get_impl());
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("duration", duration.to_simple_string());
    throw err;
}

Fmi::date_time::TimeDuration Fmi::date_time::DateTime::operator-(const DateTime& other) const
try
{
    const Type new_type = sub_impl(type(), other.type());

    if (new_type != NORMAL)
    {
        return TimeDuration(new_type);
    }

    return TimeDuration(m_time_point - other.m_time_point);
}
catch (...)
{
    auto err = Fmi::Exception::Trace(BCP, "Operation failed");
    err.addParameter("this", to_simple_string());
    err.addParameter("other", other.to_simple_string());
    throw err;
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::operator ++ (int)
try
{
  if (is_special())
    return *this;

  const auto tmp(*this);
  m_time_point += detail::duration_t(1);
  return tmp;
}
catch (...)
{
  auto err = Fmi::Exception::Trace(BCP, "Operation failed");
  err.addParameter("this", to_simple_string());
  throw err;
}

Fmi::date_time::DateTime& Fmi::date_time::DateTime::operator++()
try
{
  if (is_special())
    return *this;

  m_time_point += detail::duration_t(1);
  return *this;
}
catch (...)
{
  auto err = Fmi::Exception::Trace(BCP, "Operation failed");
  err.addParameter("this", to_simple_string());
  throw err;
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::operator--(int)
try
{
  if (is_special())
    return *this;

  const auto tmp(*this);
  m_time_point -= detail::duration_t(1);
  return tmp;
}
catch (...)
{
  auto err = Fmi::Exception::Trace(BCP, "Operation failed");
  err.addParameter("this", to_simple_string());
  throw err;
}

Fmi::date_time::DateTime& Fmi::date_time::DateTime::operator--()
try
{
  if (is_special())
    return *this;

  m_time_point -= detail::duration_t(1);
  return *this;
}
catch (...)
{
  auto err = Fmi::Exception::Trace(BCP, "Operation failed");
  err.addParameter("this", to_simple_string());
  throw err;
}

Fmi::date_time::Date Fmi::date_time::DateTime::date() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Cannot get date from special DateTime");
    return Date(date::floor<detail::days_t>(m_time_point));
}

Fmi::date_time::TimeDuration Fmi::date_time::DateTime::time_of_day() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Cannot get time of day from special DateTime");
    return TimeDuration(m_time_point - date::floor<detail::days_t>(m_time_point));
}

std::time_t Fmi::date_time::DateTime::as_time_t() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Cannot get time of day from special DateTime");
    return std::chrono::duration_cast<Fmi::detail::seconds_t>(m_time_point - epoch.m_time_point).count();
}

struct std::tm Fmi::date_time::DateTime::as_tm() const
{
    const auto date_ = date();
    const auto ymd = date_.year_month_day();
    const auto time = time_of_day();
    struct tm result;
    result.tm_year = ymd.year - 1900;
    result.tm_mon = ymd.month - 1;
    result.tm_mday = ymd.day;
    result.tm_wday = date_.day_of_week().iso_encoding();
    result.tm_yday = date_.day_of_year() - 1;
    result.tm_hour = time.hours();
    result.tm_min = time.minutes();
    result.tm_sec = time.seconds();
    result.tm_isdst = -1;
    return result;
}

std::string Fmi::date_time::DateTime::to_simple_string() const
{
    if (is_special())
        return Base::special_time_as_string();
    return date().to_simple_string() + " " + maybe_discard_seconds_part(time_of_day().to_simple_string());
}

std::string Fmi::date_time::DateTime::to_iso_string() const
{
    if (is_special())
        return Base::special_time_as_string();
    return date().to_iso_string() + "T" + maybe_discard_seconds_part(time_of_day().to_iso_string());
}

std::string Fmi::date_time::DateTime::to_iso_extended_string() const
{
    if (is_special())
        return Base::special_time_as_string();
    return date().to_iso_extended_string() + "T" + maybe_discard_seconds_part(time_of_day().to_iso_extended_string());
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::from_tm(const std::tm& tm)
{
    return DateTime(Date::from_tm(tm), TimeDuration::from_tm(tm));
}

namespace
{
    using namespace boost::spirit::qi;
    using iterator = std::string::const_iterator;

    std::optional<Fmi::date_time::parser::date_time_members_t>
    try_parse_impl(
        const char* filename,     // We are not interested about this location
        int line,                 // in backtrace. Therefore
        const char* function,
        const std::string& str,
        const rule<iterator, Fmi::date_time::parser::date_members_t()>& date_grammar,
        const rule<iterator, Fmi::date_time::parser::duration_members_t()>& time_grammar,
        const rule<iterator> date_time_separator)
    {
        namespace p = boost::phoenix;
        const std::string input = Fmi::trim_copy(str);

        Fmi::date_time::parser::date_time_members_t tmp;

        iterator first = input.begin();
        iterator last = input.end();

        Fmi::date_time::parser::TimeZoneOffsetParser<iterator> p_offset;

#if SUPPORT_DATE_TIME_PARSER_GRAMMAR
        rule<iterator, Fmi::date_time::parser::date_time_members_t()> parser =
            lexeme[
                date_grammar
                >> -(omit[date_time_separator] >> time_grammar >> - p_offset)
            ];

        const bool parse_ok = parse(first, last, parser >> eoi, tmp);
#else
        const bool parse_ok = parse(first, last,
            lexeme[
                date_grammar[p::ref(tmp.date) = _1]
                >> -(omit[date_time_separator]
                    >> time_grammar[p::ref(tmp.time) = _1]
                    >> - p_offset[p::ref(tmp.tz_offset) = _1])
            ] >> eoi);
#endif
        if (!parse_ok)
        {
            return std::nullopt;
        }

        DEBUG(std::cout << "src=" << str;)
        DEBUG(std::cout << ": " << tmp << std::endl;)

        return tmp;
    }

    Fmi::date_time::DateTime as_date_time(
        const char* filename,     // We are not interested about this location
        int line,                 // in backtrace. Therefore
        const char* function,
        const Fmi::date_time::parser::date_time_members_t& tmp,
        bool throw_on_error)
    {
        Fmi::date_time::Date date(
            tmp.date.year,
            tmp.date.month,
            tmp.date.mday);

        Fmi::date_time::TimeDuration time(
            tmp.time ? tmp.time->hours : 0,
            tmp.time ? tmp.time->minutes : 0,
            tmp.time ? tmp.time->get_seconds() : 0,
            tmp.time ? tmp.time->get_mks() : 0);

        Fmi::date_time::DateTime dt(date, time);
        if (tmp.tz_offset)
        {
            // Convert to UTC as we do not support posix time zones
            dt -= Fmi::date_time::Minutes(tmp.tz_offset->get_offset_minutes());
        }
        return dt;
    }

    std::optional<Fmi::date_time::DateTime> try_parse(
        const char* filename,     // We are not interested about this location
        int line,                 // in backtrace. Therefore
        const char* function,
        const std::string& str,
        const rule<iterator, Fmi::date_time::parser::date_members_t()>& date_grammar,
        const rule<iterator, Fmi::date_time::parser::duration_members_t()>& time_grammar,
        const rule<iterator> date_time_separator,
        bool *have_tz)
    {
        const std::string input = Fmi::trim_copy(str);
        const auto members = try_parse_impl(filename, line, function, input, date_grammar, time_grammar, date_time_separator);
        if (!members)
        {
            return std::nullopt;
        }

        try
        {
            const auto result = as_date_time(filename, line, function, *members, true);
            if (have_tz)
                *have_tz = bool(members->tz_offset);
            return result;
        }
        catch (...)
        {
            return std::nullopt;
        }
    }


    Fmi::date_time::DateTime parse(
        const char* filename,     // We are not interested about this location
        int line,                 // in backtrace. Therefore
        const char* function,
        const std::string& str,
        const rule<iterator, Fmi::date_time::parser::date_members_t()>& date_grammar,
        const rule<iterator, Fmi::date_time::parser::duration_members_t()>& time_grammar,
        const rule<iterator> date_time_separator)
    {
        try
        {
            const std::string input = Fmi::trim_copy(str);
            const auto members = try_parse_impl(filename, line, function, input, date_grammar, time_grammar, date_time_separator);
            if (!members)
            {
                auto err = Fmi::Exception::Trace(filename, line, function,
                    "Failed to parse date time from string '" + str + "'");
                throw err;
            }

            const auto result = as_date_time(filename, line, function, *members, true);
            return result;
        }
        catch (...)
        {
            auto err = Fmi::Exception::Trace(filename, line, function,
                "Failed to parse date time from string '" + str + "'");
            throw err;
        }
    }
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::from_string(const std::string& str)
{
    try
    {
        using namespace boost::spirit::qi;
        using namespace Fmi::date_time::parser;
        DateParser<iterator, char> p_date_1('-', false);
        DateParser<iterator, char> p_date_2('-', true);
        DurationParser<iterator, char> p_time(':', false, 24);
        rule<iterator> date_time_separator = +space;
        const auto result = parse(BCP,
            str,
            p_date_2 | p_date_1,
            p_time,
            date_time_separator);
        return result;
    }
    catch (...)
    {
        throw Fmi::Exception(BCP, "Operation failed for string '" + str + "'");
    }
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::from_iso_string(const std::string& str)
{
    try
    {
        using namespace boost::spirit::qi;
        using namespace Fmi::date_time::parser;
        DateParser<iterator, char> p_date(0, true);
        DurationParser<iterator, char> p_time(0, false, 24);
        rule<iterator> date_time_separator = char_('T');
        const auto result = parse(BCP, str, p_date, p_time, date_time_separator);
        return result;
    }
    catch (...)
    {
        throw Fmi::Exception(BCP, "Operation failed for string '" + str + "'");
    }
}

Fmi::date_time::DateTime Fmi::date_time::DateTime::from_iso_extended_string(const std::string& str)
{
    try
    {
        using namespace boost::spirit::qi;
        using namespace Fmi::date_time::parser;
        DateParser<iterator, char> p_date('-', true);
        DurationParser<iterator, char> p_time(':', false, 24);
        rule<iterator> date_time_separator = char_('T');
        const auto result = parse(BCP, str, p_date, p_time, date_time_separator);
        return result;
    }
    catch (...)
    {
        throw Fmi::Exception(BCP, "Operation failed for string '" + str + "'");
    }
}

std::optional<Fmi::date_time::DateTime>
Fmi::date_time::DateTime::try_parse_iso_string(
    const std::string& str,
    bool* have_tz)
{
    using namespace boost::spirit::qi;
    using namespace Fmi::date_time::parser;
    DateParser<iterator, char> p_date(0, true);
    DurationParser<iterator, char> p_time(0, false, 24);
    rule<iterator> date_time_separator = char_('T');
    const auto result = try_parse(BCP, str, p_date, p_time, date_time_separator, have_tz);
    return result;
}

std::optional<Fmi::date_time::DateTime>
Fmi::date_time::DateTime::try_parse_iso_extended_string(
    const std::string& str,
    bool* have_tz)
{
    using namespace boost::spirit::qi;
    using namespace Fmi::date_time::parser;
    DateParser<iterator, char> p_date(':', true);
    DurationParser<iterator, char> p_time('-', false, 24);
    rule<iterator> date_time_separator = char_('T');
    const auto result = try_parse(BCP, str, p_date, p_time, date_time_separator, have_tz);
    return result;
}

std::optional<Fmi::date_time::DateTime>
Fmi::date_time::DateTime::try_parse_string(
    const std::string& str,
    bool *have_tz)
{
    using namespace boost::spirit::qi;
    using namespace Fmi::date_time::parser;
    DateParser<iterator, char> p_date_1('-', false);
    DateParser<iterator, char> p_date_2('-', true);
    DurationParser<iterator, char> p_time(':', false, 24);
    rule<iterator> date_time_separator = +space;
    const std::optional<Fmi::date_time::DateTime> result =
        try_parse(BCP, str, p_date_2 | p_date_1, p_time, date_time_separator, have_tz);
    return result;
}

Fmi::date_time::DateTime
Fmi::date_time::try_parse_iso(const std::string& str, bool* have_tz)
{
    using namespace boost::spirit::qi;
    using namespace Fmi::date_time;
    std::optional<Fmi::date_time::DateTime> result;
    result = DateTime::try_parse_iso_extended_string(str, have_tz);
    if (!result) {
        result = DateTime::try_parse_iso_string(str, have_tz);
    }
    if (result) {
        return *result;
    }

    return Fmi::date_time::DateTime::NOT_A_DATE_TIME;
}

Fmi::date_time::DateTime
Fmi::date_time::parse_iso(const std::string& str)
{
    const auto result = Fmi::date_time::try_parse_iso(str, nullptr);
    if (result.is_special())
        throw Fmi::Exception(BCP, "Invalid ISO time '" + str + "'");
    return result;
}

Fmi::date_time::DateTime Fmi::date_time::from_time_t(long time)
{
    return Fmi::date_time::DateTime(Fmi::date_time::Date::epoch.get_impl() + Fmi::detail::seconds_t(time));
}
