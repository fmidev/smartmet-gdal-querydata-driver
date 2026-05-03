#include "Date.h"
#include "Internal.h"
#include "ParserDefinitions.h"
#include "../Exception.h"
#include "../StringConversion.h"

const Fmi::date_time::Date Fmi::date_time::Date::epoch(1970, 1, 1);

namespace internal = Fmi::date_time::internal;

Fmi::date_time::Date::Date() = default;

Fmi::date_time::Date::Date(const Type& type)
    : Base(type)
{
}

Fmi::date_time::Date::Date(const Date& other) = default;

Fmi::date_time::Date::Date(int year, unsigned month, unsigned day)
    : Base(Date::NORMAL)
{
    date::year year_(year);
    date::month month_(month);
    date::day day_(day);

    date::year_month_day ymd(year_, month_, day_);
    if (! ymd.ok())
    {
        std::ostringstream err;
        err << "Invalid date " << date::format("%Y-%m-%d", ymd);
        if (! ymd.year().ok())
            err << " (year)";
        if (! ymd.month().ok())
            err << " (month)";
        if (! ymd.day().ok())
            err << " (day)";

        throw Fmi::Exception(BCP, err.str());
    }
    date = date::local_days(ymd);
}

Fmi::date_time::Date::Date(const date::local_days& date)
    : Base(Date::NORMAL)
    , date(date)
{
}

Fmi::date_time::Date::~Date() = default;

Fmi::date_time::Date& Fmi::date_time::Date::operator=(const Date& other) = default;

int Fmi::date_time::Date::year() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::year_month_day ymd(date);
    return int(ymd.year());
}

unsigned Fmi::date_time::Date::month() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::year_month_day ymd(date);
    return unsigned(ymd.month());
}

unsigned Fmi::date_time::Date::day() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::year_month_day ymd(date);
    return unsigned(ymd.day());
}

Fmi::date_time::YMD Fmi::date_time::Date::year_month_day() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::year_month_day ymd(date);

    YMD result;
    result.year = int(ymd.year());
    result.month = unsigned(ymd.month());
    result.day = unsigned(ymd.day());
    return result;
}

Fmi::date_time::Weekday Fmi::date_time::Date::day_of_week() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::weekday wd(date);
    return wd;
}

// FIXME: add test for this
std::tm Fmi::date_time::Date::as_tm() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    std::tm tm;
    date::year_month_day ymd(date);
    tm.tm_year = int(ymd.year()) - 1900;
    tm.tm_mon = unsigned(ymd.month()) - 1;
    tm.tm_mday = unsigned(ymd.day());
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_wday = int(day_of_week().iso_encoding() - 1);
    tm.tm_yday = day_of_year() - 1;
    tm.tm_isdst = -1;
    return tm;
}

int Fmi::date_time::Date::day_of_year() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    date::year year_(year());
    date::month month_(1);
    date::day day_(1);
    date::year_month_day ymd(year_, month_, day_);
    return (date - date::local_days(ymd)).count() + 1;
}

Fmi::date_time::Date Fmi::date_time::Date::end_of_month() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    const auto ymd1 = year_month_day();
    date::year year_(ymd1.year);
    date::month month_(ymd1.month);
    date::day day_(1);

    date::year_month ym(year_, month_);
    ym += date::months(1);
    date::year_month_day ymd(ym.year(), ym.month(), day_);
    date::local_days tmp(ymd);
    tmp -= date::days(1);
    return Date(tmp);
}

long Fmi::date_time::Date::modjulian_day() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    return *this - epoch + 40587;
}

long Fmi::date_time::Date::julian_day() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    return *this - epoch + 2440588;
}

int Fmi::date_time::Date::week_number() const
{
    if (is_special())
        throw Fmi::Exception(BCP, "Operation not supported for special values");

    // Code ported from boost (boost/date_time/gregorian/gregorian_calendar.ipp)
    const auto ymd = year_month_day();
    const auto jBegin = Date(ymd.year, 1, 1).julian_day();
    const auto jCurr = julian_day();
    const unsigned long day = (jBegin + 3) % 7;
    const unsigned long week = (jCurr + day - jBegin + 4) / 7;

    if (week >= 1 && week <= 52)
    {
        return static_cast<int>(week);
    }

    if (week == 53)
    {
        if ((day == 6) || ((day == 5) && (date::year(ymd.year).is_leap())))
        {
            return static_cast<int>(week);
        }
        else
        {
            return 1;
        }
    }
    else if (week == 0)
    {
        const auto jBegin = Date(ymd.year - 1, 1, 1).julian_day();
        const auto jCurr = julian_day();
        const unsigned long day = (jBegin + 3) % 7;
        const unsigned long week = (jCurr + day -jBegin + 4) / 7;
        return static_cast<int>(week);
    }
    else
    {
        throw Fmi::Exception(BCP, "INTERNAL ERROR: failed to get"
            " week number for " + date::format("%Y-%m-%d", date));
    }
}

std::string Fmi::date_time::Date::to_simple_string() const
{
    if (is_special())
        return special_time_as_string();

    return format_time("%Y-%b-%d", *this);
}

std::string Fmi::date_time::Date::to_iso_string() const
{
    if (is_special())
        return special_time_as_string();

    return format_time("%Y%m%d", *this);
}

std::string Fmi::date_time::Date::to_iso_extended_string() const
{
    if (is_special())
        return special_time_as_string();

    return format_time("%Y-%m-%d", *this);
}

bool Fmi::date_time::Date::operator==(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator == (other);

    return date == other.date;
}

bool Fmi::date_time::Date::operator!=(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator != (other);

    return date != other.date;
}

