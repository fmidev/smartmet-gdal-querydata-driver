// ======================================================================
/*!
 * \brief Implementation of singleton TimeZoneFactory
 */
// ======================================================================

#include "TimeZoneFactory.h"
#include "Exception.h"
#include "StringConversion.h"
#include "WorldTimeZones.h"
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std;

namespace Fmi
{
static const char* default_coordinates = "/usr/share/smartmet/timezones/timezone.shz";

// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple
 */
// ----------------------------------------------------------------------

class TimeZoneFactory::Impl
{
 public:
  Impl();

  std::unique_ptr<WorldTimeZones> m_Coordinates;
  const date::tzdb& m_Regions;
};

// ----------------------------------------------------------------------
/*!
 * \brief Implementation details constructor
 *
 * Note: Since we're using the static singleton pattern this makes everything
 * thread safe.
 */
// ----------------------------------------------------------------------

TimeZoneFactory::Impl::Impl()
    : m_Regions(date::get_tzdb())
{
  try
  {
    m_Coordinates.reset(new WorldTimeZones(default_coordinates));
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Private constructor for instance()
 */
// ----------------------------------------------------------------------

TimeZoneFactory::TimeZoneFactory() : m_Impl(new Impl()) {}

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

TimeZoneFactory::~TimeZoneFactory() = default;

// ----------------------------------------------------------------------
/*!
 * \brief Set the time zone coordinate database filename
 */
// ----------------------------------------------------------------------

void TimeZoneFactory::set_coordinate_file(const string&)
{
  std::cerr << "Warning: TimeZOneFactor::set_coordinate_file is deprecated\n" << std::flush;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the time region database filename
 */
// ----------------------------------------------------------------------

void TimeZoneFactory::set_region_file(const string&)
{
  std::cerr << "Warning: TimeZoneFactory::set_region_file is deprecated\n" << std::flush;
}

// ----------------------------------------------------------------------
/*!
 * \brief List the known databases
 */
// ----------------------------------------------------------------------

vector<string> TimeZoneFactory::region_list()
{
  try
  {
    return date_time::TimeZonePtr::get_region_list();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a time zone given a region name
 */
// ----------------------------------------------------------------------

Fmi::TimeZonePtr TimeZoneFactory::time_zone_from_region(const string& id)
{
  try
  {
    Fmi::TimeZonePtr ptr(date::locate_zone(id));

    if (!ptr)
       throw Fmi::Exception(BCP, "TimeZoneFactory does not recognize region '" + id + "'");

    return ptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a time zone given a string (region name or posix description)
 */
// ----------------------------------------------------------------------

Fmi::TimeZonePtr TimeZoneFactory::time_zone_from_string(const string& desc)
{
  try
  {
    // Try region name at first
    Fmi::TimeZonePtr ptr(date::locate_zone(desc));

    // FIXME: POSIX TZ are currently not supported
    //if (!ptr)
    //{
    ////Region name not found: try POSIX TZ description (may throw exception)
    //ptr.reset(new boost::local_time::posix_time_zone(desc));
    //}

    return ptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a time zone given a coordinate
 */
// ----------------------------------------------------------------------

Fmi::TimeZonePtr TimeZoneFactory::time_zone_from_coordinate(float lon, float lat)
{
  try
  {
    string tz = m_Impl->m_Coordinates->zone_name(lon, lat);
    Fmi::TimeZonePtr ptr = time_zone_from_string(tz);
    if (!ptr)
      throw Fmi::Exception(BCP,
                           "TimeZoneFactory could not convert given coordinate " +
                               Fmi::to_string(lon) + "," + Fmi::to_string(lat) +
                               " to a valid time zone name");

    return ptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a time zone given a coordinate
 */
// ----------------------------------------------------------------------

std::string TimeZoneFactory::zone_name_from_coordinate(float lon, float lat)
{
  try
  {
    return m_Impl->m_Coordinates->zone_name(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the TimeZoneFactory instance
 */
// ----------------------------------------------------------------------

TimeZoneFactory& TimeZoneFactory::instance()
{
  try
  {
    static TimeZoneFactory obj;
    return obj;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi
