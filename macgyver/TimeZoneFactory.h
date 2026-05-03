// ======================================================================
/*!
 * \file
 * \brief Interface of (thread safe) singleton TimeZoneFactory
 */
// ======================================================================

#pragma once

#include "LocalDateTime.h"
#include <memory>
#include <string>
#include <vector>

namespace Fmi
{
class TimeZoneFactory
{
 public:
  std::vector<std::string> region_list();

  Fmi::TimeZonePtr time_zone_from_string(const std::string& desc);
  Fmi::TimeZonePtr time_zone_from_region(const std::string& id);
  Fmi::TimeZonePtr time_zone_from_coordinate(float lon, float lat);
  std::string zone_name_from_coordinate(float lon, float lat);

  static TimeZoneFactory& instance();

  // NO LONGER SUPPORTED, THESE JUST PRINT WARNINGS!
  void set_region_file(const std::string& file);
  void set_coordinate_file(const std::string& file);

 private:
  // Implementation hiding
  class Impl;
  std::unique_ptr<Impl> m_Impl;

  TimeZoneFactory();
  ~TimeZoneFactory();

};  // class TimeZoneFactory
}  // namespace Fmi

// ======================================================================
