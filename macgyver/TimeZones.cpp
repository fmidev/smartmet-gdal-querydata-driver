// ======================================================================
/*!
 * \brief Implementation of class TimeZones
 */
// ======================================================================

#include "TimeZones.h"
#include "Exception.h"
#include "StringConversion.h"
#include "WorldTimeZones.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>

using namespace std;

const char* default_regions = "/usr/share/smartmet/timezones/date_time_zonespec.csv";
const char* default_coordinates = "/usr/share/smartmet/timezones/timezone.shz";

namespace Fmi
{
// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple
 */
// ----------------------------------------------------------------------

class TimeZones::Pimple
{
 public:
  Pimple(const std::string& regionsFile, const std::string& coordinatesFile)
      : itsRegions(date::get_tzdb()), itsCoordinates(coordinatesFile)
  {
    try
    {
      // Create all known timezones once for better access speed later on.
      auto regions = date_time::TimeZonePtr::get_region_list();
      for (const auto& id : regions)
      {
        date_time::TimeZonePtr ptr(id);
        if (!ptr)
          throw Fmi::Exception(BCP, "Unknown timezone definition")
              .addParameter("Filename", regionsFile)
              .addParameter("ID", id);

        itsKnownZones[id] = ptr;
      }
    }
    catch (...)
    {
      throw Fmi::Exception::Trace(BCP, "Operation failed!");
    }
  }
  const date::tzdb& itsRegions;
  WorldTimeZones itsCoordinates;
  std::unordered_map<std::string, Fmi::TimeZonePtr> itsKnownZones;
};

// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

TimeZones::~TimeZones() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

TimeZones::TimeZones() : itsPimple(new Pimple(default_regions, default_coordinates)) {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor with alternate data sources
 */
// ----------------------------------------------------------------------

TimeZones::TimeZones(const std::string& regionsFile, const std::string& coordinatesFile)
    : itsPimple(new Pimple(regionsFile, coordinatesFile))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief List the known databases
 */
// ----------------------------------------------------------------------

vector<string> TimeZones::region_list() const
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

Fmi::TimeZonePtr TimeZones::time_zone_from_region(const string& id) const
{
  try
  {
    auto value = itsPimple->itsKnownZones.find(id);
    if (value != itsPimple->itsKnownZones.end())
      return value->second;

    throw Fmi::Exception(BCP, "TimeZones does not recognize region '" + id + "'");
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

Fmi::TimeZonePtr TimeZones::time_zone_from_string(const string& desc) const
{
  try
  {
    // Try region name at first
    auto value = itsPimple->itsKnownZones.find(desc);
    if (value != itsPimple->itsKnownZones.end())
      return value->second;

    // FIXME: POSIX TZ descriptions are not supported currently
    // Try POSIX TZ description (may throw) if region name is unknown
    //return Fmi::TimeZonePtr(new boost::local_time::posix_time_zone(desc));

    return Fmi::TimeZonePtr();
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

Fmi::TimeZonePtr TimeZones::time_zone_from_coordinate(double lon, double lat) const
{
  try
  {
    string tz = itsPimple->itsCoordinates.zone_name(lon, lat);
    Fmi::TimeZonePtr ptr = time_zone_from_string(tz);
    if (!ptr)
      throw Fmi::Exception(BCP,
                           "TimeZones could not convert given coordinate " + Fmi::to_string(lon) +
                               "," + Fmi::to_string(lat) + " to a valid time zone name");

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

std::string TimeZones::zone_name_from_coordinate(double lon, double lat) const
{
  try
  {
    return itsPimple->itsCoordinates.zone_name(lon, lat);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Fmi
