// ======================================================================
/*!
 * \file
 * \brief Interface of TimeZones class
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"
#include <memory>
#include <string>
#include <vector>

namespace Fmi
{
class TimeZones
{
 public:
  ~TimeZones();
  TimeZones();
  TimeZones(const std::string& regionFile, const std::string& coordinateFile);
  TimeZones& operator=(const TimeZones& other) = delete;
  TimeZones(const TimeZones& other) = delete;

  std::vector<std::string> region_list() const;

  Fmi::TimeZonePtr time_zone_from_string(const std::string& desc) const;
  Fmi::TimeZonePtr time_zone_from_region(const std::string& id) const;
  Fmi::TimeZonePtr time_zone_from_coordinate(double lon, double lat) const;
  std::string zone_name_from_coordinate(double lon, double lat) const;

 private:
  // Implementation hiding
  class Pimple;
  std::unique_ptr<Pimple> itsPimple;

};  // class TimeZones
}  // namespace Fmi

// ======================================================================
