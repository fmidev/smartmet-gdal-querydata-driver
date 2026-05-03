#include "LocalDateTime.h"
#include "TimeZonePtr.h"
#include "Internal.h"
#include "../Exception.h"

using Fmi::date_time::Date;
using Fmi::date_time::DateTime;
using Fmi::date_time::TimeDuration;
using Fmi::date_time::TimeZonePtr;
using Fmi::date_time::LocalDateTime;

namespace detail = Fmi::detail;

LocalDateTime::LocalDateTime() noexcept
    : Base()
{
}

LocalDateTime::LocalDateTime(Type type) noexcept
    : Base(type)
{
}

LocalDateTime::LocalDateTime(const LocalDateTime& src) noexcept = default;

LocalDateTime::LocalDateTime(const detail::zoned_time_t& zoned_time) noexcept
    : Base(NORMAL)
    , ldt(zoned_time)
{
}

LocalDateTime::LocalDateTime(
    const DateTime& time,
    const TimeZonePtr& tz,
    enum ErrorHandling err_handling)

    : Base(NORMAL)
{
    try
    {
        if (time.is_special())
        {
            set_type(time.type());
            return;
        }

        if (!tz) {
            ldt = detail::zoned_time_t(
                TimeZonePtr::utc.zone_ptr(),
                time.get_impl());
            set_type(NORMAL);
            return;
        }

        try
        {
            const auto ldt_utc  = detail::zoned_time_t(
                TimeZonePtr::utc.zone_ptr(),
                detail::time_point_t(time.get_impl()) );
            const auto sys_time = ldt_utc.get_sys_time();
            ldt = detail::zoned_time_t(tz.zone_ptr(), sys_time);
            set_type(NORMAL);
        }
        catch(const std::exception& e)
        {
            if (err_handling == EXCEPTION_ON_ERROR)
                throw;

            set_type(NOT_A_DATE_TIME);
        }
    }
    catch (...)
    {
        auto err = Fmi::Exception::Trace(BCP, "Failed to construct Fmi::date_time::LocalDateTime");
        err.addParameter("Time", time.to_simple_string());
        err.addParameter("TimeZone", tz->name());
        throw err;
    }
}

LocalDateTime::LocalDateTime(
    const Date& date,
    const TimeDuration& time,
    const TimeZonePtr& tz,
    enum ErrorHandling err_handling,
    enum Choose choose)

    : Base()
{
    try
    {
        if (date.is_special())
        {
            set_type(date.type());
            return;
        }

        if (time.is_special())
        {
            set_type(time.type());
            return;
        }

        if (!tz)
        {
            ldt = detail::zoned_time_t(
                TimeZonePtr::utc.zone_ptr(),
                detail::time_point_t(date.get_impl() + time.get_impl()) );
            set_type(NORMAL);
            return;
        }

        try
        {
            ldt = make_zoned_time(date.get_impl() + time.get_impl(), tz.zone_ptr(), choose);
            set_type(NORMAL);
        }
        catch(...)
        {
            if (err_handling == EXCEPTION_ON_ERROR)
                throw;

            set_type(NOT_A_DATE_TIME);
        }
    }
    catch (...)
    {
        auto err = Fmi::Exception::Trace(BCP, "Failed to construct Fmi::date_time::LocalDateTime");
        err.addParameter("Date", date.to_simple_string());
        err.addParameter("Time", time.to_simple_string());
        err.addParameter("TimeZone", tz->name());
        throw err;
    }
}

LocalDateTime::LocalDateTime(
    const detail::time_point_t& time,
    const date::time_zone* tz,
    enum ErrorHandling err_handling)

    : Base(NORMAL)
{
    if (!tz)
    {
        ldt = detail::zoned_time_t(
            TimeZonePtr::utc.zone_ptr(),
            time);
        set_type(NORMAL);
        return;
    }

    try
    {
        ldt = detail::zoned_time_t(tz, time);
        set_type(NORMAL);
    }
    catch (...)
    {
        if (err_handling == EXCEPTION_ON_ERROR)
            throw Fmi::Exception::Trace(BCP, "Operation failed!");

        set_type(NOT_A_DATE_TIME);
    }
}

