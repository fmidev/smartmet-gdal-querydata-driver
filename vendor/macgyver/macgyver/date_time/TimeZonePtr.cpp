#include "TimeZonePtr.h"

#include "../Exception.h"
#include "../StringConversion.h"

using namespace Fmi::date_time;

TimeZonePtr TimeZonePtr::utc("UTC");

TimeZonePtr::TimeZonePtr() noexcept
    : tz(utc.zone_ptr())
{
}

TimeZonePtr::TimeZonePtr(const std::string& name)
    : tz(date::locate_zone(name))
{
    if (!tz)
        throw Fmi::Exception(BCP, "Time zone '" + name + "' not found");
}

TimeZonePtr::TimeZonePtr(const date::time_zone* tz) noexcept
    : tz(tz)
{
}

TimeZonePtr::TimeZonePtr(const TimeZonePtr& src) noexcept = default;

TimeZonePtr::~TimeZonePtr() = default;

TimeZonePtr& Fmi::date_time::TimeZonePtr::operator = (const TimeZonePtr& src) noexcept = default;

const date::time_zone* Fmi::date_time::TimeZonePtr::zone_ptr() const
{
    return tz;
}

bool TimeZonePtr::is_utc() const
{
    if (!tz)
        throw Fmi::Exception(BCP, "Time zone not set");

    // I wanted to use offset, but it seems that there is no easy
    // way to see whether zone has also DST.
    // So fall back to detection by name (AP 2024/03/27)
    std::string name = tz->name();

    if (name.substr(0, 4) == "Etc/")
        name = name.substr(4);

    return (name == "UTC" || name == "GMT");
}

std::vector<std::string> TimeZonePtr::get_region_list()
{
    // It seems that Date library (date/tz.cpp) returns additional
    // wrong names beginning with lowercase letter when using the system TZDB.
    // Require that all names must begin with an uppercase letter.
    std::vector<std::string> result;
    const date::tzdb& regions = date::get_tzdb();
    for (const auto& r : regions.zones)
    {
        const std::string& name = r.name();
        if (!name.empty() && std::isupper(*name.begin()))
            result.push_back(name);
    }

#if (defined(USE_OS_TZDB) && USE_OS_TZDB==0) || defined(FMI_CALENDAR_USES_STD_CHRONO)
    for (const auto& link : regions.links)
    {
        const std::string& name = link.name();
        if (!name.empty() && std::isupper(*name.begin()))
            result.push_back(name);
    }
#endif
    std::sort(result.begin(), result.end());
    return result;
}

std::map<std::string, TimeZonePtr> TimeZonePtr::get_region_map(bool debug)
{
    std::map<std::string, TimeZonePtr> result;
    const date::tzdb& regions = date::get_tzdb();
    for (const auto& r : regions.zones)
    {
        const std::string& name = r.name();
        if (!name.empty() && std::isupper(*name.begin()))
        {
            result[name] = &r;
            if (debug)
                std::cout << "TIME ZONE: " << name << std::endl;
        }
    }

#if (defined(USE_OS_TZDB) && USE_OS_TZDB==0) || defined(FMI_CALENDAR_USES_STD_CHRONO)
    for (const auto& link : regions.links)
    {
        const std::string& name = link.name();
        if (!name.empty() && std::isupper(*name.begin()))
        {
            try
            {
                const std::string& target = link.target();
                const TimeZonePtr tz(result.at(target));
                result[name] = tz;
                if (debug)
                    std::cout << "TIME ZONE LINK: " << name << " -> " << target << std::endl;
            }
            catch(const std::exception& e)
            {
                Fmi::Exception err(BCP, "Failed to resolve time zone link");
                err.addParameter("name", name);
                err.addParameter("target", link.target());
                throw err;
            }
        }
    }
#endif
    return result;
}