bool Fmi::date_time::Date::operator<(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator < (other);

    return date < other.date;
}

bool Fmi::date_time::Date::operator<=(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator <= (other);

    return date <= other.date;
}

bool Fmi::date_time::Date::operator>(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator > (other);

    return date > other.date;
}

bool Fmi::date_time::Date::operator>=(const Date& other) const
{
    if (is_special() || other.is_special())
        return Fmi::date_time::Base::operator <= (other);

    return date >= other.date;
}

Fmi::date_time::Date Fmi::date_time::Date::operator+(int num_days) const
{
    if (is_special())
        return *this;

    date::local_days tmp(date);
    tmp += date::days(num_days);
    return Date(tmp);
}

Fmi::date_time::Date Fmi::date_time::Date::operator-(int num_days) const
{
    if (is_special())
        return *this;

    date::local_days tmp(date);
    tmp -= date::days(num_days);
    return Date(tmp);
}

int Fmi::date_time::Date::operator-(const Date& other) const
{
    if (is_special() || other.is_special())
    {
        Fmi::Exception error(BCP, "Operation not supported for special values");
        error.addParameter("this", to_simple_string());
        error.addParameter("other", other.to_simple_string());
        throw error;
    }
    return (date - other.date).count();
}

Fmi::date_time::Date& Fmi::date_time::Date::operator+=(int num_days)
{
    if (is_special())
        return *this;

    date += date::days(num_days);
    return *this;
}

Fmi::date_time::Date& Fmi::date_time::Date::operator-=(int num_days)
{
    if (is_special())
        return *this;

    date -= date::days(num_days);
    return *this;
}

Fmi::date_time::Date Fmi::date_time::Date::operator ++ (int)
{
    if (is_special())
        return *this;

    date::local_days tmp(date);
    date += date::days(1);
    return Date(tmp);
}

Fmi::date_time::Date Fmi::date_time::Date::operator -- (int)
{
    if (is_special())
        return *this;

    date::local_days tmp(date);
    date -= date::days(1);
    return Date(tmp);
}

Fmi::date_time::Date& Fmi::date_time::Date::operator ++ ()
{
    if (is_special())
        return *this;

    date += date::days(1);
    return *this;
}

Fmi::date_time::Date& Fmi::date_time::Date::operator -- ()
{
    if (is_special())
        return *this;

    date -= date::days(1);
    return *this;
}

Fmi::date_time::Date Fmi::date_time::Date::from_time_t(std::time_t time)
{
    const Fmi::detail::seconds_t seconds(time);
    const Fmi::detail::days_t days = std::chrono::duration_cast<Fmi::detail::days_t>(seconds);
    return epoch + days.count();
}

Fmi::date_time::Date Fmi::date_time::Date::from_iso_string(const std::string& str)
{
    using namespace boost::spirit;
    using iterator = std::string::const_iterator;
    // Date parser: No separator (separator=0), numeric month (true)
    Fmi::date_time::parser::DateParser<iterator, char> parser(0, true);
    Fmi::date_time::parser::date_members_t members;
    iterator begin = str.begin();
    iterator end = str.end();
    if (!qi::parse(begin, end,
            parser >> qi::eoi,
            members))
    {
        auto err = Fmi::Exception::Trace(BCP, "Failed to parse date from string '" + str + "'");
        throw err;
    }
    return Fmi::date_time::Date(members.year, members.month, members.mday);
}

Fmi::date_time::Date Fmi::date_time::Date::from_iso_extended_string(const std::string& str)
{
    using namespace boost::spirit;
    using iterator = std::string::const_iterator;
    // Date parser: Separator is '-' (separator='-'), numeric month (true)
    Fmi::date_time::parser::DateParser<iterator, char> parser('-', true);
    Fmi::date_time::parser::date_members_t members;
    iterator begin = str.begin();
    iterator end = str.end();
    if (!qi::parse(begin, end,
            parser >> qi::eoi,
            members))
    {
        auto err = Fmi::Exception::Trace(BCP, "Failed to parse date from string '" + str + "'");
        err.addParameter("Error position", std::string(begin, end));
        throw err;
    }
    return Fmi::date_time::Date(members.year, members.month, members.mday);
}

Fmi::date_time::Date Fmi::date_time::Date::from_string(const std::string& str)
{
    using namespace boost::spirit;
    using iterator = std::string::const_iterator;
    // Date parser: Separator is '-' (separator='-'), numeric month (true)
    Fmi::date_time::parser::DateParser<iterator, char> parser1('-', true);
    // Date parser: No separator (separator=0), numeric month (true)
    Fmi::date_time::parser::DateParser<iterator, char> parser2(0, true);
    // Date parser: Separator is '-' (separator='-'), alphabetic month (false) - abbrev only supported
    Fmi::date_time::parser::DateParser<iterator, char> parser3('-', false);

    Fmi::date_time::parser::date_members_t members;

    iterator begin = str.begin();
    iterator end = str.end();
    if (!qi::parse(begin, end,
        (parser1 | parser3 | parser2) >> qi::eoi,
        members))
    {
        auto err = Fmi::Exception::Trace(BCP, "Failed to parse date from string '" + str + "'");
        throw err;
    }
    return Fmi::date_time::Date(members.year, members.month, members.mday);
}

Fmi::date_time::Date Fmi::date_time::Date::from_tm(const std::tm& tm)
{
    return Fmi::date_time::Date(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

Fmi::date_time::Date Fmi::date_time::date_from_string(const std::string& str)
{
    return Fmi::date_time::Date::from_string(str);
}