LocalDateTime::~LocalDateTime() = default;

LocalDateTime& LocalDateTime::operator = (const LocalDateTime& src) = default;

const Fmi::detail::zoned_time_t& LocalDateTime::get_impl() const
{
    check_no_special(BCP);
    return ldt;
}

void LocalDateTime::check_no_special(
    const char* _filename,
    int _line,
    const char* _function) const
{
    if (is_special())
        throw Fmi::Exception(_filename, _line, _function, "Not supported for special LocalDateTime values");
}

Date LocalDateTime::date() const
{
    if (is_special())
        return Date(type());

    return utc_time().date();
}

DateTime LocalDateTime::local_time() const
{
    if (is_special())
        return DateTime(type());

    return DateTime(ldt.get_local_time());
}

TimeZonePtr LocalDateTime::zone() const
{
    check_no_special(BCP);
    return TimeZonePtr(ldt.get_time_zone());
}

TimeDuration LocalDateTime::time_of_day() const
{
    return utc_time().time_of_day();
}

DateTime LocalDateTime::utc_time() const
{
    if (is_special())
        return DateTime(type());

    TimeDuration diff(get_sys_info().offset);
    return local_time() - diff;
}

LocalDateTime LocalDateTime::to_tz(const TimeZonePtr& zone) const
{
    check_no_special(BCP);
    const DateTime utc_time_ = this->utc_time();
    return LocalDateTime(utc_time_, zone);
}

bool LocalDateTime::dst_on() const
{
    check_no_special(BCP);
    return get_sys_info().save != std::chrono::seconds(0);
}

TimeDuration LocalDateTime::offset() const
{
    check_no_special(BCP);
    const auto sys_info = get_sys_info();
    const detail::seconds_t off = sys_info.offset;
    return std::chrono::duration_cast<Fmi::detail::duration_t>(off);
}

#if !USE_OS_TZDB
TimeDuration LocalDateTime::dst_offset() const
{
    check_no_special(BCP);
    const auto sys_info = get_sys_info();
    const detail::seconds_t save = sys_info.save;
    return std::chrono::duration_cast<Fmi::detail::duration_t>(save);
}
#endif

std::string LocalDateTime::abbrev() const
{
    check_no_special(BCP);
    return get_sys_info().abbrev;
}

date::sys_info LocalDateTime::get_sys_info() const
{
    check_no_special(BCP);
    return ldt.get_info();
}

std::pair<detail::sys_time_t, detail::sys_time_t>
LocalDateTime::get_dst_times() const
{
    std::pair<detail::sys_time_t, detail::sys_time_t> result;
    auto info = get_sys_info();
    if (info.save == std::chrono::seconds(0))
    {
        const auto next = info.end + std::chrono::seconds(2);
        info = get_impl().get_time_zone()->get_info(next);
        const detail::sys_time_t begin = info.begin;
        const detail::sys_time_t end = info.end;
        result = std::make_pair(begin, end);
    }
    else
    {
        const detail::sys_time_t begin = info.begin;
        const detail::sys_time_t end = info.end;
        result = std::make_pair(begin, end);
    }
    return result;
}

std::string LocalDateTime::to_simple_string() const
{
    switch (type())
    {
        case Type::NORMAL:
            return internal::remove_trailing_zeros(local_time().to_simple_string())
                    + " " + abbrev();

        case Type::NOT_A_DATE_TIME:
            return "not-a-date-time";

        case Type::NEG_INFINITY:
            return "NEG_INFINITY";

        case Type::POS_INFINITY:
            return "POS_INFINITY";

        default:
            throw Fmi::Exception(BCP, "INTERNAL ERROR: Invalid LocalDateTime type");
    }
}

std::string LocalDateTime::to_iso_string() const
{
    check_no_special(BCP);
    const std::string s_dt = local_time().to_iso_string();
    const std::string s_zone = zone().zone_ptr() == TimeZonePtr::utc.zone_ptr()
        ? "Z"
        : date::format("%z", ldt);
    return s_dt + s_zone;
}

std::string LocalDateTime::to_iso_extended_string() const
{
    check_no_special(BCP);
    const std::string s_dt = local_time().to_iso_extended_string();
    const std::string s_zone = zone().zone_ptr() == TimeZonePtr::utc.zone_ptr()
        ? "Z"
        : date::format("%Ez", ldt);
    return s_dt + s_zone;
}

void LocalDateTime::advance(const TimeDuration& td)
try
{
    const Type new_type = add_impl(type(), td.type());
    if (new_type != NORMAL)
    {
        set_type(new_type);
        return;
    }

    const auto mks = td.total_microseconds();
    const detail::duration_t adv =
        std::chrono::duration_cast<detail::duration_t>(
            std::chrono::microseconds(mks));
    const date::time_zone* tz = get_impl().get_time_zone();
    const auto& impl = get_impl();
    const auto sys_pt = impl.get_sys_time();
    const auto new_sys_pt = sys_pt + adv;
    const detail::zoned_time_t new_local_pt(tz, new_sys_pt);
    ldt = detail::zoned_time_t(tz, new_local_pt);
}
catch (...)
{
    throw std::move(Fmi::Exception::Trace(BCP, "Operation failed!")
        .addParameter("this", to_simple_string())
        .addParameter("advance", td.to_simple_string()));
}

bool LocalDateTime::operator == (const LocalDateTime& other) const
try
{
    return compare_with(other) == 0;
}
catch (const std::exception& e)
{
    throw std::move(Fmi::Exception::Trace(BCP, e.what())
        .addParameter("This", to_simple_string())
        .addParameter("Other", other.to_simple_string()));
}

bool LocalDateTime::operator != (const LocalDateTime& other) const
try
{
    return compare_with(other) != 0;
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Operation failed");
}

bool LocalDateTime::operator < (const LocalDateTime& other) const
try
{
    return compare_with(other) < 0;
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Operation failed");
}

bool LocalDateTime::operator > (const LocalDateTime& other) const
try
{
    return compare_with(other) > 0;
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Operation failed");
}

bool LocalDateTime::operator <= (const LocalDateTime& other) const
try
{
    return compare_with(other) <= 0;
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Operation failed");
}

bool LocalDateTime::operator >= (const LocalDateTime& other) const
try
{
    return compare_with(other) >= 0;
}
catch (const std::exception& e)
{
    throw Fmi::Exception::Trace(BCP, "Operation failed");
}

int LocalDateTime::compare_with(const LocalDateTime& other) const
{
    if (is_not_a_date_time() || other.is_not_a_date_time())
        throw
            std::move(
                Fmi::Exception(BCP, "Operation not supported for NOT_A_DATE_TIME")
                .addParameter("This", to_simple_string())
                .addParameter("Other", other.to_simple_string()));

    if (is_special() || other.is_special())
    {
        if (type() == other.type())
            return 0;

        return type() < other.type() ? -1 : 1;
    }

    const auto sys_time = get_impl().get_sys_time();
    const auto other_sys_time = other.get_impl().get_sys_time();
    if (sys_time == other_sys_time)
        return 0;

    return sys_time < other_sys_time ? -1 : 1;
}

LocalDateTime Fmi::date_time::operator + (const LocalDateTime& time, const TimeDuration& td)
{
    LocalDateTime tmp(time);
    tmp.advance(td);
    return tmp;
}

LocalDateTime Fmi::date_time::operator - (const LocalDateTime& time, const TimeDuration& td)
{
    LocalDateTime tmp(time);
    tmp.advance(-td);
    return tmp;
}

TimeDuration Fmi::date_time::operator - (const LocalDateTime& to, const LocalDateTime& from)
{
    if (from.is_special() || to.is_special())
    {
        return TimeDuration();
    }

    const auto sys_from = from.get_impl().get_sys_time();
    const auto sys_to = to.get_impl().get_sys_time();
    return Fmi::date_time::TimeDuration(sys_to - sys_from);
}


LocalDateTime Fmi::date_time::LocalDateTime::operator ++ (int)
{
    LocalDateTime tmp(*this);
    advance(TimeDuration(detail::duration_t(1)));
    return tmp;
}

LocalDateTime& Fmi::date_time::LocalDateTime::operator ++ ()
{
    advance(TimeDuration(detail::duration_t(1)));
    return *this;
}

LocalDateTime Fmi::date_time::LocalDateTime::operator -- (int)
{
    LocalDateTime tmp(*this);
    advance(TimeDuration(detail::duration_t(-1)));
    return tmp;
}

LocalDateTime& Fmi::date_time::LocalDateTime::operator -- ()
{
    advance(TimeDuration(detail::duration_t(-1)));
    return *this;
}

Fmi::detail::zoned_time_t
Fmi::date_time::LocalDateTime::make_zoned_time(
    const detail::time_point_t& time,
    const date::time_zone* tz,
    enum Fmi::date_time::LocalDateTime::Choose choose)
{
    switch (choose)
    {
        case Choose::EARLIEST:
            return Fmi::detail::zoned_time_t(tz, time, date::choose::earliest);

        case Choose::LATEST:
            return detail::zoned_time_t(tz, time, date::choose::latest);
            break;

        case Choose::AUTO:
            try {
                return detail::zoned_time_t(tz, time);
            }
            catch (const date::ambiguous_local_time&)
            {
                return detail::zoned_time_t(tz, time, date::choose::latest);
            }
            catch (const date::nonexistent_local_time&)
            {
                return detail::zoned_time_t(tz, time, date::choose::earliest);
            }

        default:
            return detail::zoned_time_t(tz, time);
    }
}

Fmi::date_time::DateTime Fmi::date_time::MicrosecClock::universal_time()
{
    const auto tmp =
        date::clock_time_conversion<date::local_t, date::utc_clock>()
            (date::utc_clock::now());
    return std::chrono::time_point_cast<Fmi::detail::microsec_t>(tmp);
}

Fmi::date_time::DateTime Fmi::date_time::MicrosecClock::local_time()
{
    TimeZonePtr tz(date::current_zone());
    const LocalDateTime utc_time(Fmi::date_time::MicrosecClock::universal_time(), TimeZonePtr::utc);
    return utc_time.to_tz(tz).local_time();
}

Fmi::date_time::DateTime Fmi::date_time::SecondClock::universal_time()
{
    const auto tmp =
        date::clock_time_conversion<date::local_t, date::utc_clock>()
            (date::utc_clock::now());
    return std::chrono::time_point_cast<Fmi::detail::microsec_t>(
        std::chrono::time_point_cast<Fmi::detail::seconds_t>(tmp));
}

Fmi::date_time::DateTime Fmi::date_time::SecondClock::local_time()
{
    TimeZonePtr tz(date::current_zone());
    const LocalDateTime utc_time(Fmi::date_time::SecondClock::universal_time(), TimeZonePtr::utc);
    return utc_time.to_tz(tz).local_time();
}

Fmi::date_time::LocalDateTime
Fmi::date_time::make_time(
    const Fmi::date_time::Date& date,
    const Fmi::date_time::TimeDuration& time,
    const Fmi::date_time::TimeZonePtr& tz)
{
    try
    {
#if 1
        return LocalDateTime(date, time, tz,
            LocalDateTime::NOT_DATE_TIME_ON_ERROR,
            LocalDateTime::Choose::EARLIEST);
#else
        detail::time_point_t tp = date.get_impl() + time.get_impl();
        try
        {
            detail::zoned_time_t ldt(tz.zone_ptr(), tp);
            return LocalDateTime(ldt);
        }
        catch (const date::ambiguous_local_time&)
        {
            // We have ambigous local time - preffer sommertime
            detail::zoned_time_t ldt(tz.zone_ptr(), tp, date::choose::earliest);
            return LocalDateTime(ldt);
        }
        catch (const date::nonexistent_local_time&)
        {
            // We have non-existent local time - preffer standard time
            detail::zoned_time_t ldt(tz.zone_ptr(), tp, date::choose::earliest);
            return LocalDateTime(ldt);
        }
#endif
    }
    catch (...)
    {
        return LocalDateTime(LocalDateTime::NOT_A_DATE_TIME);
    }
}
